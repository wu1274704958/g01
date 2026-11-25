#pragma once

#include "CoreMinimal.h"
#include "BaseStream.h"
#include "gsync/core/types.h"
#include <memory>

struct FMqasDirectStreamState;
struct FMqasPeerState;
class FMqasPeer;

enum class EMqasDirectStreamType : uint8 { Datagram };

// Public methods and events are used on the game thread.
class MQASNET_API BaseDirectStream : public BaseStream
{
public:
    virtual ~BaseDirectStream();
    virtual EMqasDirectStreamType GetDirectStreamType() const = 0;
    virtual ErrorCode Open(GSY_RequestId RequestId = NONE_RID) = 0;
    virtual ErrorCode Disconnect(const FString& Reason = FString(), GSY_RequestId RequestId = NONE_RID) = 0;
    bool IsConnected() const;
    GSY_P2PStreamId GetLocalStreamId() const;
    ErrorCode GetStreamIds(GSY_P2PStreamId& OutLocal, GSY_P2PStreamId& OutRemote) const;
    DECLARE_MULTICAST_DELEGATE(FConnected);
    DECLARE_MULTICAST_DELEGATE_OneParam(FFailed, ErrorCode);
    DECLARE_MULTICAST_DELEGATE_OneParam(FDisconnected, const FString&);
    FConnected OnConnected;
    FFailed OnFailed;
    FDisconnected OnDisconnected;
protected:
    BaseDirectStream();
    std::shared_ptr<FMqasDirectStreamState> State;
private:
    friend struct FMqasPeerState;
    friend class FMqasPeer;
};
