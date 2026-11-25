#pragma once

#include "CoreMinimal.h"
#include "BaseDirectStream.h"
#include "BaseConnect.h"
#include "P2PHelperStream.h"
#include "gsync/example1/p2p_context.h"
#include "gsync/example1/p2p_datagram.h"
#include <memory>
#include <atomic>
#include <type_traits>

struct FMqasPeerState;
struct FMqasDirectStreamState;
class FMqasDatagram;
class BaseDirectStream;

// All methods and delegates are used on the game thread. The SDK retains the
// native callback state until the peer has finished closing.
class MQASNET_API FMqasPeer : public BaseConnect
{
public:
    static std::shared_ptr<FMqasPeer> Create();
    ~FMqasPeer() override;
    // Attach a stream to this peer. On the server, inbound streams match a
    // pre-attached stream by negotiated local ID; unmatched streams are
    // created automatically and delivered by OnIncomingStream.
    template<class T> requires std::is_base_of_v<BaseDirectStream, T>
    std::shared_ptr<T> AddStream(GSY_P2PStreamId LocalStreamId) {
        auto Stream = T::Create();
        return RegisterStream(Stream, LocalStreamId) ? Stream : nullptr;
    }
    ErrorCode Start(uint64 Client, const FString& Config, const std::shared_ptr<P2PHelperStream>& Lobby,
        const FP2PHelperResult& Result, GSY_P2PStreamPolicy StreamPolicy = nullptr,
        decltype(GSY_P2PDatagramConnectionContext::on_channel_request) ChannelPolicy = nullptr);
    void Close();
    void Disconnect() override;
    GSY_PeerId GetRemotePeerId() const;
    GSY_ConnectionHwnd GetConnection() const;
    ErrorCode FindStream(GSY_P2PStreamId RemoteStreamId, GSY_StreamId& OutStream) const;
    std::shared_ptr<BaseDirectStream> FindDirectStream(GSY_P2PStreamId RemoteStreamId) const;
    std::shared_ptr<BaseDirectStream> FindLocalStream(GSY_P2PStreamId LocalStreamId) const;
    DECLARE_MULTICAST_DELEGATE(FConnected);
    DECLARE_MULTICAST_DELEGATE_OneParam(FTerminal, ErrorCode);
    DECLARE_MULTICAST_DELEGATE_OneParam(FIncomingStream, std::shared_ptr<BaseDirectStream>);
    DECLARE_MULTICAST_DELEGATE_TwoParams(FStreamFailed, GSY_StreamId, ErrorCode);
    FConnected OnConnected;
    FTerminal OnFailed;
    FTerminal OnClosed;
    FIncomingStream OnIncomingStream;
    FStreamFailed OnStreamFailed;
private:
    FMqasPeer();
    void OnNativeStreamOpen(GSY_ConnectionHwnd, GSY_StreamId, ErrorCode) override;
    void OnNativeStreamClose(GSY_ConnectionHwnd, GSY_StreamId, ErrorCode) override;
    bool RegisterStream(const std::shared_ptr<BaseDirectStream>& Stream, GSY_P2PStreamId LocalStreamId);
    std::shared_ptr<FMqasPeerState> State;
    friend struct FMqasPeerState;
};

class MQASNET_API FMqasDatagram : public BaseDirectStream
{
public:
    static std::shared_ptr<FMqasDatagram> Create();
    ~FMqasDatagram() override;
    EMqasDirectStreamType GetDirectStreamType() const override { return EMqasDirectStreamType::Datagram; }
    ErrorCode Open(GSY_RequestId RequestId = NONE_RID) override;
    GSY_P2PDatagramConnectionContext* GetContext();
    ErrorCode Disconnect(const FString& Reason = FString(), GSY_RequestId RequestId = NONE_RID) override;
    ErrorCode OpenChannel(GSY_RequestId RequestId = NONE_RID);
    ErrorCode CloseChannel(GSY_RequestId RequestId = NONE_RID);
    ErrorCode Send(const TArray<uint8>& Data, GSY_RequestId RequestId = NONE_RID);
    bool IsChannelOpen() const;
    DECLARE_MULTICAST_DELEGATE_OneParam(FReceived, const TArray<uint8>&);
    DECLARE_MULTICAST_DELEGATE_TwoParams(FRejected, const FString&, GSY_RequestId);
    DECLARE_MULTICAST_DELEGATE_FourParams(FChannelResult, bool, ErrorCode, GSY_RequestId, const FString&);
    FRejected OnRejected;
    FChannelResult OnChannelResult;
    FConnected OnChannelOpened;
    FConnected OnChannelClosed;
    FReceived OnReceived;
protected:
    FMqasDatagram() = default;
    std::atomic<bool> ChannelOpen{false};
    friend class FMqasPeer;
    friend struct FMqasPeerState;
};

// Called by the SDK owner after GSY_terminate has drained native callbacks.
void MQASNET_API MqasP2PShutdown();
