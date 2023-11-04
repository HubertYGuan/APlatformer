// Fill out your copyright notice in the Description page of Project Settings.


#include "DoubleJumpBoots.h"

// Sets default values
ADoubleJumpBoots::ADoubleJumpBoots()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADoubleJumpBoots::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADoubleJumpBoots::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	CharacterRef->DoubleJumpBootsPickup();
	Destroy();
}

// Called every frame
void ADoubleJumpBoots::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

