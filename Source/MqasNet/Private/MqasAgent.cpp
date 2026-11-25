#include "MqasAgent.h"
#include "MqasServer.h"
#include "Async/Async.h"
#include "EUtility/Public/UFilePreprocessor.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <unordered_map>
#include <atomic>
#include <mutex>

struct FMqasClientState
{
    bool Active = true;
    std::unordered_map<GSY_ConnectionHwnd, std::shared_ptr<BaseConnect>> Connections;
    std::vector<std::shared_ptr<FMqasPeer>> Peers;
    std::vector<std::shared_ptr<FMqasServer>> Servers;
};
struct FMqasAgent::FImpl
{
    std::mutex NativeMutex;
    std::unordered_map<GSY_ConnectionHwnd, std::weak_ptr<BaseConnect>> NativeConnections;
    GSY_Context Context{};
    GSY_BaseConnectionContext ConnectionContext{};
    bool Initialized = false;
    std::atomic<uint64> Epoch{0};
    FMqasClientId NextId = 1;
    std::unordered_map<FMqasClientId, std::shared_ptr<FMqasClientState>> Clients;
    // Keep callback contexts and stream objects alive until the SDK drains.
    std::vector<std::shared_ptr<FMqasClientState>> Retired;
    std::unordered_map<GSY_ConnectionHwnd, std::weak_ptr<FMqasClientState>> Owners;
};
FMqasAgent& FMqasAgent::Get() { static FMqasAgent Instance; return Instance; }
FMqasAgent::FMqasAgent() : Impl(std::make_unique<FImpl>())
{
    Impl->Context.on_error = Error;
    auto& C = Impl->ConnectionContext;
    C.on_connect = Connected; C.on_disconnect = Disconnected; C.on_error = ConnectError;
    C.on_stream_open = StreamOpen; C.on_stream_close = StreamClose;
}
FMqasAgent::~FMqasAgent()
{
    if (Impl->Initialized) { GSY_terminate(); MqasP2PShutdown(); MqasServerShutdown(); }
}
uint64 FMqasAgent::GetRuntimeEpoch() const { return Impl->Epoch.load(); }
FMqasClientId FMqasAgent::CreateClient()
{
    check(IsInGameThread());
    if (!Impl->Initialized) {
        ++Impl->Epoch;
        if (GSY_initialize(0, &Impl->Context) != EC_Ok) return InvalidMqasClient;
        Impl->Initialized = true;
    }
    const auto Id = Impl->NextId++;
    Impl->Clients.emplace(Id, std::make_shared<FMqasClientState>());
    return Id;
}
void FMqasAgent::ReleaseClient(FMqasClientId Id)
{
    check(IsInGameThread());
    auto It = Impl->Clients.find(Id); if (It == Impl->Clients.end()) return;
    auto Client = It->second; Client->Active = false;
    const auto Peers = Client->Peers;
    for (const auto& Peer : Peers) if (Peer) Peer->Close();
    for (auto& Pair : Client->Connections) GSY_disconnect(Pair.first);
    Client->Servers.clear();
    Impl->Retired.push_back(Client);
    Impl->Clients.erase(It);
    PruneRetired();
    if (Impl->Clients.empty() && Impl->Initialized) {
        ++Impl->Epoch;
        if (GSY_terminate() == EC_Ok) {
            MqasP2PShutdown(); MqasServerShutdown(); Impl->Initialized = false;
            for (const auto& Retired : Impl->Retired)
                for (const auto& [Handle, Connection] : Retired->Connections)
                    if (Connection) Connection->Invalidate();
            Impl->NativeConnections.clear(); Impl->Owners.clear(); Impl->Retired.clear();
        }
    }
}
bool FMqasAgent::AgentIsInitialized(FMqasClientId Id) const { return Impl->Initialized && Impl->Clients.contains(Id); }
bool FMqasAgent::AgentIsConnected(FMqasClientId Id, GSY_ConnectionHwnd H) const
{
    auto C = Impl->Clients.find(Id); if (C == Impl->Clients.end()) return false;
    auto It = C->second->Connections.find(H); return It != C->second->Connections.end() && It->second->IsConnected();
}
TArray<GSY_PeerId> FMqasAgent::AgentGetPeerIds(FMqasClientId Id) const
{
    check(IsInGameThread());
    TArray<GSY_PeerId> Result;
    const auto Client = Impl->Clients.find(Id);
    if (Client == Impl->Clients.end()) return Result;
    for (const auto& Peer : Client->second->Peers)
        if (Peer && Peer->GetRemotePeerId() != INVALID_PID) Result.Add(Peer->GetRemotePeerId());
    return Result;
}
FString FMqasAgent::GetConfigPath(FMqasClientId Id, const FString& Config) const
{
    if (!Impl->Clients.contains(Id)) return {};
    const FString Source = FPaths::ProjectContentDir() / Config;
    const FString Destination = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() /
        TEXT("MqasClients") / FString::Printf(TEXT("%u-%llu"), FPlatformProcess::GetCurrentProcessId(), static_cast<unsigned long long>(Id)) / Config);
    FString Content;
    if (!FFileHelper::LoadFileToString(Content, *Source)) return {};
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(Destination));
    return FFileHelper::SaveStringToFile(UFilePreprocessor::PreprocessString(Content), *Destination) ? Destination : FString();
}
std::weak_ptr<BaseConnect> FMqasAgent::AgentConnectTo(FMqasClientId Id, GSY_EngineId Engine, const FString& Config, const FString& Ip, short Port)
{
    return Connect(Id, Engine, Config, Ip, Port, nullptr);
}
std::weak_ptr<BaseConnect> FMqasAgent::Connect(FMqasClientId Id, GSY_EngineId Engine,
    const FString& Config, const FString& Ip, short Port, const GSY_ContextExtension* Options)
{
    check(IsInGameThread());
    auto C = Impl->Clients.find(Id);
    if (C == Impl->Clients.end()) return {};
    const auto Path = GetConfigPath(Id, Config);
    if (Path.IsEmpty()) return {};
    auto& Context = Impl->ConnectionContext;
    Context.options = Options;
    const auto H = GSY_connect(Engine, TCHAR_TO_UTF8(*Path), TCHAR_TO_UTF8(*Ip), Port, &Context);
    Context.options = nullptr;
    if (H == InvalidConnection) return {};
    auto Conn = std::make_shared<BaseConnect>(); Conn->InitData(H, Engine, Config, Ip, Port);
    C->second->Connections.emplace(H, Conn); Impl->Owners.emplace(H, C->second);
    return Conn;
}
ErrorCode FMqasAgent::ConnectSession(FMqasClientId Id, const FString& Config,
    GSY_SessionHandle Session, std::weak_ptr<BaseConnect>& OutConnection, std::shared_ptr<BaseConnect> Connection)
{
    check(IsInGameThread());
    OutConnection.reset();
    auto Client = Impl->Clients.find(Id);
    if (Client == Impl->Clients.end()) return EC_NotInitialized;
    const FString Path = GetConfigPath(Id, Config);
    if (Path.IsEmpty()) return EC_Fail;
    if (Connection && Connection->GetHwnd() != InvalidConnection) return EC_WrongStreamState;
    if (!Connection) Connection = std::make_shared<BaseConnect>();
    std::lock_guard Lock(Impl->NativeMutex);
    GSY_ConnectionHwnd Handle = InvalidConnection;
    const ErrorCode Code = GSY_connect_session(TCHAR_TO_UTF8(*Path), Session,
        &Impl->ConnectionContext, &Handle);
    if (Code != EC_Pending) return Code;
    Impl->NativeConnections.insert_or_assign(Handle, Connection);
    Connection->InitData(Handle, 2, Config, FString(), 0);
    Client->second->Connections.emplace(Handle, Connection);
    Impl->Owners.emplace(Handle, Client->second);
    OutConnection = Connection;
    return Code;
}

