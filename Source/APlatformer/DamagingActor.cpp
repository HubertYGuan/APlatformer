#include "DamagingActor.h"
#include "Engine/DamageEvents.h"
// Fill out your copyright notice in the Description page of Project Settings.

// Sets default values
ADamagingActor::ADamagingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADamagingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADamagingActor::DamageTick()
{
	if (!CurrentCharacterRef)
	{
		return;
	}
	CurrentCharacterRef->TakeDamage(DamagePerTick, FDamageEvent(), nullptr, this);
}

// Called every frame
void ADamagingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void ADamagingActor::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	CurrentCharacterRef = CharacterRef;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ADamagingActor::DamageTick, TickRate, true);
}

void ADamagingActor::EndOverlapAction(AAPlatformerCharacter *CharacterRef)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	CurrentCharacterRef = nullptr;
}
