// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalDoorComponent.h"
#include "Engine/Public/SceneView.h"
#include "Engine//LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UPortalDoorComponent::UPortalDoorComponent()
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

	// ...
}

void UPortalDoorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (portals.Contains(this))
	{
		portals.Remove(this);
	}
	Super::EndPlay(EndPlayReason);
}

const TArray<UPortalDoorComponent*>& UPortalDoorComponent::GetAllPortals()
{
	return portals;
	// TODO: �ڴ˴����� return ���
}

void UPortalDoorComponent::SetOtherDoor(UPortalDoorComponent * other)
{
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
}

UPortalDoorComponent * UPortalDoorComponent::GetOtherDoor() const
{
	return otherDoor;
}

bool UPortalDoorComponent::bIsDoorOpen() const
{
	bool otherOpen = otherDoor != nullptr && otherDoor->bIsDoorOpenSelf && otherDoor->doorCamera != nullptr && otherDoor->doorShowSelf != nullptr;
	return bIsDoorOpenSelf && otherOpen && doorCamera != nullptr && doorShowSelf != nullptr;
	
}

bool UPortalDoorComponent::ShouldRender(USceneCaptureComponent2D * capture, FBox lastBox) const
{
	if (bIsDoorOpen())
	{
		FBox myBox = GetSceneComponentScreenBox(this->doorShowSelf, capture);
		FBox2D myRect(FVector2D(myBox.Min.X, myBox.Min.Y), FVector2D(myBox.Max.X, myBox.Max.Y));
		FBox2D lastRect(FVector2D(lastBox.Min.X, lastBox.Min.Y), FVector2D(lastBox.Max.X, lastBox.Max.Y));
		return myRect.Intersect(lastRect) && myBox.Max.Z > lastBox.Min.Z;
	}
	return false;
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

FVector UPortalDoorComponent::ProjectWorldToScreen(const FVector & worldPos, const FMatrix & projMatrix)
{
	FPlane Result = projMatrix.TransformFVector4(FVector4(worldPos, 1.f));

	if (FMath::Abs(Result.W) > 0.0003f)
	{
		const float RHW = 1.0f / Result.W;
		FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

		// Move from projection space to normalized 0..1 UI space
		/*const float NormalizedX = (PosInScreenSpace.X / 2.f) + 0.5f;
		const float NormalizedY = 1.f - (PosInScreenSpace.Y / 2.f) - 0.5f;*/
		return FVector(PosInScreenSpace.X, PosInScreenSpace.Y, PosInScreenSpace.Z);
	}
	// the result of this will be x and y coords in -1..1 projection space
	

	return FVector(0,0,0);
}

FBox UPortalDoorComponent::GetSceneComponentScreenBox(const USceneComponent * sceneCom, USceneCaptureComponent2D * capture)
{
	FMatrix res;
	BuildProjectionMatrix(FIntPoint(capture->TextureTarget->GetSurfaceWidth(), capture->TextureTarget->GetSurfaceHeight()),
		capture->ProjectionType,
		capture->FOVAngle * (float)PI / 360.0f, capture->OrthoWidth, res);
	auto selfBounds = sceneCom->CalcBounds(sceneCom->GetComponentTransform().GetRelativeTransform(capture->GetComponentTransform()));

	FVector point[8];
	point[0] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[1] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[2] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[3] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	point[4] = selfBounds.Origin + FVector(selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	point[5] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	point[6] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, selfBounds.BoxExtent.Z);
	point[7] = selfBounds.Origin + FVector(-selfBounds.BoxExtent.X, -selfBounds.BoxExtent.Y, -selfBounds.BoxExtent.Z);
	FVector screenPoint[8];
	for (int i = 0; i < 8; ++i)
	{
		screenPoint[i] = ProjectWorldToScreen(point[i], res);
	}

	float xMin = screenPoint[0].X;
	float xMax = screenPoint[0].X;
	float yMin = screenPoint[0].Y;
	float yMax = screenPoint[0].Y;
	float zMin = screenPoint[0].Z;
	float zMax = screenPoint[0].Z;
	for (int i = 0; i < 8; ++i)
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
		if (screenPoint[i].Z < zMax)
		{
			zMax = screenPoint[i].Z;
		}
	}
	return FBox(FVector(xMin, yMin, zMin), FVector(xMax,yMax,zMax));
}
