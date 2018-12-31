// Fill out your copyright notice in the Description page of Project Settings.

#include "Throughable.h"

// Add default functionality here for any IThroughable functions that are not pure virtual.
TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>> UThroughableComponent::throughableMap;

void UThroughableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdatePassingPortal();
}

void UThroughableComponent::AddNearPortalDoor(UPortalDoorComponent * nearPortal)
{
	if (nearPortal != nullptr)
	{
		nearPortals.Add(nearPortal);
	}
}
void UThroughableComponent::RemoveNearPortalDoor(UPortalDoorComponent * nearPortal)
{
	if (nearPortals.Contains(nearPortal))
	{
		nearPortals.Remove(nearPortal);
	}
}

void UThroughableComponent::UpdatePassingPortal()
{
	if (!throughableComponent.IsValid())
	{
		passingPortal.Reset();
		for (auto& portal : nearPortals)
		{
			if (portal.IsValid())
			{
				auto& thrMap = portal->GetOverlapdThrougbleComp();
				if (thrMap.Contains(throughableComponent))
				{
					thrMap.Remove(throughableComponent);
				}
			}
		}
		nearPortals.Empty();
		return;
	}
		
	passingPortal.Reset();
	for (auto& portal : nearPortals)
	{
		if (portal.IsValid())
		{
			if (!passingPortal.IsValid() || (throughableComponent->GetComponentLocation() - portal->doorShowSelf->GetComponentLocation()).SizeSquared() <
				(throughableComponent->GetComponentLocation() - passingPortal->doorShowSelf->GetComponentLocation()).SizeSquared())
			{
				passingPortal = portal;
			}
		}
		else
		{
			nearPortals.Remove(portal);
		}
	}
}

void UThroughableComponent::SetThroughableComponent(USceneComponent * throughable)
{
	if (throughable == nullptr)
	{
		
		if (throughableMap.Contains(throughableComponent))
		{
			throughableMap.Remove(throughableComponent);
		}
		auto old = throughableComponent.IsValid() ? throughableComponent.Get() : nullptr;
		passingPortal.Reset();
		for (auto& portal : nearPortals)
		{
			if (portal.IsValid())
			{
				auto& thrMap = portal->GetOverlapdThrougbleComp();
				if (thrMap.Contains(throughableComponent))
				{
					thrMap.Remove(throughableComponent);
				}
			}
		}
		nearPortals.Empty();
		throughableComponent.Reset();
		OnSetThroughableComponent(old, nullptr);
		return;
	}
	else
	{
		if (throughableMap.Contains(throughable))
		{
			if (throughableMap[throughable].IsValid())
			{
				throughableMap[throughable]->SetThroughableComponent(nullptr);
			}
			else
			{
				throughableMap.Remove(throughable);
			}
		}
		auto lastRef = throughableComponent.IsValid()? throughableComponent.Get():nullptr;
		if (throughableMap.Contains(throughableComponent))
		{
			throughableMap.Remove(throughableComponent);
		}
		passingPortal.Reset();
		for (auto& portal : nearPortals)
		{
			if (portal.IsValid())
			{
				auto& thrMap = portal->GetOverlapdThrougbleComp();
				if (thrMap.Contains(throughableComponent))
				{
					thrMap.Remove(throughableComponent);
				}
			}
		}
		nearPortals.Empty();
		throughableMap.Add(throughable, this);
		throughableComponent = throughable;
		OnSetThroughableComponent(lastRef, throughableComponent.IsValid() ? throughableComponent.Get() : nullptr);
	}

	/*USceneComponent* oldOne = throughableComponent.IsValid() ? throughableComponent.Get() : nullptr;
	throughableComponent = throughable;
	USceneComponent* newOne = throughableComponent.IsValid() ? throughableComponent.Get() : nullptr;
	OnSetThroughableComponent(oldOne, newOne);*/
}

void UThroughableComponent::OnSetThroughableComponent(USceneComponent * oldOne, USceneComponent * newOne)
{
}

void UThroughableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (throughableMap.Contains(throughableComponent))
	{
		throughableMap.Remove(throughableComponent);
	}
	for (auto& portal : nearPortals)
	{
		if (portal.IsValid())
		{
			if(portal->GetOverlapdThrougbleComp().Contains(throughableComponent))
				portal->GetOverlapdThrougbleComp().Remove(throughableComponent);
		}
	}
	nearPortals.Empty();
}

void UThroughableComponent::BeginPlay()
{
}

const TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>>& UThroughableComponent::GetThroughableMap()
{
	return throughableMap;
	// TODO: 在此处插入 return 语句
}
