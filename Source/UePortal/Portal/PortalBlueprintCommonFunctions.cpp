// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalBlueprintCommonFunctions.h"
#include "PortalDoorComponent.h"

int UPortalBlueprintCommonFunctions::GetNowPortalNum()
{
	return UPortalDoorComponent::GetAllPortals().Num();
}

float UPortalBlueprintCommonFunctions::GlobalClipPlane()
{
	return GNearClippingPlane;
}

bool UPortalBlueprintCommonFunctions::IsTwoWeakPointEqual(UObject * a, UObject * b)
{
	TWeakPtr<UObject> r;
	return TWeakObjectPtr<UObject>(a) == b;
}
