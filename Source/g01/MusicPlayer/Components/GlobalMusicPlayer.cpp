// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicPlayer/Components/GlobalMusicPlayer.h"

#include "MusicPlayer/enum/EPumpMode.h"

// Sets default values for this component's properties
UGlobalMusicPlayer::UGlobalMusicPlayer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGlobalMusicPlayer::BeginPlay()
{
	Super::BeginPlay();

    if(AutoInitialize)
        InitMusicPlayer();
    if (IsInitMusicPlayer() && AutoPlay)
    {
        pumper->set_Mode(GetPumpModeRaw());
        pumper->play();
    }
	// ...
	
}

void UGlobalMusicPlayer::BeginDestroy()
{
    Super::BeginDestroy();
    UninitMusicPlayer();
}


// Called every frame
void UGlobalMusicPlayer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

    pumper->tick();
}

bool UGlobalMusicPlayer::InitMusicPlayer()
{
    if(IsInitMusicPlayer())
        return true;
        
    player = std::make_shared<MusicPlayer>();
    pumper = std::make_shared<PlaylistMgr>(*player);

    const std::wstring path(*MusicRootPath);
    pumper->initInfoProvider(AutoPlay,path);
    return true;
}

bool UGlobalMusicPlayer::IsInitMusicPlayer()
{
    return (bool)player;
}

void UGlobalMusicPlayer::UninitMusicPlayer()
{
    if (player)
    {
        pumper.reset();
        player.reset();
    }
}

void UGlobalMusicPlayer::SetMusicRootPath(const FString& path)
{
    MusicRootPath = path;
}

eqd_mp::EPumpMode UGlobalMusicPlayer::GetPumpModeRaw() const
{
    return eqd_mp::EPumpMode::Rand;
}

const FString& UGlobalMusicPlayer::GetMusicRootPath() const
{
    return MusicRootPath;
}

