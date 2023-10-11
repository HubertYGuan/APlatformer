// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OverlapInterface.h"
#include "APlatformerCharacter.h"
#include "JumpPad.generated.h"

UCLASS()
class APLATFORMER_API AJumpPad : public AActor, public  IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJumpPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  //The boost velocity this jump pad provides
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
  float BoostVelocity;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

  //override of overlap action
  virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

};
