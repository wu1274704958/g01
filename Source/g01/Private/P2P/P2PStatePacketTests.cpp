#if WITH_DEV_AUTOMATION_TESTS
#include "P2PStatePacket.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FP2PStatePacketTest, "g01.P2P.StatePacket", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FP2PStatePacketTest::RunTest(const FString&)
{
    FP2PStatePacket Original; Original.Sequence = 42;
    Original.Position = FVector(1, 2, 3); Original.Velocity = FVector(4, 5, 6);
    Original.Rotation = FRotator(10, 20, 30);
    auto Data = Original.Encode(); FP2PStatePacket Decoded;
    TestTrue(TEXT("Valid state"), FP2PStatePacket::Decode(Data, Decoded));
    TestEqual(TEXT("Sequence"), Decoded.Sequence, Original.Sequence);
    TestEqual(TEXT("Position"), Decoded.Position, Original.Position);
    TestEqual(TEXT("Rotation"), Decoded.Rotation, Original.Rotation);
    TestEqual(TEXT("Velocity"), Decoded.Velocity, Original.Velocity);
    Data.Pop(); TestFalse(TEXT("Truncated state"), FP2PStatePacket::Decode(Data, Decoded));
    Data = Original.Encode(); Data[0] ^= 1; TestFalse(TEXT("Invalid magic"), FP2PStatePacket::Decode(Data, Decoded));
    Data = Original.Encode(); Data.Add(0); TestFalse(TEXT("Trailing data"), FP2PStatePacket::Decode(Data, Decoded));
    TestFalse(TEXT("Duplicate sequence"), FP2PStatePacket::IsNewer(42, 42));
    TestFalse(TEXT("Older sequence"), FP2PStatePacket::IsNewer(41, 42));
    TestTrue(TEXT("Sequence wrap"), FP2PStatePacket::IsNewer(0, MAX_uint32));
    return true;
}
#endif
