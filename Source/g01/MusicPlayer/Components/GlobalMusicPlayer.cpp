// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicPlayer/Components/GlobalMusicPlayer.h"

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
        pumper->setMode(GetPumpModeRaw());
        pumper->pump();
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

    if (!player->isOff() && player->getActive() == BASS_ACTIVE_STOPPED &&
        (pumper->getMode() != fv::Pumper::NONE || pumper->hasNextMusic()))
    {
        pumper->pump();
    }
}

bool UGlobalMusicPlayer::InitMusicPlayer()
{
    if(IsInitMusicPlayer())
        return true;
        
    player = std::make_shared<fv::MusicPlayer>();
    pumper = std::make_shared<fv::Pumper>(*player);

    pumper->init(*MusicRootPath);
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

fv::Pumper::PUMP_MODE UGlobalMusicPlayer::GetPumpModeRaw() const
{
    return fv::Pumper::RAND;
}

const FString& UGlobalMusicPlayer::GetMusicRootPath() const
{
    return MusicRootPath;
}

