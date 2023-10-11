// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystemData.generated.h"

/**
 * 
 */
UCLASS()
class APLATFORMER_API USaveSystemData : public USaveGame
{
	GENERATED_BODY()
	public:
	UPROPERTY(VisibleAnywhere, BLueprintReadOnly, Category = Save)
	int HighestSaveIndex;
};
