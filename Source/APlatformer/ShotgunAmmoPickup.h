// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "ShotgunAmmoPickup.generated.h"

UCLASS()
class APLATFORMER_API AShotgunAmmoPickup : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShotgunAmmoPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

	UPROPERTY()
	int Ammo = 12;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
