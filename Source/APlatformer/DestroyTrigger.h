// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "DestroyTrigger.generated.h"

class UBoxComponent;

UCLASS()
class APLATFORMER_API ADestroyTrigger : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
	//the base box collision for the trigger
	UPROPERTY(VisibleDefaultsOnly)
	UBoxComponent* BoxCollision;

public:	
	// Sets default values for this actor's properties
	ADestroyTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> BoundActors;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;
};
