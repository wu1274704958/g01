// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MusicPlayer/enum/EPumpMode.h"
#include "music_player/music_player.h"
#include "GlobalMusicPlayerActor.generated.h"

UCLASS()
class G01_API AGlobalMusicPlayerActor : public AActor
{
public:
    virtual void PostInitializeComponents() override;

private:
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AGlobalMusicPlayerActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    UFUNCTION(BlueprintCallable, Category = "Music")
    bool InitMusicPlayer();
    UFUNCTION(BlueprintCallable, Category = "Music")
    bool IsInitMusicPlayer();
    UFUNCTION(BlueprintCallable, Category = "Music")
    void UninitMusicPlayer();

    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicRootPath(const FString& path);

    UFUNCTION(BlueprintCallable, Category = "Music")
    const FString& GetMusicRootPath() const;

    eqd_mp::EPumpMode GetPumpModeRaw() const;

    [[nodiscard]] const std::shared_ptr<MusicPlayer>& GetPlayer() const;
    [[nodiscard]] const std::shared_ptr<PlaylistMgr>& GetPlaylistMgr() const;

protected:
    std::shared_ptr<MusicPlayer> Player;
    std::shared_ptr<PlaylistMgr> Pumper;

    UPROPERTY(EditAnywhere, Category = "Music")
    FString MusicRootPath;
    UPROPERTY(EditAnywhere, Category = "Music")
    bool AutoInitialize;
    UPROPERTY(EditAnywhere, Category = "Music")
    bool AutoPlay;
    UPROPERTY(EditAnywhere, Category = "Music")
    EPumpMode PumpMode = EPumpMode::Loop;
};
