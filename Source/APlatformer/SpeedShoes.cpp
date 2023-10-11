// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedShoes.h"

// Sets default values
ASpeedShoes::ASpeedShoes()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASpeedShoes::BeginPlay()
{
	Super::BeginPlay();
}

void ASpeedShoes::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
  CharacterRef->SetShoesTrue();
  CharacterRef->GetPCRef()->CreateSprint();
  Destroy();
}

// Called every frame
void ASpeedShoes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}