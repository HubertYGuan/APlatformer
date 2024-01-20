// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableDevice.h"

void AInteractableDevice::StartInteract_Implementation(AAPlatformerCharacter *CharacterRef)
{
}

void AInteractableDevice::CancelInteract_Implementation(AAPlatformerCharacter *CharacterRef)
{
}

void AInteractableDevice::FinishInteract_Implementation(AAPlatformerCharacter *CharacterRef)
{
  if (!HoveringCharacter)
  {
    return;
  }
  HoveringCharacter->SetIsInteractingFalse();
  if (HoveringCharacter->GetHoveredPosition() == FVector())
  {
    return;
  }
  EndHover(HoveringCharacter);
  //hovering cooldown
  bOnCooldown = true;
  if (AutoCooldownResetOverride)
  {
    return;
  }
  GetWorld()->GetTimerManager().SetTimer(CooldownHandle, this, &AInteractable::SetOnCooldownFalse, 0.5, false);
}

void AInteractableDevice::StartHover_Implementation(AAPlatformerCharacter *CharacterRef)
{
  //call this parent function first
  HoveringCharacter = CharacterRef;
}

void AInteractableDevice::EndHover_Implementation(AAPlatformerCharacter *CharacterRef)
{
  if (!HoveringCharacter)
  {
    return;
  }
  if (HoveringCharacter->GetIsInteracting())
  {
  CancelInteract(HoveringCharacter);
  }
  //call this parent function first
  HoveringCharacter = nullptr;
}
