// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PPortalTree.generated.h"

/**
 * 
 */
UCLASS()
class UEPORTAL_API UPPortalTree : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		class UPPortalNode* rootNode;
	UPROPERTY()
		TArray<UPPortalNode*> nodePoolHigh;
	UPROPERTY()
		TArray<UPPortalNode*> nodePoolMid;
	UPROPERTY()
		TArray<UPPortalNode*> nodePoolLow;
	
public:
	UPPortalNode* QureyPortalNode(int32 layer);

private:
	UPPortalNode* QureyPortalNodeInternal(TArray<UPPortalNode*>& pool, int32 layer);
};
