// Fill out your copyright notice in the Description page of Project Settings.


#include "DoubleJumpOrb.h"

// Sets default values
ADoubleJumpOrb::ADoubleJumpOrb()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADoubleJumpOrb::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADoubleJumpOrb::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	CharacterRef->DoubleJumpOrbOverlap();
	Destroy();
}

// Called every frame
void ADoubleJumpOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

