// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OverlapInterface.h"
#include "APlatformerCharacter.h"
#include "SpeedShoes.generated.h"

UCLASS()
class APLATFORMER_API ASpeedShoes : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpeedShoes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  //Called when character overlaps with this
  UFUNCTION()
  virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
