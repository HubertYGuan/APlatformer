// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "TipTrigger.generated.h"

UCLASS()
class APLATFORMER_API ATipTrigger : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ATipTrigger();

  //The type of tip (set in blueprint)
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
  FName TipType;

  //if the show tip function is bound to TimerDel
  bool bIsBound = false;

  //if the tip has been shown
  bool bIsTipShown = false;

  bool bCanRemoveTip = true;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  //override of interface function for overlap action
  virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

  //override of interface function for end overlap action
  virtual void EndOverlapAction(AAPlatformerCharacter *CharacterRef) override;

  //to show the tip widget, bound to TimerDel
  UFUNCTION()
  void ShowTip(AAPlatformerCharacter *CharacterRef);

  FTimerDelegate TimerDel;

  FTimerHandle TimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
