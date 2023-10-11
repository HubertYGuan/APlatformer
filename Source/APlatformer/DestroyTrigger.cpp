// Fill out your copyright notice in the Description page of Project Settings.


#include "DestroyTrigger.h"
#include "Components/BoxComponent.h"

// Sets default values
ADestroyTrigger::ADestroyTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a new BoxCollision component for this actor with default size (ai generated code lmoa)
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	BoxCollision->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void ADestroyTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADestroyTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADestroyTrigger::OverlapAction(AAPlatformerCharacter *CharacterRef)
{
	for (AActor* SelectedActor : BoundActors)
	{
		SelectedActor->Destroy();
  }
	BoundActors.Empty();
}
