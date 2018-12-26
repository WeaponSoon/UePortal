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
	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		static int GetNowPortalNum();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal Component")
		static float GlobalClipPlane();
};
