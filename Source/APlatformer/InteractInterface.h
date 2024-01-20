// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

class AAPlatformerCharacter;

UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class APLATFORMER_API IInteractInterface
{
	GENERATED_BODY()
	

  public:
	//plays when character starts hovering cursor over thing
  virtual void StartHover(AAPlatformerCharacter *CharacterRef);
	//plays when stop hovering cursor
  virtual void EndHover(AAPlatformerCharacter *CharacterRef);
	//interact progress wheel is implemented via blueprint timeline
	//below should be blueprint implementable events tied to a bleuprint timeline
	//they should be called in AAPlatformerCharacter binded to interact key

	virtual void StartInteract(AAPlatformerCharacter *CharacterRef);

	virtual void CancelInteract(AAPlatformerCharacter *CharacterRef);

	virtual void FinishInteract(AAPlatformerCharacter *CharacterRef);
	protected:

	bool bOnCooldown;

	public:

	virtual bool GetOnCooldown(){return bOnCooldown;}
};