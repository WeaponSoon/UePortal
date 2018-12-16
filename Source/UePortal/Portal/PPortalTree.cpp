// Fill out your copyright notice in the Description page of Project Settings.

#include "PPortalTree.h"
#include "PPortalNode.h"
#include "Engine.h"
#include "PortalDoorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

UPPortalNode*  UPPortalTree::QureyPortalNode(int32 layer)
{
	UPPortalNode* retRes = nullptr;
	int32 clippedLayer = FMath::Clamp<int32>(layer, 0, 2);
	switch (clippedLayer)
	{
	case 0:
		retRes = QureyPortalNodeInternal(nodePoolHigh, clippedLayer);
		break;
	case 1:
		retRes = QureyPortalNodeInternal(nodePoolMid, clippedLayer);
		break;
	case 2:
		retRes = QureyPortalNodeInternal(nodePoolLow, clippedLayer);
		break;
	default:
		break;
	}
	return retRes;
}

UPPortalNode* UPPortalTree::QureyPortalNodeInternal(TArray<UPPortalNode*>& pool, int32 layer)
{
	if (pool.Num() <= 0)
	{
		UPPortalNode* tempPtr = NewObject<UPPortalNode>();
		UTextureRenderTarget2D* temTxt = NewObject<UTextureRenderTarget2D>();
		temTxt->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		FIntPoint reslution = GetWorld()->GetGameViewport()->Viewport->GetSizeXY();
		float ratio = FMath::Sqrt(layer + 1);
		temTxt->InitAutoFormat(reslution.X / ratio, reslution.Y / ratio);
		temTxt->UpdateResourceImmediate(true);
		tempPtr->SetRenderTexture(temTxt);
		tempPtr->portalDoor = nullptr;
		pool.Add(tempPtr);
		//temTxt->InitAutoFormat()
	}
	return pool.Pop();
}

void UPPortalTree::InitPortalTree(const USceneCaptureComponent2D* root)
{
	rootCamera = const_cast<USceneCaptureComponent2D*>(root);
	rootNode = QureyPortalNode(0);
	rootCamera->bCaptureEveryFrame = false;
}

void UPPortalTree::BuildPortalTree()
{
	UPPortalNode* currentSearch = rootNode;
	
}

void UPPortalTree::BuildPortalTreeInternal(UPPortalNode * node)
{
	USceneCaptureComponent2D* curCam = node->portalDoor == nullptr ? rootCamera : node->portalDoor->doorCamera;
	if (node->portalDoor != nullptr)
	{
		curCam->SetWorldLocation(node->cameraTran.GetLocation());
		curCam->SetWorldRotation(node->cameraTran.GetRotation());
		curCam->bEnableClipPlane = true;
		curCam->ClipPlaneNormal = node->clipPlaneNormal;
		curCam->ClipPlaneBase = node->clipPlanePos;
	}
	FBox lastBox = node->portalDoor == nullptr ?
		FBox(FVector(-1, -1, 0), FVector(1, 1, 0)) : UPortalDoorComponent::GetSceneComponentScreenBox(node->portalDoor->GetOtherDoor()->doorShowSelf, curCam);
	for (int i = 0; i < UPortalDoorComponent::GetAllPortals().Num(); ++i)
	{
		auto nextPortalDoor = UPortalDoorComponent::GetAllPortals()[i];
		if (nextPortalDoor->ShouldRender(curCam, lastBox))
		{
			auto nextNode = QureyPortalNode(0);
			nextNode->portalDoor = nextPortalDoor;
			node->AddChild(nextNode);

		}
		
	}

}

void UPPortalTree::RenderPortalTree()
{

}