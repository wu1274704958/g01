// Fill out your copyright notice in the Description page of Project Settings.


#include "T1PlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "T1Character.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "T1PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


void AT1PlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetEnhancedInputSystem();
}


AT1PlayerController::AT1PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
	: APlayerController(ObjectInitializer)
{
	TurnRateGamepad = 50.0f;
	SteeringRate = 50.0f;
    WalkSpeedRate = 0.6f;
    RunSpeedRate = 0.8f;
	PlayerCameraManagerClass = AT1PlayerCameraManager::StaticClass();
	//SetupCamera();
}

FVector AT1PlayerController::GetInputDirection()
{
	return FVector(InputDirection,0);
}

void AT1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	auto inputComp = Cast<UEnhancedInputComponent>(InputComponent);
	if(inputComp)
	{
		if(IA_Move)inputComp->BindAction(IA_Move.Get(),ETriggerEvent::Triggered,this,&AT1PlayerController::OnMoveTriggered);
		if(IA_Look)inputComp->BindAction(IA_Look.Get(),ETriggerEvent::Triggered,this,&AT1PlayerController::OnLookTriggered);
		if(IA_Jump)inputComp->BindAction(IA_Jump.Get(),ETriggerEvent::Triggered,this,&AT1PlayerController::OnJumpTriggered);
	    if(IA_IsRunning)inputComp->BindAction(IA_IsRunning.Get(),ETriggerEvent::Triggered,this,&AT1PlayerController::OnIsRunningTrigged);
	}
}

void AT1PlayerController::SetEnhancedInputSystem() const
{
	if (auto InputSystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (!InputMapping.IsNull())
		{
			InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
		}
	}
}


void AT1PlayerController::OnMoveTriggered(const FInputActionValue& v)
{
	if(GetCharacter()->GetVelocity().Z != 0)
		return;
	const auto Axis = v.Get<FVector2d>();
	InputDirection = Axis.GetSafeNormal();
	const FRotator YawRotation(0, GetControlRotation().Yaw, 0);
	const auto Dir = YawRotation.RotateVector(FVector(InputDirection,0));

	const auto Rot = Dir.ToOrientationRotator();
	const auto ActorDir = GetCharacter()->GetActorRotation().Vector().GetSafeNormal();
	
	GetCharacter()->SetActorRotation(FMath::RInterpTo(GetCharacter()->GetActorRotation(),Rot,GetWorld()->GetDeltaSeconds(),SteeringRate));

	GetCharacter()->AddMovementInput(Dir, InputDirection.Length() * FMath::Clamp(ActorDir.Dot(Dir),0,1) *
	    IsRunning ? RunSpeedRate : WalkSpeedRate);
}

void AT1PlayerController::OnLookTriggered(const FInputActionValue& v)
{
	auto axis = v.Get<FVector>();
	const auto delta =  GetWorld()->GetDeltaSeconds();
	GetCharacter()->AddControllerYawInput(axis.X * TurnRateGamepad * delta);
	GetCharacter()->AddControllerPitchInput(axis.Y * TurnRateGamepad * delta);
}

void AT1PlayerController::OnIsRunningTrigged(const FInputActionValue& v)
{
    IsRunning = !IsRunning;
}

void AT1PlayerController::MoveForward(float Value) const
{
	const FRotator YawRotation(0, GetControlRotation().Yaw, 0);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	GetCharacter()->AddMovementInput(Direction, Value);
}

void AT1PlayerController::MoveRight(float Value) const
{
	const FRotator YawRotation(0, GetControlRotation().Yaw, 0);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	GetCharacter()->AddMovementInput(Direction, Value);
}

void AT1PlayerController::OnJumpTriggered(const FInputActionValue& v)
{
	GetCharacter()->GetCharacterMovement()->DoJump(false);
}
