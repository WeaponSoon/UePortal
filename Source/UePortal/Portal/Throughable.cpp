// Fill out your copyright notice in the Description page of Project Settings.

#include "Throughable.h"
#include "Engine.h"
// Add default functionality here for any IThroughable functions that are not pure virtual.
TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>> UThroughableComponent::throughableMap;

UThroughableComponent::UThroughableComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PostPhysicsTickFunction.bCanEverTick = true;
	PostPhysicsTickFunction.TickGroup = TG_PostUpdateWork;
	PrePhysicsTickFunction.bCanEverTick = true;
	PrePhysicsTickFunction.TickGroup = TG_PrePhysics;
}

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

void UThroughableComponent::PostPhysics(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef & MyCompletionGraphEvent)
{
	
}

void UThroughableComponent::PrePhysics(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef & MyCompletionGraphEvent)
{
	
}

const TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>>& UThroughableComponent::GetThroughableMap()
{
	return throughableMap;
	// TODO: 在此处插入 return 语句
}

void UThroughableComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	if (bRegister)
	{
		if (SetupActorComponentTickFunction(&PostPhysicsTickFunction))
		{
			PostPhysicsTickFunction.Target = this;
			PostPhysicsTickFunction.AddPrerequisite(this, this->PrimaryComponentTick);
		}
	}
	else
	{
		if (PostPhysicsTickFunction.IsTickFunctionRegistered())
		{
			PostPhysicsTickFunction.UnRegisterTickFunction();
		}
	}
	if (bRegister)
	{
		if (SetupActorComponentTickFunction(&PrePhysicsTickFunction))
		{
			PrePhysicsTickFunction.Target = this;
			//PostPhysicsTickFunction.AddPrerequisite(this, this->PrimaryComponentTick);
		}
	}
	else
	{
		if (PrePhysicsTickFunction.IsTickFunctionRegistered())
		{
			PrePhysicsTickFunction.UnRegisterTickFunction();
		}
	}
}

void FThroughablePrePhysicsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef & MyCompletionGraphEvent)
{
	FActorComponentTickFunction::ExecuteTickHelper(Target, /*bTickInEditor=*/ false, DeltaTime, TickType, [this, &DeltaTime, &TickType, &CurrentThread, &MyCompletionGraphEvent](float DilatedTime)
	{
		Target->PrePhysics(DeltaTime, TickType, CurrentThread, MyCompletionGraphEvent);
	});
}

FString FThroughablePrePhysicsTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[UThroughableComponent::PrePhysicsTick]");
}

void FThroughablePostPhysicsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef & MyCompletionGraphEvent)
{
	FActorComponentTickFunction::ExecuteTickHelper(Target, /*bTickInEditor=*/ false, DeltaTime, TickType, [this, &DeltaTime, &TickType, &CurrentThread, &MyCompletionGraphEvent](float DilatedTime)
	{
		Target->PostPhysics(DeltaTime, TickType, CurrentThread, MyCompletionGraphEvent);
	});
}

FString FThroughablePostPhysicsTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[UThroughableComponent::PostPhysicsTick]");
}
