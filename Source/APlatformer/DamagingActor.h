// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "DamagingActor.generated.h"

UCLASS()
class APLATFORMER_API ADamagingActor : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamagingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int DamagePerTick;

	AAPlatformerCharacter *CurrentCharacterRef;

	//in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TickRate;

	UFUNCTION()
	void DamageTick();

	FTimerHandle TimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

	virtual void EndOverlapAction(AAPlatformerCharacter *CharacterRef) override;
};
