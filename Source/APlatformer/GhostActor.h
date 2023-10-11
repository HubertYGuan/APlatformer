// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "GhostActor.generated.h"

UCLASS()
class APLATFORMER_API AGhostActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGhostActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//supposed to be set in BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface *VisibleMaterial;
};
