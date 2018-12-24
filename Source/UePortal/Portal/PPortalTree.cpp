// Fill out your copyright notice in the Description page of Project Settings.

#include "PPortalTree.h"
#include "PPortalNode.h"
#include "Engine.h"
#include "PortalDoorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

const FName UPPortalTree::BACK_CAMERA_NAME("BackCamera");

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

void UPPortalTree::RecyclePortalNode(UPPortalNode * node)
{
	if (node->belongLayer <= 0)
	{
		nodePoolHigh.Push(node);
	}
	else if (node->belongLayer <= 1)
	{
		nodePoolMid.Push(node);
	}
	else
	{
		nodePoolLow.Push(node);
	}
}

UPPortalNode* UPPortalTree::QureyPortalNodeInternal(TArray<UPPortalNode*>& pool, int32 layer)
{
	if (pool.Num() <= 0)
	{
		UPPortalNode* tempPtr = NewObject<UPPortalNode>();
		UTextureRenderTarget2D* temTxt = NewObject<UTextureRenderTarget2D>();
		temTxt->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		FIntPoint reslution = GEngine->GameViewport->Viewport->GetSizeXY();
		float ratio = FMath::Sqrt(layer + 1);
		temTxt->InitAutoFormat(reslution.X / ratio, reslution.Y / ratio);
		temTxt->UpdateResourceImmediate(true);
		tempPtr->SetRenderTexture(temTxt);
		tempPtr->portalDoor = nullptr;
		tempPtr->motherTree = this;
		tempPtr->belongLayer = layer;
		pool.Add(tempPtr);
		//temTxt->InitAutoFormat()
	}
	return pool.Pop();
}

void UPPortalTree::InitPortalTree(const USceneCaptureComponent2D* root, AActor* motherActor)
{
	rootCamera = const_cast<USceneCaptureComponent2D*>(root);
	rootNode = QureyPortalNode(0);
	rootCamera->bCaptureEveryFrame = false;

	auto captures = motherActor->GetComponentsByClass(USceneCaptureComponent2D::StaticClass());
	USceneCaptureComponent2D* anotherSc = nullptr;
	for (auto& capture : captures)
	{
		if (capture->GetFName().IsEqual(BACK_CAMERA_NAME, ENameCase::CaseSensitive))
		{
			anotherSc = Cast<USceneCaptureComponent2D>(capture);
			break;
		}
	}
	
	if (anotherSc == nullptr)
	{
		anotherSc = NewObject<USceneCaptureComponent2D>(motherActor, BACK_CAMERA_NAME);
		anotherSc->RegisterComponent();
		//motherActor->AddInstanceComponent(anotherSc);
	}
	anotherSc->bCaptureEveryFrame = false;
	anotherSc->AttachToComponent(rootCamera, FAttachmentTransformRules(EAttachmentRule::KeepRelative,false));
	anotherSc->SetRelativeLocation(FVector::ForwardVector * 2 *  GNearClippingPlane);
	anotherSc->SetRelativeRotation(FQuat(FVector::UpVector, PI));
	
}

void UPPortalTree::BuildPortalTree()
{
	rootNode->RecycleChildren();
	BuildPortalTreeInternal(rootNode, 0);
	
}

