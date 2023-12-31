// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunZoneTrigger.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AWallRunZoneTrigger::AWallRunZoneTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWallRunZoneTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWallRunZoneTrigger::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	CharacterRef->SetCanWallRunTrue();
}

void AWallRunZoneTrigger::EndOverlapAction(AAPlatformerCharacter *CharacterRef)
{
	UKismetSystemLibrary::PrintString(this, "ending wallrunzone overlap");
	CharacterRef->SetCanWallRunFalse();
}

// Called every frame
void AWallRunZoneTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

