// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class APLATFORMER_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()
	
	public:
	//the custom save name set by the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString CustomName;

	//all of the gear
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gear)
	bool bHasShoes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gear)
	bool bHasClimbingGear;

	//spawn info
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Spawn)
	FVector LastCheckpointPos = FVector(0,0,0);

	//level of last checkpoint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Level)
	int LastCheckpointLevel = 0;

	//Player Stats (physical not like scores)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerStats)
	double MaxSpeedDefault;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerStats)
	double JumpVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Level)
	int LevelUnlocked;

	//the current level
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Level)
	int LevelNumber;
};
