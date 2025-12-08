// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MqasNetAgent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMqasNet, Log, All);

/**
 * 
 */
UCLASS()
class G01_API AUMqasNetAgent : public AActor
{
	GENERATED_BODY()

public:
	AUMqasNetAgent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	UFUNCTION(BlueprintCallable,Category="MqasNetAgent")
	bool ConnectToHolePunchingServer(const FString& name, const FString& psd);
	UFUNCTION(BlueprintCallable,Category="MqasNetAgent")
	bool DisconnectToHolePunchingServer();
	UFUNCTION(BlueprintCallable,Category="MqasNetAgent")
	bool IsInitialized() const { return bInitialized; }
	unsigned int GetSelfId() const { return SelfId; }

private:
	static AUMqasNetAgent* instance;
	static void OnConnectedCallback(int code,unsigned int peer_id);
	static void OnRequestCallback(struct PeerData* peer_data);
	static void OnErrorCallback(const char* msg,int code);

private:
	bool bInitialized = false;
	unsigned int SelfId = 0;
};
