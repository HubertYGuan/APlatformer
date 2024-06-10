// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunAmmoPickup.h"

// Sets default values
AShotgunAmmoPickup::AShotgunAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShotgunAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShotgunAmmoPickup::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	CharacterRef->AddShotgunAmmo(Ammo);
	Destroy();
}

// Called every frame
void AShotgunAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

