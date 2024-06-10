// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimSequence.h"
#include "APlatformerCharacter.h"

UShotgunComponent::UShotgunComponent()
{
  // Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UShotgunComponent::BeginPlayBruh()
{
  Name = FName("Shotgun");

  DamagePerBullet = 10;
  EquipTime = 0.5;
  HolsterTime = 0.4;
  ReloadTime = 1.5;
  MaxAmmo = 6;
  MinimumFireTime = 0.5;
  LoadedAmmo = MaxAmmo;
}

void UShotgunComponent::Fire()
{
  UKismetSystemLibrary::PrintString(this, "Shotgun firing");
  UGunComponent::Fire();
}

void UShotgunComponent::StartShooting()
{
  UGunComponent::StartShooting();
}

void UShotgunComponent::StopShooting()
{
}

void UShotgunComponent::Reload()
{
  if (Character->GetShotgunReserve() == 0)
  {
    UKismetSystemLibrary::PrintString(this, "no bullets left to reload");
    return;
  }
  UGunComponent::Reload();
}

void UShotgunComponent::FinishReload()
{
  SetCanShootTrue();
  if (int CharacterAmmo = Character->GetShotgunReserve() < MaxAmmo)
	{
		LoadedAmmo = CharacterAmmo;
    Character->DecreaseShotgunReserve(CharacterAmmo);
	}
  else
  {
    LoadedAmmo = MaxAmmo;
    Character->DecreaseShotgunReserve(MaxAmmo);
  }
}

void UShotgunComponent::ShootCalc()
{
}
