#pragma once

#include "gsync/core/connection.h"
#include <utility>

// Owns an SDK socket handle, not the socket itself. Handles can be released
// independently even when they refer to the same underlying socket.
class MQASNET_API FMqasSocketHandle
{
public:
    FMqasSocketHandle() = default;
    ~FMqasSocketHandle() { Reset(); }
    FMqasSocketHandle(const FMqasSocketHandle&) = delete;
    FMqasSocketHandle& operator=(const FMqasSocketHandle&) = delete;
    FMqasSocketHandle(FMqasSocketHandle&& Other) noexcept : Handle(std::exchange(Other.Handle, GSY_INVALID_SOCKET)) {}
    FMqasSocketHandle& operator=(FMqasSocketHandle&& Other) noexcept
    {
        if (this != &Other) { Reset(); Handle = std::exchange(Other.Handle, GSY_INVALID_SOCKET); }
        return *this;
    }

    ErrorCode Acquire(GSY_ConnectionHwnd Connection)
    {
        GSY_SocketHandle NewHandle = GSY_INVALID_SOCKET;
        const auto Code = GSY_GetConnectionSocket(Connection, &NewHandle);
        if (Code == EC_Ok) { Reset(); Handle = NewHandle; }
        return Code;
    }
    ErrorCode Compare(const FMqasSocketHandle& Other, bool& Same) const
    {
        int Result = 0;
        const auto Code = GSY_AreSameSocket(Handle, Other.Handle, &Result);
        Same = Code == EC_Ok && Result != 0;
        return Code;
    }
    void Reset()
    {
        if (Handle != GSY_INVALID_SOCKET) GSY_ReleaseSocket(std::exchange(Handle, GSY_INVALID_SOCKET));
    }
    GSY_SocketHandle Get() const { return Handle; }
    explicit operator bool() const { return Handle != GSY_INVALID_SOCKET; }

private:
    GSY_SocketHandle Handle = GSY_INVALID_SOCKET;
};
