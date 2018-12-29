// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalDoorComponent.h"
#include "Engine/Public/SceneView.h"
#include "Engine//LocalPlayer.h"
#include "Materials/Material.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Console.h"
#include "Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/BoxComponent.h"
#include "Delegates/Delegate.h"
#include "Throughable.h"
#include "CustomMeshComponent.h"

const FName UPortalDoorComponent::PORTAL_RANGE_NAME("PortalRange");
// Sets default values for this component's properties
UPortalDoorComponent::UPortalDoorComponent() : otherDoor(nullptr), doorCamera(nullptr), doorShowSelf(nullptr), bIsDoorOpenSelf(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

TArray<UPortalDoorComponent*> UPortalDoorComponent::portals;
// Called when the game starts
void UPortalDoorComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!portals.Contains(this))
	{
		portals.Add(this);
	}
	// ...
	
}


// Called every frame
void UPortalDoorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsDoorOpen())
	{
		//TODO find throughable components in portal range;
	}
	// ...
}

void UPortalDoorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (portals.Contains(this))
	{
		portals.Remove(this);
		SetOtherDoor(nullptr);
	}
	Super::EndPlay(EndPlayReason);
}

const TArray<UPortalDoorComponent*>& UPortalDoorComponent::GetAllPortals()
{
	return portals;
	// TODO: 在此处插入 return 语句
}

