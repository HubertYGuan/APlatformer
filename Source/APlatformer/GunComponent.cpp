// Fill out your copyright notice in the Description page of Project Settings.


#include "GunComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/ShapeComponent.h"
#include "APlatformerCharacter.h"

UGunComponent::UGunComponent()
{
  // Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UGunComponent::BeginPlayBruh()
{
	LoadedAmmo = MaxAmmo;
}

void UGunComponent::Fire()
{
	UKismetSystemLibrary::PrintString(this, "attempting firing");
	UKismetSystemLibrary::PrintString(this, UKismetStringLibrary::Conv_IntToString(LoadedAmmo));
	UKismetSystemLibrary::PrintString(this, UKismetStringLibrary::Conv_IntToString(MaxAmmo));
	if (LoadedAmmo > 0)
	{
		LoadedAmmo--;
	}
	else
	{
		StopShooting();
		bCanShoot = false;
		return;
	}
	
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		PlayAnimation(FireAnimation, false);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, "no animation specified");
	}
	//do the gun's specific line trace(s) that deals damage and stuff
	ShootCalc();
	// delay when you can shoot again
	bCanShoot = false;
  GetWorld()->GetTimerManager().SetTimer(FireHandle, this, &UGunComponent::SetCanShootTrue, MinimumFireTime, false);
}

// Simple full auto implementation that may be overridden
void UGunComponent::StartShooting()
{
	UKismetSystemLibrary::PrintString(this, "trying to start shooting (see if bool false)");
	if (!bCanShoot)
	{
		UKismetSystemLibrary::PrintString(this, "bCanShoot false");
		return;
	}
	Fire();
}

void UGunComponent::StopShooting()
{
}

void UGunComponent::Equip(AAPlatformerCharacter *CharacterRef = nullptr)
{
	if (CharacterRef == nullptr)
	{
		if (Character != nullptr)
		{
			CharacterRef = Character;
		}
		else
		{
			return;
		}
	}
	else
	{
		Character = CharacterRef;
	}
	bHolstered = false;
	Character->bWeaponOut = true;
	Character->GunOut = this;
	//currently no equip animations
  AttachGun(CharacterRef);
  GetWorld()->GetTimerManager().SetTimer(EquipHandle, this, &UGunComponent::SetCanShootTrue, EquipTime, false);
  if (EquipAnimation != nullptr)
	{
		PlayAnimation(EquipAnimation, false);
	}
	if (EquipSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, Character->GetActorLocation());
	}
}

void UGunComponent::Holster()
{
	// Intended so that you can't holster while you're doing some other handling stuff
	if ((!bCanShoot && LoadedAmmo != 0) || bHolstered)
	{
		return;
	}
	// Play anim
	if (HolsterAnimation != nullptr)
	{
		PlayAnimation(HolsterAnimation, false);
	}
	if (HolsterSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HolsterSound, Character->GetActorLocation());
	}
	GetWorld()->GetTimerManager().SetTimer(HolsterHandle, this, &UGunComponent::FinishHolster, HolsterTime, false);
}

void UGunComponent::FinishHolster()
{
	AttachToComponent(Character->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("HolsterSocket")));
	bHolstered = true;
	Character->bWeaponOut = false;
	Character->GunOut = nullptr;
	if (Character->bWantsToEquip1)
	{
		Character->bWantsToEquip1 = false;
		Character->EquipInput1();
		return;
	}
	if (Character->bWantsToEquip2)
	{
		Character->bWantsToEquip2 = false;
		Character->EquipInput2();
		return;
	}
}

void UGunComponent::Reload()
{
	if (LoadedAmmo == MaxAmmo)
	{
		return;
	}
	if (Character->GetShotgunReserve() == 0)
	{
		return;
	}
	if (GetWorld()->GetTimerManager().GetTimerRemaining(ReloadHandle) > 0.0f)
	{
		return;
	}
	if (ReloadAnimation != nullptr)
	{
		PlayAnimation(ReloadAnimation, false);
	}
	if (ReloadSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, Character->GetActorLocation());
	}
	UKismetSystemLibrary::PrintString(this, "reloading");
	bCanShoot = false;
	GetWorld()->GetTimerManager().SetTimer(ReloadHandle, this, &UGunComponent::FinishReload, ReloadTime, false);
}

void UGunComponent::FinishReload()
{
	
}

void UGunComponent::AttachGun(AAPlatformerCharacter *CharacterRef, bool bEquip)
{
	Character = CharacterRef;
	if (Character == nullptr)
	{
		return;
	}

	// Destroy the pickup hitbox

	TArray<USceneComponent*> Children;
	GetChildrenComponents(true, Children);
	for (USceneComponent* Child : Children)
	{
		if (Cast<UShapeComponent>(Child))
		{
			Child->DestroyComponent();
		}
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	if (bEquip)
	{
		AttachToComponent(Character->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("HolsterSocket")));
		return;
	}
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
}

void UGunComponent::ShootCalc()
{
}
