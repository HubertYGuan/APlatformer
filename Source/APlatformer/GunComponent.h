// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GunComponent.generated.h"

class AAPlatformerCharacter;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APLATFORMER_API UGunComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	//copied from template (but I use line traces and not projectiles)
	public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float DamagePerBullet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	bool bHolstered;

	// Below times in seconds:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float EquipTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float HolsterTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float ReloadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float MinimumFireTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float ShootDelay;

	// Ammo currently stored in gun, ammo in reserve is stored in character (specific to each ammo type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	int LoadedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	int MaxAmmo;

	// Whether gun can be fired not accounting for ammo (e.g. mid-handling or in a scene where you're not allowed to shoot)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	bool bCanShoot = false;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* HolsterSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* ReloadSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimSequenceBase* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimSequenceBase* EquipAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimSequenceBase* HolsterAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimSequenceBase* ReloadAnimation;


	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** Sets default values for this component's properties */
	UGunComponent();

	// Called when the game starts, by pickup actor
	UFUNCTION()
	virtual void BeginPlayBruh();

	// Actually shoots the gun, consumes one ammo each time and checks ammo reserves to see if possible, else forces StopShooting
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Fire();

	// Called on click to initiate shooting
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void StartShooting();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void StopShooting();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Equip(AAPlatformerCharacter *CharacterRef);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Holster();

	// Timer function called in holster to move gun to holster
	UFUNCTION()
	virtual void FinishHolster();

	FTimerHandle HolsterHandle;

	// Reloads gun
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Reload();

	// Timer function called in reload to change gun ammo values (not defined in this class, only children)
	UFUNCTION()
	virtual void FinishReload();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void AttachGun(AAPlatformerCharacter *CharacterRef, bool bEquip = false);

	protected:
	// Unique shooting calculation for every weapon that damages hit damageable actors
	UFUNCTION()
	virtual void ShootCalc();

	UFUNCTION()
	void SetCanShootTrue(){bCanShoot = true;}

	// Handle used when the gun is being equipped (to prevent shooting)
	FTimerHandle EquipHandle;

	// Handle used when delaying the start of shooting (if gun has shoot delay)
	FTimerHandle StartShootHandle;

	// Handle used when the gun is firing full-auto or semi-auto
	FTimerHandle FireHandle;

	// Handle used when between gun bursts (if gun has burst fire)
	FTimerHandle BurstHandle;

	// Handle used when the gun is reloading
	FTimerHandle ReloadHandle;

	/** The Character holding this weapon*/
	AAPlatformerCharacter* Character;
};
