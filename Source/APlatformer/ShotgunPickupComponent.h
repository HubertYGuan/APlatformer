// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunPickupComponent.h"
#include "ShotgunPickupComponent.generated.h"

/**
 * 
 */
UCLASS()
class APLATFORMER_API UShotgunPickupComponent : public UGunPickupComponent
{
	GENERATED_BODY()
	
	protected:
	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;
};
