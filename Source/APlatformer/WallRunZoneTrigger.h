// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "WallRunZoneTrigger.generated.h"

UCLASS()
class APLATFORMER_API AWallRunZoneTrigger : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWallRunZoneTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//override of interface function for overlap action
  virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

  //override of interface function for end overlap action
  virtual void EndOverlapAction(AAPlatformerCharacter *CharacterRef) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
