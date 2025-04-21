// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T1Character.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class G01_API AT1Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AT1Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	void SetupCamera();
private:
	UPROPERTY(EditAnywhere,Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;
	UPROPERTY(EditAnywhere,Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
};
