// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OverlapInterface.generated.h"

class AAPlatformerCharacter;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UOverlapInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class APLATFORMER_API IOverlapInterface
{
	GENERATED_BODY()
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
  public:
  virtual void OverlapAction(AAPlatformerCharacter *CharacterRef);

  virtual void EndOverlapAction(AAPlatformerCharacter *CharacterRef);
};
