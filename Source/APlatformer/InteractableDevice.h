// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "InteractableDevice.generated.h"

/**
 For doors, boxes, all that kind of stuff, not pickups

 */
UCLASS()
class APLATFORMER_API AInteractableDevice : public AInteractable
{
	GENERATED_BODY()

	protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AutoCooldownResetOverride;

	public:
	UFUNCTION(BlueprintNativeEvent)
	void StartInteract(AAPlatformerCharacter *CharacterRef) override;
	UFUNCTION(BlueprintNativeEvent)
	void CancelInteract(AAPlatformerCharacter *CharacterRef) override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void FinishInteract(AAPlatformerCharacter *CharacterRef) override;
	UFUNCTION(BlueprintNativeEvent)
	void StartHover(AAPlatformerCharacter *CharacterRef) override;
	UFUNCTION(BlueprintNativeEvent)
	void EndHover(AAPlatformerCharacter *CharacterRef) override;
};
