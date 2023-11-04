// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCThing.generated.h"

/**
 * 
 */
class AAPlatformerCharacter;
UCLASS()
class APLATFORMER_API APCThing : public APlayerController
{
	GENERATED_BODY()
  public:
  //Creates a widget to show checkpoint reached
  UFUNCTION(BlueprintImplementableEvent, Category = Checkpoint)
  void CreateCheckpointReached();

  //Creates a widget to show respawn screen
  UFUNCTION(BlueprintImplementableEvent, Category = Checkpoint)
  void CreateRespawnScreen();

  //Creates a widget to show sprinting is unlocked
  UFUNCTION(BlueprintImplementableEvent, Category = Ability)
  void CreateSprint();

  //Creates a widget to show climbing is unlocked
  UFUNCTION(BlueprintImplementableEvent, Category = Ability)
  void CreateClimbing();

  //Creates a widget to show sliding is unlocked
  UFUNCTION(BlueprintImplementableEvent, Category = Ability)
  void CreateSliding();

  //Creates a widget to show double jumping is unlocked
  UFUNCTION(BlueprintImplementableEvent, Category = Ability)
  void CreateDoubleJump();

  //Creates/removes a widget for a tip: crouch
  UFUNCTION(BlueprintImplementableEvent, Category = Tips)
  void CreateTip(FName TipType);

  UFUNCTION(BlueprintImplementableEvent, Category = Tips)
  void RemoveTip(FName TipType);

  UFUNCTION(BlueprintImplementableEvent, Category = Save)
  void CreateSavedGame(const FString & SlotName, int32 SlotNumber, bool success);

  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Menu)
  void CreatePausedMenu();

  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Menu)
  void RemovePausedMenu();

  UPROPERTY(BlueprintReadWrite, Category = Menu)
  bool bIsPaused = false;
};
