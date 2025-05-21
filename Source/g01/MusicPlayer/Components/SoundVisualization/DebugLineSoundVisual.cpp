// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugLineSoundVisual.h"

#include "IntVectorTypes.h"

#ifdef min
#undef  min
#endif


// Sets default values for this component's properties
UDebugLineSoundVisual::UDebugLineSoundVisual()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UDebugLineSoundVisual::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}

void UDebugLineSoundVisual::onVisualizationData(const std::span<float>& data)
{
    Super::onVisualizationData(data);

    auto len = std::min(points.size(),data.size());
    for (auto i = 0; i < len; i++)
    {
        DrawDebugCircle(GetWorld(),GetOwner()->GetActorLocation() + points[i] ,
            BaseSize + sqrtf(data[i]) * SizeScale,7,
            BaseColor,false,BaseDuration,0,1,FVector(0,1,0),
            FVector(1,0,0),false);
    }
    
}

void UDebugLineSoundVisual::onRegisted(eqd_mp::EFFTDataType type, size_t dataSize)
{
    Super::onRegisted(type, dataSize);

    points.resize(dataSize);

    //random points in sphere
    for (int32 i = 0; i < static_cast<int32>(dataSize); ++i)
    {
        // 1. 生成随机方向（单位向量）
        FVector Direction = FMath::VRand();
        
        // 2. 生成随机半径比例（立方根保证球体内均匀分布）
        float RandomProportion = FMath::Pow(FMath::FRand(), 1.0f/3.0f);
        
        // 3. 计算最终坐标
        points[i] = Direction * (RandomProportion * SphereRadius);
    }
    
}


// Called every frame
void UDebugLineSoundVisual::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

eqd_mp::EFFTDataType UDebugLineSoundVisual::GetFFTDataType() const
{
    return eqd_mp::EFFTDataType::FFT1024;
}