void UPortalDoorComponent::InitPortalDoor(const USceneCaptureComponent2D * camera, const UCustomMeshComponent * model, const UMaterial * mat, bool isOpenSelfFirst)
{
	doorCamera = const_cast<USceneCaptureComponent2D*>(camera);
	doorShowSelf = const_cast<UCustomMeshComponent*>(model);
	bIsDoorOpenSelf = isOpenSelfFirst;
	if (doorCamera != nullptr)
	{
		doorCamera->bCaptureEveryFrame = false;
	}

	
	if (doorShowSelf != nullptr)
	{
		auto doorHalfSize = portalSize / 2;
		TArray<FCustomMeshTriangle> triggles;
		FCustomMeshTriangle triggle;
		triggle.Vertex0 = FVector(0,-doorHalfSize.X, doorHalfSize.Y);
		triggle.Vertex1 = FVector(0, doorHalfSize.X, doorHalfSize.Y);
		triggle.Vertex2 = FVector(0, -doorHalfSize.X, -doorHalfSize.Y);
		FCustomMeshTriangle triggle1;
		triggle1.Vertex0 = FVector(0, -doorHalfSize.X, -doorHalfSize.Y);
		triggle1.Vertex1 = FVector(0, doorHalfSize.X, doorHalfSize.Y);
		triggle1.Vertex2 = FVector(0, doorHalfSize.X, -doorHalfSize.Y);
		triggles.Add(triggle);
		triggles.Add(triggle1);
		doorShowSelf->SetCustomMeshTriangles(triggles);
		
		meshTriggles.Empty();
		meshTriggles.Add(triggle);
		meshTriggles.Add(triggle1);
	}
	if (originMat != const_cast<UMaterial*>(mat))
	{
		originMat = const_cast<UMaterial*>(mat);
		if (originMat != nullptr)
		{
			InstanceMaterial();
		}
	}
	AActor* ownerAct = GetOwner();
	portalRange = nullptr;
	auto boxeCs = ownerAct->GetComponentsByClass(UBoxComponent::StaticClass());
	for (int i = 0; i < boxeCs.Num(); ++i)
	{
		auto boxref = Cast<UBoxComponent>(boxeCs[i]);
		if (Cast<UCustomMeshComponent>(boxref->GetAttachParent()) != nullptr && Cast<UCustomMeshComponent>(boxref->GetAttachParent()) == doorShowSelf && boxref->GetFName().IsEqual(PORTAL_RANGE_NAME))
		{
			portalRange = boxref;
			break;
		}
	}
	if (portalRange == nullptr)
	{
		portalRange = NewObject<UBoxComponent>(ownerAct, PORTAL_RANGE_NAME);
		portalRange->RegisterComponent();
		//ownerAct->AddInstanceComponent(portalRange);
		if(doorShowSelf != nullptr)
			portalRange->AttachToComponent(doorShowSelf, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	}
	
	
	FVector extends;
	if (doorShowSelf != nullptr)
	{
		portalRange->SetRelativeLocation(FVector(portalRangeZLength / 2, 0, 0) * doorShowSelf->GetComponentScale());
		portalRange->SetRelativeRotation(FQuat::Identity);
		extends = FVector(portalRangeZLength / 2, portalSize.X/2, portalSize.Y/2);
		
	}
	portalRange->SetBoxExtent(extends);
	portalRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	portalRange->SetGenerateOverlapEvents(true);
	portalRange->SetActive(true);
	
	portalRange->OnComponentBeginOverlap.Clear();
	//portalRange->OnComponentBeginOverlap.AddDynamic(this, &UPortalDoorComponent::TestDynamicAddComponent);
}

void UPortalDoorComponent::SetOtherDoor(UPortalDoorComponent * other)
{
	if (otherDoor == other)
		return;

	if (otherDoor != nullptr)
	{
		otherDoor->otherDoor = nullptr;
	}
	if (other != nullptr)
	{
		other->SetOtherDoor(nullptr);
		other->otherDoor = this;
	}
	otherDoor = other;
	if (bIsDoorOpen())
	{
		doorCamera->bCaptureEveryFrame = false;
		doorCamera->bCaptureOnMovement = false;
		doorCamera->bCameraCutThisFrame = true;
	}
}

UPortalDoorComponent * UPortalDoorComponent::GetOtherDoor() const
{
	return otherDoor;
}

bool UPortalDoorComponent::bIsDoorOpen() const
{
	bool otherOpen = otherDoor != nullptr && otherDoor->bIsDoorOpenSelf && otherDoor->doorCamera != nullptr && otherDoor->doorShowSelf != nullptr &&otherDoor->originMat != nullptr;
	return bIsDoorOpenSelf && otherOpen && doorCamera != nullptr && doorShowSelf != nullptr && originMat != nullptr;
	
}

bool UPortalDoorComponent::ShouldRender(USceneCaptureComponent2D * capture, FBox lastBox) const
{
	
	if (bIsDoorOpen())
	{
		FBox myBox = GetSceneComponentScreenBox(this->doorShowSelf, meshTriggles, capture);
		FBox2D myRect(FVector2D(myBox.Min.X, myBox.Min.Y), FVector2D(myBox.Max.X, myBox.Max.Y));
		FBox2D lastRect(FVector2D(lastBox.Min.X, lastBox.Min.Y), FVector2D(lastBox.Max.X, lastBox.Max.Y));
		//return bIsDoorOpen();
		FVector dir = doorShowSelf->GetComponentLocation() - capture->GetComponentLocation();
		bool angleRe = FVector::DotProduct(dir, doorShowSelf->GetForwardVector()) <= 0;

		return myRect.Intersect(lastRect) && myBox.Max.Z > lastBox.Min.Z && myBox.Max.Z > GNearClippingPlane && angleRe;
	}
	return false;
}

void UPortalDoorComponent::InstanceMaterial()
{
	if (doorShowSelf != nullptr)
	{
		if (originMat != nullptr)
		{
			doorShowSelf->CreateDynamicMaterialInstance(0, originMat);
		}
	}
}

int UPortalDoorComponent::GetNowNowPortalDoorNum()
{
	return portals.Num();
}

void UPortalDoorComponent::OriginMaterial(const UMaterial * origin)
{
	if (originMat != origin)
	{
		originMat = const_cast<UMaterial*>(origin);
		if (originMat != nullptr)
		{
			InstanceMaterial();
		}
	}
}

void UPortalDoorComponent::TestDynamicAddComponent()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, "Dynamic Component Touched");
	auto res = portalRange;
	if (res != nullptr)
	{
		UBoxComponent* box = Cast<UBoxComponent>(res);
		TSet<UPrimitiveComponent*> overlaped;
		TSet<UPrimitiveComponent*> overlapedThroughable;
		box->GetOverlappingComponents(overlaped);
		for (auto& comp : overlaped)
		{
			//check if the comp is a throughable component, if so, add it to overlapedThroughable set
			if (comp->GetOwner() != nullptr)
			{
				auto thrs = comp->GetOwner()->GetComponents();
				for (auto& thr : thrs)
				{
					if (thr->GetClass()->Implements<UThroughable>())
					{
						if (Cast<IThroughable>(thr)->throughableComponent.IsValid() && Cast<IThroughable>(thr)->throughableComponent.Get() == comp)
						{
							overlapedThroughable.Add(comp);
							break;
						}
					}
				}
			}
		}
		//TODO compair current overlaped throughable and the new one then update;
	}
}

