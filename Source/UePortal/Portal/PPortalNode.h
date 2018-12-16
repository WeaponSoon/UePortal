// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PPortalNode.generated.h"

/**
 * 
 */
UCLASS()
class UEPORTAL_API UPPortalNode : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
		class UTextureRenderTarget2D* renderTexture;
	UPROPERTY()
		TArray<UPPortalNode*> childrenNode;
	UPROPERTY()
		FVector clipPlanePos;
	UPROPERTY()
		FVector clipPlaneNormal;
	UPROPERTY()
		FTransform cameraTran;
	class UPPortalTree* motherTree;

	UPROPERTY()
		class UPortalDoorComponent* portalDoor;
public:
	void SetRenderTexture(const UTextureRenderTarget2D* renderTarget2D);
	UTextureRenderTarget2D* GetRenderTexture();
	void AddChild(const UPPortalNode* child);
	void RecycleChildren();

	void SetMotherTree(const UPPortalTree* mother);
};
