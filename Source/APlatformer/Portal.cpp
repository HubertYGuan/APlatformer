// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APortal::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	//check if the portal is activated
	if (bIsActivated)
	{
	//check the type
	if (PortalType == "Level")
	{
		UMainGI *GIRef = CharacterRef->GetGIRef();
		GIRef->LevelNumber = LevelNumber;
		if (GIRef->LevelUnlocked<LevelNumber)
		{
			GIRef->LevelUnlocked = LevelNumber;
		}
		if (bShouldAutoSave)
		{
			GIRef->SelectedSaveIndex = -1;
			GIRef->SelectedSaveSlot = FString("LastAutoSave");
			GIRef->LoadSaveDelegate.BindDynamic(GIRef, &UMainGI::LoadSave);
			CharacterRef->CreateSave("LastAutoSave", -1, "LastAutoSave", false);
		}
		else
		{
			UGameplayStatics::OpenLevel(this, LevelDestination);
		}
	}
	else
	{
	CharacterRef->SetActorLocationAndRotation(TeleportLocation, TeleportRotation);
	}
	}
}

void APortal::ActivatePortal_Implementation()
{
	bIsActivated = true;
}

void APortal::DeActivatePortal_Implementation()
{
	bIsActivated = false;
}
