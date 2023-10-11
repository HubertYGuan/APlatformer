// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGI.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MySaveGame.h"
#include "SaveSystemData.h"

void UMainGI::Init()
{
  //load a savesystemdata file to get an accurate last highestsaveindex
  if (USaveSystemData *LoadedSaveThing = Cast<USaveSystemData>(UGameplayStatics::LoadGameFromSlot(FString("SaveSystemData"), -2)))
  {
    HighestSaveIndex = LoadedSaveThing->HighestSaveIndex;
  }
  else
  {
    UKismetSystemLibrary::PrintString(this, "Failed to load SaveSystemData");
  }
  //loads last auto save
  if (UMySaveGame *LoadedSaveThing = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SelectedSaveSlot, SelectedSaveIndex)))
  {
    LoadedSave = LoadedSaveThing;
    LevelNumber = LoadedSave->LevelNumber;
    LevelUnlocked = LoadedSave->LevelUnlocked;
  }
  else
  {
    //the game save system kind of breaks if LastAutoSave doesn't exist
    UKismetSystemLibrary::PrintString(this, "Automatically creating LastAutoSave");
    if (UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())))
    {
      //Set all of the save data to defualt
      SaveGameInstance->bHasShoes = false;
      SaveGameInstance->bHasClimbingGear = false;
      SaveGameInstance->MaxSpeedDefault = 500.f;
      SaveGameInstance->JumpVelocity = 450.f;

      SaveGameInstance->CustomName = "LastAutoSave";
      SaveGameInstance->LevelUnlocked = 1;
      SaveGameInstance->LevelNumber = 1;

      UGameplayStatics::SaveGameToSlot(SaveGameInstance, SelectedSaveSlot, SelectedSaveIndex);
    }
    else
    {
      UKismetSystemLibrary::PrintString(this, "Failed to create LastAutoSave");
    }
  }
  UpdateSaveArrayAndIndex();
}

void UMainGI::UpdateSaveArrayAndIndex()
{
  SaveSlotArray.Empty();

  if (UGameplayStatics::DoesSaveGameExist(FString("LastAutoSave"), -1))
  {
    SaveSlotArray.Add(FString("LastAutoSave"));
  }

  int i = 0;

  //HighestSaveIndex should only be able to increment by one unless someone manually tampers with the save files
  //but HighestSaveIndex can decrement by any amount
  for (i; i<=HighestSaveIndex+1; i++)
  {
    if (UGameplayStatics::DoesSaveGameExist(FString("SaveSlot_")+FString::FromInt(i), i))
    {
      UKismetSystemLibrary::PrintString(this, "bing bing bing save with index "+FString::FromInt(i));
      SaveSlotArray.Add(FString("SaveSlot_")+FString::FromInt(i));
    }
    else if (i>=HighestSaveIndex)
    {
      //this means the highest save index was decremented by some amount or the highest save index stayed the same
      while (SaveSlotArray.Last(0)==FString("DeletedSlot"))
      {
        SaveSlotArray.Pop();
        i--;
      }
      break;
    }
    else
    {
      //create a deleted slot placeholder, should be ignored by menu
      UKismetSystemLibrary::PrintString(this, "Creating deleted slot at "+FString::FromInt(i));
      SaveSlotArray.Add(FString("DeletedSlot"));
    }
  }

  HighestSaveIndex = i-1;

  //save SaveSystemData
  if (USaveSystemData* SaveGameInstance = Cast<USaveSystemData>(UGameplayStatics::CreateSaveGameObject(USaveSystemData::StaticClass())))
  {
    FAsyncSaveGameToSlotDelegate SavedDelegate;
    SavedDelegate.BindUObject(this, &UMainGI::NotifySaveSystemDataCreated);

    UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstance, FString("SaveSystemData"), -2, SavedDelegate);
  }
}

void UMainGI::LoadSave()
  {
    if (UMySaveGame *LoadedSaveThing = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SelectedSaveSlot, SelectedSaveIndex)))
    {
      LoadedSave = LoadedSaveThing;
      LevelNumber = LoadedSave->LevelNumber;
      LevelUnlocked = LoadedSave->LevelUnlocked;
      UGameplayStatics::OpenLevel(this, FName("LoadingSaveLevel"));
      FTimerHandle UnusedHandle;
      GetWorld()->GetTimerManager().SetTimer(UnusedHandle, [this, LoadedSaveThing]()
      {
        UGameplayStatics::OpenLevel(this, FName("Level"+FString::FromInt(LevelNumber)));
      }, 0.01f, false);
      UKismetSystemLibrary::PrintString(this, "should be loading"+FString("Level"+FString::FromInt(LevelNumber)));
    }
    else
    {
      UKismetSystemLibrary::PrintString(this, "Failed to load save");
    }
  }

  void UMainGI::NotifySaveSystemDataCreated(const FString &SlotName, int32 SlotNumber, bool success)
  {
    if (success)
    {
      UKismetSystemLibrary::PrintString(this, "Created SaveSystemData with highestsaveindex "+FString::FromInt(HighestSaveIndex));
    }
    else
    {
      UKismetSystemLibrary::PrintString(this, "Failed to save SaveSystemData");
    }
  }
