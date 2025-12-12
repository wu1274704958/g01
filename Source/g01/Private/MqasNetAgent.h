// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MqasNetAgent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMqasNet, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConnectedDelegate,int,code,int,peer_id);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestDelegate,const FString&,name,int,id);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnErrorDelegate,const FString&,msg,int,code);

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
	UFUNCTION(BlueprintCallable,Category="MqasNetAgent")
	int GetSelfId() const { return SelfId; }
	UFUNCTION(BlueprintCallable,Category="MqasNetAgent")
	bool IsConnectToHolePunchingServer() const { return SelfId != 0; }
private:
	void OnConnectedCallback(int code,unsigned int peer_id);
	void OnRequestCallback(const FString& name,unsigned int id);
	void OnErrorCallback(const FString& msg,int code);
private:
	static AUMqasNetAgent* instance;
	static void OnConnectedCallbackGlobal(int code,unsigned int peer_id);
	static void OnRequestCallbackGlobal(struct PeerData* peer_data);
	static void OnErrorCallbackGlobal(const char* msg,int code);
public:
	UPROPERTY(BlueprintAssignable,Category="MqasNetAgent")
	FOnConnectedDelegate OnConnected;
	UPROPERTY(BlueprintAssignable,Category="MqasNetAgent")
	FOnRequestDelegate OnRequest;
	UPROPERTY(BlueprintAssignable,Category="MqasNetAgent")
	FOnErrorDelegate OnError;
private:
	bool bInitialized = false;
	unsigned int SelfId = 0;
};
