#pragma once

#include "BaseDirectStream.h"
#include <atomic>

struct FMqasDirectStreamState {
    std::weak_ptr<FMqasPeerState> Peer;
    std::weak_ptr<BaseDirectStream> Owner;
    std::atomic<GSY_StreamId> Stream{INVALID_SID};
    std::atomic<GSY_ConnectionHwnd> Connection{InvalidConnection};
    std::atomic<bool> Connected{false}, Opened{false};
    GSY_P2PStreamId LocalId = 0;
    bool Incoming = false;
};
