// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "MainGI.generated.h"

DECLARE_DYNAMIC_DELEGATE(FLoadSaveDelegate);

/**
 * 
 */
class UMySaveGame;

UCLASS()
class APLATFORMER_API UMainGI : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;

	public:
	//loads the last auto save by default
	UPROPERTY(VisibleAnywhere, BLueprintReadWrite, Category = Save)
	FString SelectedSaveSlot = "LastAutoSave";

	UPROPERTY(VisibleAnywhere, BLueprintReadWrite, Category = Save)
	int SelectedSaveIndex = -1;

	//SaveSlotArray contains last auto save, but you must subtract one from the index to get the save index
	UPROPERTY(VisibleAnywhere, BLueprintReadOnly, Category = Save)
	TArray<FString> SaveSlotArray;

	UPROPERTY(VisibleAnywhere, BLueprintReadWrite, Category = Save)
	TArray<int> DeletedIndexArray;

	UPROPERTY(VisibleAnywhere, BLueprintReadOnly, Category = Save)
	int HighestSaveIndex = -1;
	
	UPROPERTY(BlueprintReadOnly, Category = Level)
  int LevelUnlocked = 1;

  UPROPERTY(BlueprintReadWrite, Category = Level)
  int LevelNumber = 0;

	UPROPERTY(VisibleAnywhere, BLueprintReadOnly, Category = Save)
	UMySaveGame *LoadedSave;

	UPROPERTY(BlueprintReadOnly)
	FLoadSaveDelegate LoadSaveDelegate;

	//Called on game init and whenever saves are added/removed
	UFUNCTION(BlueprintCallable, Category = Save)
	void UpdateSaveArrayAndIndex();

	//called before the APlatformerCharacter method LoadSave is called, handles cross-level save logic and automatically opens the LoadingSaveLevel
	UFUNCTION(BlueprintCallable, Category = Save)
	void LoadSave();

	UFUNCTION()
	void NotifySaveSystemDataCreated(const FString & SlotName, int32 SlotNumber, bool success);
};
