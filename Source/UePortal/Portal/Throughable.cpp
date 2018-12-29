// Fill out your copyright notice in the Description page of Project Settings.

#include "Throughable.h"

// Add default functionality here for any IThroughable functions that are not pure virtual.

void IThroughable::AddNearPortalDoor(UPortalDoorComponent * nearPortal)
{
	if (nearPortal != nullptr)
	{
		nearPortals.Add(nearPortal);
	}
}
void IThroughable::RemoveNearPortalDoor(UPortalDoorComponent * nearPortal)
{
	if (nearPortals.Contains(nearPortal))
	{
		nearPortals.Remove(nearPortal);
	}
}