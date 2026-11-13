#pragma once
#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

struct FP2PStatePacket
{
    uint32 Sequence = 0;
    FVector Position = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    FVector Velocity = FVector::ZeroVector;

    TArray<uint8> Encode() const
    {
        TArray<uint8> Data;
        FMemoryWriter Writer(Data, true);
        uint32 Magic = 0x47303150, Version = 1, Seq = Sequence;
        FVector Pos = Position, Vel = Velocity;
        FRotator Rot = Rotation;
        Writer << Magic << Version << Seq << Pos << Rot << Vel;
        return Data;
    }
    static bool Decode(const TArray<uint8>& Data, FP2PStatePacket& Out)
    {
        if (Data.Num() > 256) return false;
        FMemoryReader Reader(Data, true);
        uint32 Magic = 0, Version = 0;
        FP2PStatePacket Packet;
        Reader << Magic << Version << Packet.Sequence << Packet.Position << Packet.Rotation << Packet.Velocity;
        if (Reader.IsError() || !Reader.AtEnd() || Magic != 0x47303150 || Version != 1 ||
            Packet.Position.ContainsNaN() || Packet.Rotation.ContainsNaN() || Packet.Velocity.ContainsNaN()) return false;
        Out = Packet;
        return true;
    }
    static bool IsNewer(uint32 Sequence, uint32 Previous) { return static_cast<int32>(Sequence - Previous) > 0; }
};
