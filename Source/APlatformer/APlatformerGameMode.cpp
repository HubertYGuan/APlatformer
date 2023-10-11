// Copyright Epic Games, Inc. All Rights Reserved.

#include "APlatformerGameMode.h"
#include "APlatformerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAPlatformerGameMode::AAPlatformerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_MainCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

  //set default player controller class (no null check lol)
  static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/FirstPerson/Blueprints/BP_PCThing"));
  if (PlayerControllerBPClass.Class != NULL)
  {
	PlayerControllerClass = PlayerControllerBPClass.Class;
  }
}
