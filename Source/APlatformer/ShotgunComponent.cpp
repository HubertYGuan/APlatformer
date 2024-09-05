// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimSequence.h"
#include "APlatformerCharacter.h"
#include "DrawDebugHelpers.h"

UShotgunComponent::UShotgunComponent()
{
  // Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}
FFloatArray FloatArray0;
FFloatArray FloatArray1;
void UShotgunComponent::BeginPlayBruh()
{
  Name = FName("Shotgun");

  DamagePerBullet = 10;
  FalloffDamageMultipliers.Add(FloatArray0);
  FalloffDamageMultipliers.Add(FloatArray1);
  FalloffDamageMultipliers[0].Add(1000.f);
  FalloffDamageMultipliers[0].Add(2000.f);
  FalloffDamageMultipliers[1].Add(0.85f);
  FalloffDamageMultipliers[1].Add(0.5f);
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
  UKismetSystemLibrary::PrintString(this, "Shotgun reloaded");
}

void UShotgunComponent::ShootCalc()
{
  // Raycast diretly out of character's eyes for 12 pellits in a shell with some small amount of spread (should have reticle to indicate spread), limit to like 15m
  // Get data from raycast for distance each pellet travels and actor hit as well as skeleton bones for skeletal meshes so we can calculate damage for each pellet based on distance and body part
  if (!Character || !Character->GetFirstPersonCameraComponent())
  {
    return;
  }

  FVector CameraLocation = Character->GetFirstPersonCameraComponent()->GetComponentLocation();
  FRotator CameraRotation = Character->GetFirstPersonCameraComponent()->GetComponentRotation();

  for (int i = 0; i < 12; i++)
  {
    // Add some random spread
    FRotator SpreadRotation = CameraRotation;
    SpreadRotation.Pitch += FMath::RandRange(-2.0f, 2.0f);
    SpreadRotation.Yaw += FMath::RandRange(-2.0f, 2.0f);

    FVector EndLocation = CameraLocation + (SpreadRotation.Vector() * 3000.0f); // 15m = 1500cm

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Character);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, EndLocation, ECC_Visibility, Params);

    if (bHit)
    {
      // Process hit
      if (HitResult.GetActor() != nullptr)
      {
        // Apply damage or other effects

        // Apply damage falloff
        float Distance = (HitResult.Location - CameraLocation).Size();
        float Damage = DamagePerBullet;

        int j = 0;

        for (auto Threshold : FalloffDamageMultipliers[0].Floats)
        {
          if (Distance < Threshold)
          {
            if (j == 0)
            {
              break;
            }
            Damage *= FalloffDamageMultipliers[1][j-1];
            break;
          }
          j++;
        }
        UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), Damage, SpreadRotation.Vector(), HitResult, Character->GetController(), Character, nullptr);
      }
      // Optionally draw debug line
      DrawDebugLine(GetWorld(), CameraLocation, HitResult.Location, FColor::Red, false, 1.0f, 0, 1.0f);
    }
    else
    {
      // Optionally draw debug line
      DrawDebugLine(GetWorld(), CameraLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
    }

}
}