void FMqasAgent::RetainPeer(FMqasClientId Id, const std::shared_ptr<FMqasPeer>& Peer)
{
    check(IsInGameThread()); auto C = Impl->Clients.find(Id);
    if (C == Impl->Clients.end() || !Peer) return;
    C->second->Peers.emplace_back(Peer);
    std::weak_ptr<FMqasClientState> WeakClient = C->second;
    std::weak_ptr<FMqasPeer> WeakPeer = Peer;
    auto Remove = [WeakClient, WeakPeer](ErrorCode) {
        auto Client = WeakClient.lock(); auto Current = WeakPeer.lock();
        if (!Client || !Current) return;
        std::erase_if(Client->Peers, [&Current](const auto& Item) { return Item == Current; });
        FMqasAgent::Get().PruneRetired();
    };
    Peer->OnFailed.AddLambda(Remove);
    Peer->OnClosed.AddLambda(Remove);
}
bool FMqasAgent::RetainServer(FMqasClientId Id, const std::shared_ptr<FMqasServer>& Server)
{
    check(IsInGameThread());
    auto Client = Impl->Clients.find(Id);
    if (Client == Impl->Clients.end() || !Server) return false;
    auto& Servers = Client->second->Servers;
    if (std::find(Servers.begin(), Servers.end(), Server) == Servers.end()) Servers.push_back(Server);
    return true;
}
void FMqasAgent::ReleaseServer(FMqasClientId Id, const std::shared_ptr<FMqasServer>& Server)
{
    check(IsInGameThread());
    auto Client = Impl->Clients.find(Id);
    if (Client != Impl->Clients.end()) std::erase(Client->second->Servers, Server);
}
TArray<std::shared_ptr<FMqasPeer>> FMqasAgent::FindPeers(FMqasClientId Id, GSY_PeerId Remote) const
{
    check(IsInGameThread());
    TArray<std::shared_ptr<FMqasPeer>> Result;
    auto C = Impl->Clients.find(Id); if (C == Impl->Clients.end()) return Result;
    for (const auto& Peer : C->second->Peers)
        if (Peer && Peer->GetRemotePeerId() == Remote) Result.Add(Peer);
    return Result;
}
bool FMqasAgent::FindPeer(FMqasClientId Id, GSY_PeerId Remote, std::shared_ptr<FMqasPeer>& OutPeer) const
{
    auto C = Impl->Clients.find(Id); if (C == Impl->Clients.end()) return false;
    for (const auto& Peer : C->second->Peers) {
        if (Peer && Peer->GetRemotePeerId() == Remote) { OutPeer = Peer; return true; }
    }
    return false;
}
void FMqasAgent::PruneRetired()
{
    std::erase_if(Impl->Retired, [](const auto& Client) {
        return Client->Connections.empty() && Client->Peers.empty();
    });
}
void FMqasAgent::Error(const char* Message, int Code)
{
    FString Copy = Message ? UTF8_TO_TCHAR(Message) : TEXT("");
    const uint64 Epoch = Get().Impl->Epoch.load();
    AsyncTask(ENamedThreads::GameThread, [Copy, Code, Epoch] {
        if (Get().Impl->Epoch.load() != Epoch) return;
        UE_LOG(LogTemp, Warning, TEXT("Mqas SDK: %s (%d)"), *Copy, Code);
        Get().OnError.Broadcast(Copy, Code);
    });
}
void FMqasAgent::Connected(int C, GSY_ConnectionHwnd H) { Get().Dispatch(0,H,0,C); }
void FMqasAgent::Disconnected(int C, GSY_ConnectionHwnd H) { Get().Dispatch(1,H,0,C); }
void FMqasAgent::ConnectError(int C, GSY_ConnectionHwnd H) { Get().Dispatch(2,H,0,C); }
void FMqasAgent::StreamOpen(GSY_ConnectionHwnd H, GSY_StreamId S, ErrorCode C) { Get().Dispatch(3,H,S,C); }
void FMqasAgent::StreamClose(GSY_ConnectionHwnd H, GSY_StreamId S, ErrorCode C) { Get().Dispatch(4,H,S,C); }
void FMqasAgent::Dispatch(int Kind, GSY_ConnectionHwnd Handle, GSY_StreamId Stream, int Code)
{
    if (Kind >= 1) {
        std::shared_ptr<BaseConnect> Native;
        {
            std::lock_guard Lock(Impl->NativeMutex);
            if (auto It = Impl->NativeConnections.find(Handle); It != Impl->NativeConnections.end()) {
                Native = It->second.lock();
                if (Kind == 1 || Kind == 2) Impl->NativeConnections.erase(It);
            }
        }
        if (Native && Kind == 3) Native->OnNativeStreamOpen(Handle, Stream, static_cast<ErrorCode>(Code));
        if (Native && Kind == 4) Native->OnNativeStreamClose(Handle, Stream, static_cast<ErrorCode>(Code));
    }
    const uint64 Epoch = Impl->Epoch.load();
    AsyncTask(ENamedThreads::GameThread, [this, Epoch, Kind, Handle, Stream, Code] {
        if (Impl->Epoch.load() != Epoch) return;
        auto Owner = Impl->Owners.find(Handle); if (Owner == Impl->Owners.end()) return;
        auto Client = Owner->second.lock(); if (!Client) return;
        auto It = Client->Connections.find(Handle); if (It == Client->Connections.end()) return;
        auto Conn = It->second;
        if (!Client->Active) {
            if (Kind == 0) GSY_disconnect(Handle);
            else if (Kind == 1) Conn->OnDisconnect(Code);
            else if (Kind == 2) Conn->OnConnectError(Code);
        } else if (Kind == 0) Conn->OnConnected(Code);
        else if (Kind == 1) Conn->OnDisconnect(Code);
        else if (Kind == 2) Conn->OnConnectError(Code);
        else if (Kind == 3) Conn->OnStreamOpen(Stream, static_cast<ErrorCode>(Code));
        else if (Kind == 4) Conn->OnStreamClose(Stream, static_cast<ErrorCode>(Code));
        if (Kind == 1 || Kind == 2) { Client->Connections.erase(Handle); Impl->Owners.erase(Handle); PruneRetired(); }
    });
}
