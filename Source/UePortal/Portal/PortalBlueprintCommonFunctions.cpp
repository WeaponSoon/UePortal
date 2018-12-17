// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalBlueprintCommonFunctions.h"
#include "PortalDoorComponent.h"

int UPortalBlueprintCommonFunctions::GetNowPortalNum()
{
	return UPortalDoorComponent::GetAllPortals().Num();
}