// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.h"
#include "APlatformerCharacter.h"
#include "KeyPickup.generated.h"

UCLASS()
class APLATFORMER_API AKeyPickup : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKeyPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

  virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

  //the type of key this is. can be portal, destroy, or create (actor) (may have animations for these)
  UPROPERTY(BlueprintReadWrite)
  FName KeyType;

  //the actor(s) it is bound to (array)
  UPROPERTY(BlueprintReadWrite)
  TArray<AActor*> BoundActors;

};
