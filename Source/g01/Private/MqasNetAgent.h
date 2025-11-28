// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MqasNetAgent.generated.h"

/**
 * 
 */
UCLASS()
class G01_API UMqasNetAgent : public UObject
{
	GENERATED_BODY()

public:
	UMqasNetAgent();
	virtual void BeginDestroy() override;
	virtual void PostInitProperties() override;

	bool ConnectToHolePunchingServer(const FString& name, const FString& psd);
	bool IsInitialized() const { return bInitialized; }
	unsigned int GetSelfId() const { return SelfId; }

private:
	static UMqasNetAgent* instance;
	static void OnConnectedCallback(int code,unsigned int peer_id);
	static void OnRequestCallback(struct PeerData* peer_data);
	static void OnErrorCallback(const char* msg,int code);

private:
	bool bInitialized = false;
	unsigned int SelfId = 0;
};
