// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunComponent.h"
#include "ShotgunComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APLATFORMER_API UShotgunComponent : public UGunComponent
{
	GENERATED_BODY()
	
	UShotgunComponent();

	virtual void BeginPlayBruh() override;

	// Actually shoots the gun
	virtual void Fire() override;

	// Called on click to initiate shooting
	virtual void StartShooting() override;

	virtual void StopShooting() override;

	virtual void Reload() override;

	virtual void FinishReload() override;

	protected:
	// Unique shooting calculation for every weapon that damages hit damageable actors
	virtual void ShootCalc() override;
};
