// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PortalBlueprintCommonFunctions.generated.h"

/**
 * 
 */
UCLASS()
class UEPORTAL_API UPortalBlueprintCommonFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	struct IgnorePair
	{
		TWeakObjectPtr<USceneComponent> obj1;
		TWeakObjectPtr<USceneComponent> obj2;
	};
private:
	
	static TArray<IgnorePair> ignoreComponents;
public:
	static const TArray<IgnorePair>& GetIgnoreComponents();

	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		static bool IgnoreBetween(USceneComponent* a, USceneComponent* b);
	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		static bool RemoveIgnoreBetween(USceneComponent* a, USceneComponent* b);
	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		static int GetNowPortalNum();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal Component")
		static float GlobalClipPlane();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal Component|Test")
		static bool IsTwoWeakPointEqual(UObject* a, UObject* b);
	UFUNCTION(BlueprintCallable, Category = "Portal Component|Change PhysicsScene")
		static void ChangePhysicsScene();
	
};
