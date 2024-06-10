// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "GunPickup.generated.h"

class UGunComponent;

UCLASS()
class APLATFORMER_API AGunPickup : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UGunComponent *GunComponent;
};
