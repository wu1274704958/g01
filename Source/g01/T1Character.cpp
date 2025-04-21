// Fill out your copyright notice in the Description page of Project Settings.


#include "T1Character.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AT1Character::AT1Character()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = bUseControllerRotationRoll = bUseControllerRotationYaw = false;
	SetupCamera();
}

void AT1Character::SetupCamera()
{
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));

	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->CameraLagSpeed = 2.5f;
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent->SetupAttachment(SpringArmComponent,USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	SpringArmComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AT1Character::BeginPlay()
{
	Super::BeginPlay();
}



// Called every frame
void AT1Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}