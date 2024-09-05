// Fill out your copyright notice in the Description page of Project Settings.


#include "Hoverable.h"

// Sets default values
AHoverable::AHoverable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHoverable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHoverable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHoverable::StartHover_Implementation(AAPlatformerCharacter *CharacterRef)
{
  //call this parent function first
  HoveringCharacter = CharacterRef;
}

void AHoverable::EndHover_Implementation(AAPlatformerCharacter *CharacterRef)
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