// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformerCharacter.h"
#include "Portal.generated.h"

UCLASS()
class APLATFORMER_API APortal : public AActor, public IOverlapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//if the portal is activated
	UPROPERTY(BlueprintReadWrite)
	bool bIsActivated = false;

	//can either be Level or Vector
	UPROPERTY(BlueprintReadWrite)
	FName PortalType;

	UPROPERTY(BlueprintReadWrite)
	FName LevelDestination;

	//if it's not a numbered level this is 0
	UPROPERTY(BlueprintReadWrite)
	int LevelNumber;

	/*whether or not the portal should force the character to auto save, if it does, the level is not opened automatically
	and is instead opened via UMainGI::LoadSave*/
	UPROPERTY(BlueprintReadWrite)
	bool bShouldAutoSave;

	UPROPERTY(BlueprintReadWrite)
	FVector TeleportLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator TeleportRotation;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OverlapAction(AAPlatformerCharacter *CharacterRef) override;

	//should play animations to activate/deactivate
	UFUNCTION(BlueprintNativeEvent)
	void ActivatePortal();

	UFUNCTION(BlueprintNativeEvent)
	void DeActivatePortal();

};
