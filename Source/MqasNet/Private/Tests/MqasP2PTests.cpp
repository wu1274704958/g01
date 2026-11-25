#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "MqasP2P.h"
#include "MqasAgent.h"
#include "MqasServer.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace {
class FTrackingStream : public BaseStream
{
public:
    int32 CloseCount = 0;
protected:
    void OnClose() override { ++CloseCount; BaseStream::OnClose(); }
};
class FAsyncStream : public FTrackingStream
{
public:
    int* GetContext() { return &Context; }
private:
    int Context = 0;
};
class FTestConnection : public BaseConnect
{
public:
    void AddStream(GSY_StreamId Id, const std::shared_ptr<BaseStream>& Stream)
    {
        Stream->InitData(Id, 123);
        _StreamMap.emplace(Id, Stream);
    }
    void SimulateConnect() { InitData(123, 0, FString(), FString(), 0); OnConnected(EC_Ok); }
    void SimulateDisconnect() { OnDisconnect(EC_Disconnected); }
    void SimulateStreamClose(GSY_StreamId Id) { OnStreamClose(Id, EC_Ok); }
};
class FTestLobbyStream : public P2PHelperStream
{
public:
    void CloseForTest() { OnClose(); }
};
class FCustomDatagram : public FMqasDatagram
{
public:
    static std::shared_ptr<FCustomDatagram> Create() { return std::shared_ptr<FCustomDatagram>(new FCustomDatagram()); }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasAsyncStreamCreationTest, "MqasNet.Lifecycle.AsyncStreamCreation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasAsyncStreamCreationTest::RunTest(const FString&)
{
    auto Connection = std::make_shared<FTestConnection>();
    Connection->SimulateConnect();
    auto Stream = std::make_shared<FAsyncStream>();
    auto Completed = std::make_shared<bool>(false);
    auto Code = Connection->MakeStream(Stream, [](GSY_ConnectionHwnd, int*, BaseConnect::FStreamCreated Complete, void* User) {
        Complete(EC_Ok, 42, 1234, User);
        return EC_Pending;
    }, [this, Completed](ErrorCode Result, GSY_StreamId Id, GSY_RequestId Rid) {
        TestEqual(TEXT("Completion result"), Result, EC_Ok);
        TestEqual(TEXT("Completion stream ID"), Id, GSY_StreamId(42));
        TestEqual(TEXT("Completion request ID"), Rid, GSY_RequestId(1234));
        *Completed = true;
    });
    TestEqual(TEXT("Request accepted"), Code, EC_Pending);
    const auto Start = FPlatformTime::Seconds();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Connection, Stream, Completed, Start] {
        if (!*Completed && FPlatformTime::Seconds() - Start < 3.0) return false;
        TestTrue(TEXT("Completed on game thread"), *Completed);
        TestTrue(TEXT("Connection owns the SDK stream"), Connection->FindStream(42) == Stream && Stream->Valid());
        Connection->SimulateDisconnect();
        TestFalse(TEXT("Connection invalidates the stream"), Stream->Valid());
        return true;
    }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasConnectionStreamLifetimeTest, "MqasNet.Lifecycle.ConnectionStreams", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasConnectionStreamLifetimeTest::RunTest(const FString&)
{
    FTestConnection Connection;
    auto Stream = std::make_shared<FTrackingStream>();
    Connection.AddStream(42, Stream);
    TestTrue(TEXT("Stream initially valid"), Stream->Valid());
    Connection.SimulateDisconnect();
    TestFalse(TEXT("Disconnect invalidates externally held stream"), Stream->Valid());
    TestEqual(TEXT("Stream closes once"), Stream->CloseCount, 1);
    Connection.SimulateStreamClose(42);
    TestEqual(TEXT("Late stream close does not close twice"), Stream->CloseCount, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasStaleLobbyCallbackTest, "MqasNet.Lifecycle.StaleLobbyCallback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasStaleLobbyCallbackTest::RunTest(const FString&)
{
    auto& Agent = FMqasAgent::Get();
    const auto OldClient = Agent.CreateClient();
    TestTrue(TEXT("Original client initialized"), Agent.AgentIsInitialized(OldClient));
    const uint64 OldEpoch = Agent.GetRuntimeEpoch();
    constexpr GSY_StreamId StreamId = 0x7FFFFFFFFFFFFF01ULL;
    auto Old = std::make_shared<FTestLobbyStream>();
    Old->InitData(StreamId, 123);
    const auto Callback = Old->GetContext()->on_registration_success;
    Async(EAsyncExecution::Thread, [Callback] { Callback(EC_Ok, StreamId, 7); }).Wait();
    Old->CloseForTest();
    Agent.ReleaseClient(OldClient);
    if (Agent.GetRuntimeEpoch() == OldEpoch) {
        AddWarning(TEXT("Another client owns the SDK; runtime restart test skipped."));
        return true;
    }
    const auto NewClient = Agent.CreateClient();
    TestTrue(TEXT("Replacement client initialized"), Agent.AgentIsInitialized(NewClient));
    auto Replacement = std::make_shared<FTestLobbyStream>();
    Replacement->InitData(StreamId, 456);
    const double Start = FPlatformTime::Seconds();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Replacement, NewClient, Start] {
        if (FPlatformTime::Seconds() - Start < 0.25) return false;
        TestFalse(TEXT("Old runtime callback cannot register a replacement stream"), Replacement->IsRegistered());
        Replacement->CloseForTest();
        FMqasAgent::Get().ReleaseClient(NewClient);
        return true;
    }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasHelperResultTest, "MqasNet.P2P.HelperResultOwnsData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasHelperResultTest::RunTest(const FString&)
{
    uint8 Token[] = {0, 1, 255, 42};
    char Reason[] = "original";
    char Relay[] = "relay";
    GSY_HelperResult Raw{}; Raw.peer_id = 7; Raw.reason = Reason; Raw.relay_token = Relay;
    Raw.verify_token = Token; Raw.verify_token_len = sizeof(Token);
    FP2PHelperResult Copy(Raw);
    Token[0] = 9; Reason[0] = 'X'; Relay[0] = 'X';
    FP2PHelperResult Second = Copy;
    auto Native = Second.Native();
    TestEqual(TEXT("Peer id"), Native.peer_id, static_cast<GSY_PeerId>(7));
    TestEqual(TEXT("Binary token length"), Native.verify_token_len, static_cast<size_t>(4));
    TestEqual(TEXT("Token was copied"), Native.verify_token[0], static_cast<uint8>(0));
    TestEqual(TEXT("Token retains non-ASCII bytes"), Native.verify_token[2], static_cast<uint8>(255));
    TestEqual(TEXT("Reason was copied"), FString(UTF8_TO_TCHAR(Native.reason)), FString(TEXT("original")));
    TestEqual(TEXT("Relay token was copied"), FString(UTF8_TO_TCHAR(Native.relay_token)), FString(TEXT("relay")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasPeerConfigurationTest, "MqasNet.P2P.Configuration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasPeerConfigurationTest::RunTest(const FString&)
{
    auto Peer = FMqasPeer::Create(); auto Other = FMqasPeer::Create();
    std::shared_ptr<BaseConnect> BasePeer = Peer;
    static_assert(std::has_virtual_destructor_v<BaseConnect>);
    TestFalse(TEXT("Peer base interface begins disconnected"), BasePeer->IsConnected());
    TestEqual(TEXT("Peer base interface begins without connection handle"), BasePeer->GetHwnd(), InvalidConnection);
    BasePeer->Disconnect();
    auto Datagram = Peer->AddStream<FMqasDatagram>(101);
    if (!TestTrue(TEXT("Attach datagram after disconnecting an idle peer"), bool(Datagram))) return false;
    TestFalse(TEXT("Peer begins disconnected"), Peer->IsConnected());
    TestFalse(TEXT("Datagram begins disconnected"), Datagram->IsConnected());
    TestFalse(TEXT("Stream id must be nonzero"), bool(Peer->AddStream<FMqasDatagram>(0)));
    TestFalse(TEXT("Duplicate local stream ID is rejected"), bool(Peer->AddStream<FMqasDatagram>(101)));
    TestTrue(TEXT("A peer can attach another stream with a different ID"), bool(Peer->AddStream<FMqasDatagram>(102)));
    auto Custom = Peer->AddStream<FCustomDatagram>(103);
    TestTrue(TEXT("Custom stream subclass can be registered"), bool(Custom));
    TestTrue(TEXT("Local lookup preserves custom stream identity"), Peer->FindLocalStream(103) == Custom);
    TestEqual(TEXT("Local stream ID"), Datagram->GetLocalStreamId(), static_cast<GSY_P2PStreamId>(101));
    TestEqual(TEXT("Cannot open before transport"), Datagram->Open(), EC_Fail);
    TestFalse(TEXT("Channel begins closed"), Datagram->IsChannelOpen());
    TestEqual(TEXT("Cannot open channel before stream"), Datagram->OpenChannel(), EC_Fail);
    TestEqual(TEXT("Cannot close channel before stream"), Datagram->CloseChannel(), EC_Fail);
    GSY_P2PStreamId Local = 123, Remote = 123;
    TestEqual(TEXT("Cannot query ids before stream"), Datagram->GetStreamIds(Local, Remote), EC_Fail);
    TestEqual(TEXT("Invalid local id is reset"), Local, static_cast<GSY_P2PStreamId>(0));
    TestEqual(TEXT("Invalid remote id is reset"), Remote, static_cast<GSY_P2PStreamId>(0));
    TArray<uint8> Data{1};
    TestEqual(TEXT("Cannot send before authentication"), Datagram->Send(Data), EC_Fail);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasClientLifecycleTest, "MqasNet.P2P.MultipleClientLifetimes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasClientLifecycleTest::RunTest(const FString&)
{
    auto& Agent = FMqasAgent::Get();
    const auto A = Agent.CreateClient(); const auto B = Agent.CreateClient();
    TestTrue(TEXT("First client starts SDK"), Agent.AgentIsInitialized(A));
    TestTrue(TEXT("Second client shares SDK"), Agent.AgentIsInitialized(B));
    TestNotEqual(TEXT("Client ids differ"), A, B);
    TestTrue(TEXT("New clients start without retained peers"), Agent.AgentGetPeerIds(A).IsEmpty());
    const uint64 SharedEpoch = Agent.GetRuntimeEpoch();
    Agent.ReleaseClient(A);
    TestFalse(TEXT("Released client is inactive"), Agent.AgentIsInitialized(A));
    TestTrue(TEXT("Released client exposes no peers"), Agent.AgentGetPeerIds(A).IsEmpty());
    TestTrue(TEXT("Other client remains active"), Agent.AgentIsInitialized(B));
    TestEqual(TEXT("Releasing one client preserves the SDK runtime"), Agent.GetRuntimeEpoch(), SharedEpoch);
    Agent.ReleaseClient(B);
    TestFalse(TEXT("Last client is inactive"), Agent.AgentIsInitialized(B));
    const auto C = Agent.CreateClient();
    TestTrue(TEXT("SDK can initialize for a later session"), Agent.AgentIsInitialized(C));
    TestNotEqual(TEXT("New client id is not reused"), C, B);
    Agent.ReleaseClient(C);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasServerLifecycleTest, "MqasNet.Server.StartStop", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasServerLifecycleTest::RunTest(const FString&)
{
    const FString Fixture = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("LocalP2PBundle/server"));
    const FString Cert = (Fixture / TEXT("cert-def.pem")).Replace(TEXT("\\"), TEXT("/"));
    const FString Key = (Fixture / TEXT("key-def.pem")).Replace(TEXT("\\"), TEXT("/"));
    if (!FPaths::FileExists(Cert) || !FPaths::FileExists(Key)) { AddWarning(TEXT("Local P2P test certificate unavailable; server integration test skipped.")); return true; }
    const FString Config = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("MqasServerAutomation.toml"));
    const FString Contents = FString::Printf(TEXT("[engine_config]\nbind_ip=\"127.0.0.1\"\nport=0\nlog_level=\"info\"\nalpn=\"echo\"\nssl_cert_path=\"%s\"\nssl_key_path=\"%s\"\n"), *Cert, *Key);
    if (!FFileHelper::SaveStringToFile(Contents, *Config)) { AddError(TEXT("Could not write server test configuration")); return false; }
    auto& Agent = FMqasAgent::Get();
    const auto Client = Agent.CreateClient();
    if (Client == InvalidMqasClient) { AddError(TEXT("SDK initialization failed")); return false; }
    struct FState { bool Started = false, Stopped = false, Failed = false; };
    auto Result = std::make_shared<FState>();
    auto Server = FMqasServer::Create();
    std::weak_ptr<FMqasServer> Weak = Server;
    Server->OnStarted.AddLambda([Result, Weak](ErrorCode Code) {
        Result->Started = Code == EC_Ok;
        if (auto S = Weak.lock(); !S || S->Stop() != EC_Ok) Result->Failed = true;
    });
    Server->OnStopped.AddLambda([Result](ErrorCode Code) { Result->Stopped = Code == EC_Ok; });
    Server->OnError.AddLambda([Result](ErrorCode) { Result->Failed = true; });
    TestNotEqual(TEXT("Launch returns a server handle"), Server->Launch(1, Config), static_cast<GSY_ServerHwnd>(InvalidServer));
    const double Start = FPlatformTime::Seconds();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Result, Server, Client, Start] {
        if (!Result->Stopped && !Result->Failed && FPlatformTime::Seconds() - Start < 10.0) return false;
        TestTrue(TEXT("Server started"), Result->Started);
        TestTrue(TEXT("Server stopped"), Result->Stopped);
        TestFalse(TEXT("No server error"), Result->Failed);
        FMqasAgent::Get().ReleaseClient(Client);
        return true;
    }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqasServerStartFailureTest, "MqasNet.Server.StartFailure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMqasServerStartFailureTest::RunTest(const FString&)
{
    const auto Client = FMqasAgent::Get().CreateClient();
    if (Client == InvalidMqasClient) { AddError(TEXT("SDK initialization failed")); return false; }
    auto Server = FMqasServer::Create();
    auto Failure = std::make_shared<ErrorCode>(EC_Ok);
    Server->OnError.AddLambda([Failure](ErrorCode Code) { *Failure = Code; });
    const FString Missing = FPaths::ProjectSavedDir() / TEXT("mqas-missing-server-configuration.toml");
    TestNotEqual(TEXT("Invalid configuration returns an asynchronous handle"), Server->Launch(1, Missing), static_cast<GSY_ServerHwnd>(InvalidServer));
    const double Start = FPlatformTime::Seconds();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Server, Client, Failure, Start] {
        if (*Failure == EC_Ok && FPlatformTime::Seconds() - Start < 10.0) return false;
        TestNotEqual(TEXT("Startup failure reports an error"), *Failure, EC_Ok);
        TestFalse(TEXT("Failed server is not started"), Server->IsStarted());
        TestEqual(TEXT("Failed server cannot be stopped"), Server->Stop(), EC_InvalidHandler);
        FMqasAgent::Get().ReleaseClient(Client);
        return true;
    }));
    return true;
}
#endif
