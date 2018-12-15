// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UePortalGameMode.h"
#include "UePortalCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUePortalGameMode::AUePortalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
