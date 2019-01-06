// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalBlueprintCommonFunctions.h"
#include "PortalDoorComponent.h"
#include "Engine/World.h"
#include "Engine.h"
#include "Physics/PhysicsFiltering.h"
#include "PortalWorldSettings.h"
#include "PPortalComponent.h"

TArray<UPortalBlueprintCommonFunctions::IgnorePair> UPortalBlueprintCommonFunctions::ignoreComponents;
int UPortalBlueprintCommonFunctions::GetNowPortalNum()
{
	return UPortalDoorComponent::GetAllPortals().Num();
}

float UPortalBlueprintCommonFunctions::GlobalClipPlane()
{
	return GNearClippingPlane;
}

const TArray<UPortalBlueprintCommonFunctions::IgnorePair>& UPortalBlueprintCommonFunctions::GetIgnoreComponents()
{
	return ignoreComponents;
}

bool UPortalBlueprintCommonFunctions::IgnoreBetween(USceneComponent* a, USceneComponent* b)
{
	APortalWorldSettings* settings = nullptr;
	
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, UPPortalComponent::currentWorld==nullptr ? "null" : "not null");
	if (UPPortalComponent::currentWorld && UPPortalComponent::currentWorld->GetWorldSettings())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, UPPortalComponent::currentWorld->GetWorldSettings()->GetClass()->GetName());
		settings = Cast<APortalWorldSettings>(UPPortalComponent::currentWorld->GetWorldSettings());
	}
	if (settings == nullptr)
		return false;

	return settings->IgnoreBetween(a, b);
}

bool UPortalBlueprintCommonFunctions::IsTwoWeakPointEqual(UObject * a, UObject * b)
{
	TWeakPtr<UObject> r;
	return TWeakObjectPtr<UObject>(a) == b;
}


void UPortalBlueprintCommonFunctions::ChangePhysicsScene()
{
	/*GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Will New Phyics Scene");
	if (GSimulationFilterShader == PhysXSimFilterShaderPortal)
		return;
	auto tem = GSimulationFilterShader;
	GSimulationFilterShader = PhysXSimFilterShaderPortal;
	auto world = GEngine->GetWorld();
	if (world != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Newing Phyics Scene");
		world->CreatePhysicsScene(world->GetWorldSettings());
		
	}*/
}



bool UPortalBlueprintCommonFunctions::RemoveIgnoreBetween(USceneComponent * a, USceneComponent * b)
{
	APortalWorldSettings* settings = nullptr;
	if (UPPortalComponent::currentWorld && UPPortalComponent::currentWorld->GetWorldSettings())
	{
		settings = Cast<APortalWorldSettings>(UPPortalComponent::currentWorld->GetWorldSettings());
	}
	if (settings == nullptr)
		return false;
	return settings->RemoveIgnoreBetween(a, b);
}
