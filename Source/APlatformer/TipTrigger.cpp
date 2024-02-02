// Fill out your copyright notice in the Description page of Project Settings.


#include "TipTrigger.h"
#include "PCThing.h"

// Sets default values
ATipTrigger::ATipTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATipTrigger::BeginPlay()
{
	Super::BeginPlay();
}

void ATipTrigger::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
  if (!bIsTipShown)
  {
  if (!bIsBound)
  {
	  TimerDel.BindUFunction(this, FName("ShowTip"), CharacterRef);
    bIsBound = true;
  }
  GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 10.f, false);
  }
}

void ATipTrigger::EndOverlapAction(AAPlatformerCharacter *CharacterRef)
{
  if (!bIsTipShown)
  {
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
  }
  else if (bCanRemoveTip)
  {
  CharacterRef->GetPCRef()->RemoveTip(TipType);
  bCanRemoveTip = false;
  }
}

void ATipTrigger::ShowTip(AAPlatformerCharacter *CharacterRef)
{
  CharacterRef->GetPCRef()->CreateTip(TipType);
  bIsTipShown = true;
}

// Called every frame
void ATipTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

