// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "Hoverable.generated.h"

UCLASS()
class APLATFORMER_API AHoverable : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHoverable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	AAPlatformerCharacter *HoveringCharacter;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void StartHover(AAPlatformerCharacter *CharacterRef) override;
	UFUNCTION(BlueprintNativeEvent)
	void EndHover(AAPlatformerCharacter *CharacterRef) override;
};
