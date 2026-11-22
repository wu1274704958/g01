#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "MqasP2P.h"
#include "MqasAgent.h"
#include "Async/Async.h"

namespace {
class FTrackingStream : public BaseStream
{
public:
    int32 CloseCount = 0;
protected:
    void OnClose() override { ++CloseCount; BaseStream::OnClose(); }
};
class FTestConnection : public BaseConnect
{
public:
    void AddStream(GSY_StreamId Id, const std::shared_ptr<BaseStream>& Stream)
    {
        Stream->InitData(Id, 123);
        _StreamMap.emplace(Id, Stream);
    }
    void SimulateDisconnect() { OnDisconnect(EC_Disconnected); }
    void SimulateStreamClose(GSY_StreamId Id) { OnStreamClose(Id, EC_Ok); }
};
class FTestLobbyStream : public P2PHelperStream
{
public:
    void CloseForTest() { OnClose(); }
};
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
    auto Peer = FMqasPeer::Create(); auto Datagram = FMqasDatagram::Create(); auto Other = FMqasPeer::Create();
    TestFalse(TEXT("Peer begins disconnected"), Peer->IsConnected());
    TestFalse(TEXT("Datagram begins disconnected"), Datagram->IsConnected());
    TestTrue(TEXT("Attach datagram"), Peer->SetDatagram(Datagram));
    auto Second = FMqasDatagram::Create();
    TestFalse(TEXT("A peer cannot attach a second datagram"), Peer->SetDatagram(Second));
    TestFalse(TEXT("Datagram cannot attach to two peers"), Other->SetDatagram(Datagram));
    TestEqual(TEXT("Cannot open before transport"), Datagram->Open(), EC_Fail);
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
#endif