void UPPortalTree::BuildPortalTreeInternal(UPPortalNode * node, int layer)
{
	if (layer >= maxLayer)
		return;
	layer++;
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
		bool isOther = false;
		if (node->portalDoor != nullptr)
			isOther = (node->portalDoor->GetOtherDoor() == nextPortalDoor);
		if (!isOther && nextPortalDoor->ShouldRender(curCam, lastBox))
		{
			auto nextNode = QureyPortalNode(layer-1);
			nextNode->portalDoor = nextPortalDoor;
			node->AddChild(nextNode);

		}
	}
	for (int i = 0; i < node->childrenNode.Num(); ++i)
	{
		if (node->portalDoor != nullptr)
		{
			curCam->SetWorldLocation(node->cameraTran.GetLocation());
			curCam->SetWorldRotation(node->cameraTran.GetRotation());
			curCam->bEnableClipPlane = true;
			curCam->ClipPlaneNormal = node->clipPlaneNormal;
			curCam->ClipPlaneBase = node->clipPlanePos;
		}
		UPortalDoorComponent* childSource = node->childrenNode[i]->portalDoor;
		UPortalDoorComponent* childDest = node->childrenNode[i]->portalDoor->GetOtherDoor();

		FVector sourceLocation = childSource->doorShowSelf->GetComponentLocation();
		FRotator sourceRotation = childSource->doorShowSelf->GetComponentRotation();

		FVector destLocation = childDest->doorShowSelf->GetComponentLocation();
		FRotator destRotation = childDest->doorShowSelf->GetComponentRotation();

		FVector relativeToSourceLocInWorld = curCam->GetComponentLocation() - sourceLocation;
		FVector relativeToSourceLocInLocal = sourceRotation.UnrotateVector(relativeToSourceLocInWorld);

		FVector relativeToDestLocInLocal = FRotator(FQuat(FVector::UpVector, PI)).RotateVector(relativeToSourceLocInLocal);
		
		FVector childCameraLoc = destRotation.RotateVector(relativeToDestLocInLocal) + destLocation;
	
		FMatrix relativeToSourceRot = FRotationMatrix(curCam->GetComponentRotation()) * FRotationMatrix(sourceRotation.GetInverse());
		
		FMatrix relativeToDestRot = relativeToSourceRot * FRotationMatrix(FRotator(FQuat(FVector::UpVector, PI)));
		
		FRotator childCameraRot = (relativeToDestRot * FRotationMatrix(destRotation)).Rotator();
		
		
		childSource->doorCamera->SetWorldLocation(childCameraLoc);
		childSource->doorCamera->SetWorldRotation(childCameraRot);
		childSource->doorCamera->bEnableClipPlane = true;
		childSource->doorCamera->ClipPlaneBase = childSource->GetOtherDoor()->doorShowSelf->GetComponentLocation();
		childSource->doorCamera->ClipPlaneNormal = childSource->GetOtherDoor()->doorShowSelf->GetForwardVector();

		node->childrenNode[i]->cameraTran = childSource->doorCamera->GetComponentTransform();
		node->childrenNode[i]->clipPlaneNormal = childSource->doorCamera->ClipPlaneNormal;
		node->childrenNode[i]->clipPlanePos = childSource->doorCamera->ClipPlaneBase;
		BuildPortalTreeInternal(node->childrenNode[i], layer);
		
	}

}


void UPPortalTree::RenderPortalTree()
{
	RenderPortalTreeInternal(rootNode);
}
void UPPortalTree::RenderPortalTreeInternal(UPPortalNode * node)
{
	for (int i = 0; i < node->childrenNode.Num(); ++i)
	{
		RenderPortalTreeInternal(node->childrenNode[i]);
	}
	if (node->portalDoor == nullptr)
	{
		for (int i = 0; i < node->childrenNode.Num(); ++i)
		{
			if (node->childrenNode[i]->portalDoor->doorShowSelf->GetClass()->IsChildOf<UMeshComponent>())
			{
				auto mesh = Cast<UMeshComponent>(node->childrenNode[i]->portalDoor->doorShowSelf);
				auto matInstDyn = Cast<UMaterialInstanceDynamic>(mesh->GetMaterial(0));
				matInstDyn->SetTextureParameterValue(FName("_MainTex"), node->childrenNode[i]->renderTexture);
			}
		}
		
	}
	else
	{
		node->portalDoor->doorCamera->SetWorldLocation(node->cameraTran.GetLocation());
		node->portalDoor->doorCamera->SetWorldRotation(node->cameraTran.GetRotation());
		node->portalDoor->doorCamera->bEnableClipPlane = true;
		node->portalDoor->doorCamera->ClipPlaneBase = node->clipPlanePos;
		node->portalDoor->doorCamera->ClipPlaneNormal = node->clipPlaneNormal;
		node->portalDoor->doorCamera->TextureTarget = node->renderTexture;
		for (int i = 0; i < node->childrenNode.Num(); ++i)
		{
			if (node->childrenNode[i]->portalDoor->doorShowSelf->GetClass()->IsChildOf<UMeshComponent>())
			{
				auto mesh = Cast<UMeshComponent>(node->childrenNode[i]->portalDoor->doorShowSelf);
				auto matInstDyn = Cast<UMaterialInstanceDynamic>(mesh->GetMaterial(0));
				matInstDyn->SetTextureParameterValue(FName("_MainTex"), node->childrenNode[i]->renderTexture);
			}
		}
		node->portalDoor->doorCamera->CaptureScene();
	}
}
