// Fill out your copyright notice in the Description page of Project Settings.


#include "GlobalMusicPlayerActor.h"


void AGlobalMusicPlayerActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (AutoInitialize)
        InitMusicPlayer();
}

// Sets default values
AGlobalMusicPlayerActor::AGlobalMusicPlayerActor(): AutoInitialize(false), AutoPlay(false)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGlobalMusicPlayerActor::BeginPlay()
{
    Super::BeginPlay();

    if (IsInitMusicPlayer() && AutoPlay)
    {
        Pumper->set_Mode(GetPumpModeRaw());
        Pumper->play();
    }
}

void AGlobalMusicPlayerActor::BeginDestroy()
{
    Super::BeginDestroy();
    UninitMusicPlayer();
}

// Called every frame
void AGlobalMusicPlayerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (Pumper)
        Pumper->tick();
}

bool AGlobalMusicPlayerActor::InitMusicPlayer()
{
    if(IsInitMusicPlayer())
        return true;
        
    Player = std::make_shared<MusicPlayer>();
    Pumper = std::make_shared<PlaylistMgr>(*Player);

    const std::wstring path(*MusicRootPath);
    Pumper->initInfoProvider(AutoPlay,path);
    return true;
}

bool AGlobalMusicPlayerActor::IsInitMusicPlayer()
{
    return static_cast<bool>(Player);
}

void AGlobalMusicPlayerActor::UninitMusicPlayer()
{
    if (Player)
    {
        Pumper.reset();
        Player.reset();
    }
}

void AGlobalMusicPlayerActor::SetMusicRootPath(const FString& path)
{
    MusicRootPath = path;
}

eqd_mp::EPumpMode AGlobalMusicPlayerActor::GetPumpModeRaw() const
{
    return static_cast<eqd_mp::EPumpMode>(PumpMode);
}

const std::shared_ptr<MusicPlayer>& AGlobalMusicPlayerActor::GetPlayer() const
{
    return Player;
}

const std::shared_ptr<PlaylistMgr>& AGlobalMusicPlayerActor::GetPlaylistMgr() const
{
    return Pumper;
}

const FString& AGlobalMusicPlayerActor::GetMusicRootPath() const
{
    return MusicRootPath;
}

