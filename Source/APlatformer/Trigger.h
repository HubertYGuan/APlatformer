// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "Trigger.generated.h"

class UBoxComponent;

UCLASS()
class APLATFORMER_API ATrigger : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
protected:
	//the base box collision for the trigger
	UPROPERTY(VisibleDefaultsOnly)
	UBoxComponent* BoxCollision;

public:	
	// Sets default values for this actor's properties
	ATrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//only blueprint classes should inherit since this function is no longer virtual
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OverlapAction(AAPlatformerCharacter *CharacterRef) override;
};
