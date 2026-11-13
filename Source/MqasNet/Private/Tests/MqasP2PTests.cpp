#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "MqasP2P.h"

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
    TestFalse(TEXT("Datagram cannot attach to two peers"), Other->SetDatagram(Datagram));
    TestEqual(TEXT("Cannot open before transport"), Datagram->Open(), EC_Fail);
    TArray<uint8> Data{1};
    TestEqual(TEXT("Cannot send before authentication"), Datagram->Send(Data), EC_Fail);
    return true;
}
#endif
