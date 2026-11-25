#include "BaseDirectStream.h"
#include "DirectStreamState.h"
#include "gsync/example1/p2p_datagram.h"

BaseDirectStream::BaseDirectStream() : State(std::make_shared<FMqasDirectStreamState>()) {}
BaseDirectStream::~BaseDirectStream() = default;
bool BaseDirectStream::IsConnected() const { return State->Connected; }
GSY_P2PStreamId BaseDirectStream::GetLocalStreamId() const { return State->LocalId; }
ErrorCode BaseDirectStream::GetStreamIds(GSY_P2PStreamId& OutLocal, GSY_P2PStreamId& OutRemote) const {
    check(IsInGameThread()); OutLocal = 0; OutRemote = 0;
    return State->Connected ? GSY_GetPeerStreamIds(State->Connection, State->Stream, &OutLocal, &OutRemote) : EC_Fail;
}
