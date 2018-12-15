// Fill out your copyright notice in the Description page of Project Settings.

#include "PPortalTree.h"
#include "PPortalNode.h"
#include "Engine.h"
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
		pool.Add(tempPtr);
		//temTxt->InitAutoFormat()
	}
	return pool.Pop();
}