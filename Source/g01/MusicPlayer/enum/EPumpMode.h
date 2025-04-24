#pragma once

UENUM(BlueprintType)
enum class EPumpMode : uint8
{
    None = 0    UMETA(DisplayName = "None"),
    Loop = 1    UMETA(DisplayName = "Loop"),
    Rand = 2    UMETA(DisplayName = "Rand")
};