void UPortalDoorComponent::BuildProjectionMatrix(FIntPoint RenderTargetSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, FMatrix& ProjectionMatrix)
{
	float const XAxisMultiplier = 1.0f;
	float const YAxisMultiplier = RenderTargetSize.X / (float)RenderTargetSize.Y;

	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float OrthoWidth = InOrthoWidth / 2.0f;
		const float OrthoHeight = InOrthoWidth / 2.0f * XAxisMultiplier / YAxisMultiplier;

		const float NearPlane = 0;
		const float FarPlane = WORLD_MAX / 8.0f;

		const float ZScale = 1.0f / (FarPlane - NearPlane);
		const float ZOffset = -NearPlane;

		ProjectionMatrix = FReversedZOrthoMatrix(
			OrthoWidth,
			OrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		if ((int32)ERHIZBuffer::IsInverted)
		{
			ProjectionMatrix = FReversedZPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
		else
		{
			ProjectionMatrix = FPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
	}
}

FVector UPortalDoorComponent::ProjectWorldToScreen(const FVector & worldPos, const FMatrix & projMatrix, bool bKeepZ)
{
	FPlane posInViewSpace = FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1)).TransformFVector4(FVector4(worldPos, 1.f));
	FPlane Result = projMatrix.TransformFVector4(posInViewSpace);

	if (FMath::Abs(Result.W) > 0.0003f)
	{
		const float RHW = 1.0f / Result.W;
		FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

		// Move from projection space to normalized 0..1 UI space
		/*const float NormalizedX = (PosInScreenSpace.X / 2.f) + 0.5f;
		const float NormalizedY = 1.f - (PosInScreenSpace.Y / 2.f) - 0.5f;*/
		FVector ret(PosInScreenSpace.X, PosInScreenSpace.Y, bKeepZ ? worldPos.Z : PosInScreenSpace.Z);
		return ret;
	}
	// the result of this will be x and y coords in -1..1 projection space
	

	return FVector(0,0,0);
}

FBox UPortalDoorComponent::GetSceneComponentScreenBox(const UCustomMeshComponent * sceneCom, const TArray<FCustomMeshTriangle>& triggles, USceneCaptureComponent2D * capture)
{
	TArray<FVector> point;
	FMatrix res;
	BuildProjectionMatrix(GEngine->GameViewport->Viewport->GetSizeXY(),
		capture->ProjectionType,
		capture->FOVAngle * (float)PI / 360.0f, capture->OrthoWidth, res);
	//auto selfBounds = sceneCom->CalcBounds(sceneCom->GetComponentTransform().GetRelativeTransform(capture->GetComponentTransform()));
	const FTransform& componentToWorld = sceneCom->GetComponentTransform();
	const FTransform& worldToEye = capture->GetComponentTransform().Inverse();
	for (int i = 0; i < triggles.Num(); ++i)
	{
		FVector pointInWrold[] = { 
			componentToWorld.TransformPosition(triggles[i].Vertex0),
			componentToWorld.TransformPosition(triggles[i].Vertex1), 
			componentToWorld.TransformPosition(triggles[i].Vertex2) };
		int lines[] = { 0,1,1,2,2,3 };
		for (int j = 0; j < 6; j += 2)
		{
			FVector p1 = worldToEye.TransformPosition(pointInWrold[lines[j]]);
			FVector p2 = worldToEye.TransformPosition(pointInWrold[lines[j + 1]]);
			if (p1.X - GNearClippingPlane >= 0)
			{
				point.Add(p1);
			}
			if (p2.X - GNearClippingPlane >= 0)
			{
				point.Add(p2);
			}
			if ((p1.X - GNearClippingPlane) * (p2.X - GNearClippingPlane) < 0)
			{
				FVector dir = p1 - p2;
				dir.Normalize();
				float ratio = FMath::Abs(GNearClippingPlane - p2.X) / FMath::Abs(p1.X - p2.X);
				point.Add(p2 + dir * ratio * (p1 - p2).Size());
			}
		}
	}

	/*point[0] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[1] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[2] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[3] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	point[4] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	point[5] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	point[6] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[7] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);*/
	TArray<FVector> screenPoint;
	for (int i = 0; i < point.Num(); ++i)
	{
		screenPoint.Add(ProjectWorldToScreen(point[i], res, true));	
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, sceneCom->GetOwner()->GetName() + ": " + screenPoint[i].ToString());
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, sceneCom->GetOwner()->GetName() + ": " + ProjectWorldToScreen(selfBounds.Origin,res,true).ToString());
	if (screenPoint.Num() <= 0)
	{
		return FBox(FVector(0, 0, -1000), FVector(1, 1, -999));
	}

	float xMin = screenPoint[0].X;
	float xMax = screenPoint[0].X;
	float yMin = screenPoint[0].Y;
	float yMax = screenPoint[0].Y;
	float zMin = screenPoint[0].Z;
	float zMax = screenPoint[0].Z;
	for (int i = 0; i < screenPoint.Num(); ++i)
	{
		if (screenPoint[i].X < xMin)
		{
			xMin = screenPoint[i].X;
		}
		if (screenPoint[i].Y < yMin)
		{
			yMin = screenPoint[i].Y;
		}
		if (screenPoint[i].Z < zMin)
		{
			zMin = screenPoint[i].Z;
		}
		if (screenPoint[i].X > xMax)
		{
			xMax = screenPoint[i].X;
		}
		if (screenPoint[i].Y > yMax)
		{
			yMax = screenPoint[i].Y;
		}
		if (screenPoint[i].Z > zMax)
		{
			zMax = screenPoint[i].Z;
		}
	}
	return FBox(FVector(xMin, yMin, zMin), FVector(xMax,yMax,zMax));
}
