// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/SceneCaptureComponent2D.h"
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
	UPROPERTY()
		USceneCaptureComponent2D* rootCamera;
public:

	UPROPERTY()
		int32 maxLayer;

	UPPortalNode* QureyPortalNode(int32 layer);
	void RecyclePortalNode(UPPortalNode* node);
	void InitPortalTree(const USceneCaptureComponent2D* root);
	void BuildPortalTree();
	void RenderPortalTree();
private:
	void BuildPortalTreeInternal(UPPortalNode* node, int layer);
	UPPortalNode* QureyPortalNodeInternal(TArray<UPPortalNode*>& pool, int32 layer);
};
