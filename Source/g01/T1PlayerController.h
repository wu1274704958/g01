// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "T1PlayerController.generated.h"


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class G01_API AT1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AT1PlayerController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable,Category="Input")
	FVector GetInputDirection();

protected:
	virtual void SetupInputComponent() override;
	void SetEnhancedInputSystem() const;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category="Input")
	TSoftObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, Category="Input")
	TSoftObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, Category="Input")
	TSoftObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditAnywhere, Category="Input")
	float TurnRateGamepad;

	UPROPERTY(EditAnywhere, Category="Input")
	float JumpForce;

	UPROPERTY(EditAnywhere,Category="Steering")
	float SteeringRate;
	
	UPROPERTY(EditAnywhere,Category="Steering")
	float SteeringSpeedRate;

	FVector2d InputDirection;
	/** Called for forwards/backward input */
	void MoveForward(float Value) const;

	/** Called for side to side input */
	void MoveRight(float Value) const;

	void OnMoveTriggered(const FInputActionValue&);
	void OnLookTriggered(const FInputActionValue&);
	void OnJumpTriggered(const FInputActionValue&);
};
