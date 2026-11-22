#include "P2PStateSyncComponent.h"
#include "P2PLinkController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "P2PStatePacket.h"

UP2PStateSyncComponent::UP2PStateSyncComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UP2PStateSyncComponent::InitializeLink(AP2PLinkController* InLink) { Link = InLink; }

void UP2PStateSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
    Super::TickComponent(DeltaTime, TickType, TickFunction);
    Elapsed += DeltaTime;
    auto* Controller = Link.Get();
    auto* Character = Cast<ACharacter>(GetOwner());
    if (!Controller || !Character || Elapsed < 1.f / 15.f) return;
    Elapsed = 0;
    FP2PStatePacket Packet;
    Packet.Sequence = Controller->NextStateSequence();
    Packet.Position = Character->GetActorLocation();
    Packet.Rotation = Character->GetActorRotation();
    Packet.Velocity = Character->GetVelocity();
    Controller->SendState(Packet.Encode());
}
