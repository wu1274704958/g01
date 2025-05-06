// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pumper.h"
#include <memory>
#include "music_player/music_player.h"
#include "GlobalMusicPlayer.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class G01_API UGlobalMusicPlayer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGlobalMusicPlayer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
    virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    

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


protected:
    std::shared_ptr<MusicPlayer> player;
    std::shared_ptr<PlaylistMgr> pumper;

    UPROPERTY(EditAnywhere, Category = "Music")
    FString MusicRootPath;
    UPROPERTY(EditAnywhere, Category = "Music")
    bool AutoInitialize;
    UPROPERTY(EditAnywhere, Category = "Music")
    bool AutoPlay;

};
