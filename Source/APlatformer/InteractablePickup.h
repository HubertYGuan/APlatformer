// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "InteractablePickup.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPickup : uint8
{
	Shotgun UMETA(DisplayName="Shotgun")
};
UCLASS()
class APLATFORMER_API AInteractablePickup : public AInteractable
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintNativeEvent)
	void StartInteract(AAPlatformerCharacter *CharacterRef) override;

	UFUNCTION(BlueprintNativeEvent)
	void CancelInteract(AAPlatformerCharacter *CharacterRef) override;

	UFUNCTION(BlueprintCallable)
	void FinishInteract(AAPlatformerCharacter *CharacterRef) override;

	UFUNCTION(BlueprintNativeEvent)
	void StartHover(AAPlatformerCharacter *CharacterRef) override;

	UFUNCTION(BlueprintNativeEvent)
	void EndHover(AAPlatformerCharacter *CharacterRef) override;

	
	UPROPERTY(BLueprintReadOnly)
	EPickup PickupType;

	//add to hovering character's inventory
	UFUNCTION(BlueprintCallable)
	void AddToInventory();
};
