// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PortalDoorComponent.h"
#include "Throughable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UThroughable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UEPORTAL_API IThroughable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	TSet<TWeakObjectPtr<UPortalDoorComponent>> nearPortals;
	TWeakObjectPtr<UPortalDoorComponent> passingPortal;
	TWeakObjectPtr<USceneComponent> throughableComponent;
	UFUNCTION()
		virtual void AddNearPortalDoor(UPortalDoorComponent* nearPortal);
	UFUNCTION()
		virtual void RemoveNearPortalDoor(UPortalDoorComponent* nearPortal);
	UFUNCTION()
		virtual void UpdatePassingPortal();
};
