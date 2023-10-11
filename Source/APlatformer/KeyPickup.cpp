// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyPickup.h"
#include "Components/PrimitiveComponent.h"
#include "GhostActor.h"

// Sets default values
AKeyPickup::AKeyPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AKeyPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKeyPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AKeyPickup::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
  TMap<FName, int> KeyTypeMap;
  KeyTypeMap.Add("Create", 0);
  KeyTypeMap.Add("Destroy", 1);
  KeyTypeMap.Add("Portal", 2);
  switch (KeyTypeMap[KeyType])
  {
  //create (does not spawn in actors) if it is a create key, BoundActors must be all GhostActors
  case 0:
    for (AActor* SelectedActor : BoundActors)
  	{
    //the selected actor should have a mesh the root component
    UPrimitiveComponent *ActorRoot = Cast<UPrimitiveComponent>(SelectedActor->GetRootComponent());
    ActorRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ActorRoot->SetMaterial(0, Cast<AGhostActor>(SelectedActor)->VisibleMaterial);
    }
    Destroy();
    break;
  //destroy
  case 1:
    for (AActor* SelectedActor : BoundActors)
  	{
      SelectedActor->Destroy();
    }
    Destroy();
    break;
  //portal
  case 2:
    for (AActor* SelectedActor : BoundActors)
  	{
      //don't crash the game by putting non-portals
      Cast<APortal>(SelectedActor)->ActivatePortal();
    }
    Destroy();
    break;
  
  default:
    break;
  }
}
