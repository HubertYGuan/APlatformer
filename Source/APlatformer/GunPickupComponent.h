// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "APlatformerCharacter.h"
#include "GunPickupComponent.generated.h"

/**
 * 
 */
UCLASS()
class APLATFORMER_API UGunPickupComponent : public USphereComponent, public IOverlapInterface
{
	GENERATED_BODY()
	
	protected:
	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;
};
