// Fill out your copyright notice in the Description page of Project Settings.

#include "PPortalTree.h"
#include "PPortalNode.h"
#include "Engine.h"
#include "PortalDoorComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Camera/CameraComponent.h"
#include "PPortalComponent.h"

const FName UPPortalTree::BACK_CAMERA_NAME("BackCamera");
const FName UPPortalTree::BACK_CAMERA_MASK_PARA_NAME("PortalMask");
const FName UPPortalTree::BACK_CAMERA_RENDER_PARA_NAME("PortalRender");

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

void UPPortalTree::InitPortalTree(const USceneCaptureComponent2D* root, AActor* motherActor, UCameraComponent* camera, UMaterialInterface* back, UPPortalComponent* mot)
{
	outMot = mot;
	doorShowBack = back;
	rootCamera = const_cast<USceneCaptureComponent2D*>(root);
	rootNode = QureyPortalNode(0);
	rootCamera->bCaptureEveryFrame = false;
	sceneCamera = camera;
	auto captures = motherActor->GetComponentsByClass(USceneCaptureComponent2D::StaticClass());
	anotherSc = nullptr;
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
	if (anotherRT == nullptr)
	{
		FIntPoint reslution = GEngine->GameViewport->Viewport->GetSizeXY();

		anotherRT = NewObject<UTextureRenderTarget2D>();
		anotherRT->InitAutoFormat(reslution.X, reslution.Y);
		anotherRT->ClearColor = FLinearColor(0, 0, 0, 1);
		anotherRT->UpdateResourceImmediate();
	}
	anotherSc->TextureTarget = anotherRT;
	anotherSc->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	anotherSc->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
	anotherSc->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	anotherSc->bCaptureOnMovement = false;
	anotherSc->bCameraCutThisFrame = true;
	if (sceneCamera != nullptr && sceneCamera->PostProcessSettings.WeightedBlendables.Array.Num() > 0)
	{
		UMaterialInterface* matInf = Cast<UMaterialInterface>(sceneCamera->PostProcessSettings.WeightedBlendables.Array[0].Object);
		UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(matInf, nullptr);
		sceneCamera->PostProcessSettings.WeightedBlendables.Array[0].Object = mat;
		sceneCamera->PostProcessSettings.WeightedBlendables.Array[0].Weight = 1;
		if (mat != nullptr)
		{
			UTexture* param;
			if (mat->GetTextureParameterValue(FMaterialParameterInfo(BACK_CAMERA_MASK_PARA_NAME), param))
			{
				mat->SetTextureParameterValue(BACK_CAMERA_MASK_PARA_NAME, anotherRT);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "ERROR: No Suitable Parameter Found In The First PostProcessMaterial");
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "ERROR: First PostProcessMaterial Is Not A Material Instance Dynamic!");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "ERROR: No Specific Camera Or Do Not Have PostProcessMaterial!");
	}
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
		FBox(FVector(-1, -1, 0), FVector(1, 1, 0)) : 
		UPortalDoorComponent::GetSceneComponentScreenBox(node->portalDoor->GetOtherDoor()->doorShowSelf, node->portalDoor->GetOtherDoor()->meshTriggles, curCam);
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
		FVector4 PortalPos(0, 0, 300, 1);
		FVector4 PortalNor(0, 0, -1, 0);
		float NearPlane = GNearClippingPlane;
		UPortalDoorComponent* passing = nullptr;
		if (outMot != nullptr && outMot->passingPortal.IsValid())
		{
			passing = outMot->passingPortal.Get();
			FMatrix changeAxis(
				FPlane(0, 0, 1, 0),
				FPlane(1, 0, 0, 0),
				FPlane(0, 1, 0, 0),
				FPlane(0, 0, 0, 1));
			const auto& toEye = sceneCamera->GetComponentTransform().Inverse();
			PortalPos = changeAxis.TransformFVector4(toEye.TransformFVector4(FVector4(passing->doorShowSelf->GetComponentLocation(),1)));
			PortalNor = changeAxis.TransformFVector4(toEye.TransformFVector4(FVector4(passing->doorShowSelf->GetForwardVector(),0)));
		}

		if (sceneCamera != nullptr && sceneCamera->PostProcessSettings.WeightedBlendables.Array.Num() > 0)
		{
			UMaterialInterface* matInf = Cast<UMaterialInterface>(sceneCamera->PostProcessSettings.WeightedBlendables.Array[0].Object);
			UMaterialInstanceDynamic* mat = Cast<UMaterialInstanceDynamic>(matInf);
			if (mat != nullptr)
			{
				mat->SetVectorParameterValue(FName("PortalPos"), FLinearColor(PortalPos));
				mat->SetVectorParameterValue(FName("PortalNor"), FLinearColor(PortalNor));
				mat->SetScalarParameterValue(FName("NearPlane"), NearPlane);
			}
		}

		for (int i = 0; i < node->childrenNode.Num(); ++i)
		{
			auto mesh = node->childrenNode[i]->portalDoor->doorShowSelf;
			auto matInstDyn = Cast<UMaterialInstanceDynamic>(mesh->GetMaterial(0));
			matInstDyn->SetTextureParameterValue(FName("_MainTex"), node->childrenNode[i]->renderTexture);
			if (passing != nullptr && passing == node->childrenNode[i]->portalDoor)
			{
				if (sceneCamera != nullptr && sceneCamera->PostProcessSettings.WeightedBlendables.Array.Num() > 0)
				{
					UMaterialInterface* matInf = Cast<UMaterialInterface>(sceneCamera->PostProcessSettings.WeightedBlendables.Array[0].Object);
					UMaterialInstanceDynamic* mat = Cast<UMaterialInstanceDynamic>(matInf);
					if (mat != nullptr)
					{
						mat->SetTextureParameterValue(FName("PortalRender"), node->childrenNode[i]->renderTexture);
						GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString("Touched ") + passing->GetName());
					}
				}
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
			auto mesh = node->childrenNode[i]->portalDoor->doorShowSelf;
			auto matInstDyn = Cast<UMaterialInstanceDynamic>(mesh->GetMaterial(0));
			matInstDyn->SetTextureParameterValue(FName("_MainTex"), node->childrenNode[i]->renderTexture);
		}
		node->portalDoor->doorCamera->CaptureScene();
	}
}
