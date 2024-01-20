// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractablePickup.h"

void AInteractablePickup::StartInteract_Implementation(AAPlatformerCharacter *CharacterRef)
{
}

void AInteractablePickup::CancelInteract_Implementation(AAPlatformerCharacter *CharacterRef)
{
}

void AInteractablePickup::FinishInteract(AAPlatformerCharacter *CharacterRef)
{
  HoveringCharacter->SetIsInteractingFalse();
  EndHover(HoveringCharacter);

  AddToInventory();
}

void AInteractablePickup::StartHover_Implementation(AAPlatformerCharacter *CharacterRef)
{
  //call this parent function first
  HoveringCharacter = CharacterRef;
}

void AInteractablePickup::EndHover_Implementation(AAPlatformerCharacter *CharacterRef)
{
  //call this parent function first
  HoveringCharacter = nullptr;
}

void AInteractablePickup::AddToInventory()
{
  //TODO add the add to inventory functions for each type of pickup
  switch (PickupType)
  {
  case EPickup::Shotgun:
    /* code */
    break;
  
  default:
    break;
  }
}