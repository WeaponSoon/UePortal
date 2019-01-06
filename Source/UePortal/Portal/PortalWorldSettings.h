// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "PortalWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class UEPORTAL_API APortalWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	APortalWorldSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	//APortalWorldSettings();

public:
	struct IgnorePair
	{
		TWeakObjectPtr<USceneComponent> obj1;
		TWeakObjectPtr<USceneComponent> obj2;
	};
private:
	
	TArray<IgnorePair> ignoreComponents;
public:
	const TArray<IgnorePair>& GetIgnoreComponents();

	bool IgnoreBetween(USceneComponent* a, USceneComponent* b);
	
	bool RemoveIgnoreBetween(USceneComponent* a, USceneComponent* b);
};
