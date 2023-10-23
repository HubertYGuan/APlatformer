// Fill out your copyright notice in the Description page of Project Settings.


#include "SlidingGear.h"
#include "PCThing.h"

// Sets default values
ASlidingGear::ASlidingGear()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASlidingGear::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASlidingGear::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	CharacterRef->SetSlidingTrue();
	CharacterRef->GetPCRef()->CreateSliding();
	Destroy();
}

// Called every frame
void ASlidingGear::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

