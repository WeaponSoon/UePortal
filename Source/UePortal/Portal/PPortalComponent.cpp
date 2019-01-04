// Fill out your copyright notice in the Description page of Project Settings.
#include "PPortalComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Engine.h"

const FName UPPortalComponent::CAMERA_RANGE("CameraRange");
// Sets default values for this component's properties
UPPortalComponent::UPPortalComponent()
{
	inRangePortalDoor = nullptr;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	
	// ...
}


void UPPortalComponent::SetPortalTree(const USceneCaptureComponent2D * capture, int32 maxLayer, UCameraComponent* camera, UMaterialInterface* backMat)
{
	PostPhysicsTickFunction.TickGroup = ETickingGroup::TG_PostPhysics;
	PrePhysicsTickFunction.TickGroup = ETickingGroup::TG_PrePhysics;
	if (portalTree != nullptr)
	{
		portalTree = nullptr;
	}
	if (capture == nullptr)
		return;
	rootCapture = const_cast<USceneCaptureComponent2D*>(capture);
	portalTree = NewObject<UPPortalTree>();
	mainCamera = camera;

	auto aspectRatio = camera->AspectRatio;
	auto nearPlane = GNearClippingPlane;
	auto fieldOfView = camera->FieldOfView;
	float halfHeight = nearPlane * FMath::Tan(fieldOfView * PI / 360);
	float halfWidth = aspectRatio * halfHeight;
	float radius = FVector(halfHeight, halfWidth, nearPlane).Size();
	const auto& spheres = GetOwner()->GetComponentsByClass(USphereComponent::StaticClass());
	USphereComponent* cameraCol = nullptr;
	for (auto& sphere : spheres)
	{
		auto sphereR = Cast<USphereComponent>(sphere);
		if (sphereR->GetAttachParent() != nullptr && sphereR->GetAttachParent() == camera && sphereR->GetAttachParent()->GetFName().IsEqual(CAMERA_RANGE))
		{
			cameraCol = sphereR;
		}
	}
	if (cameraCol == nullptr)
	{
		cameraCol = NewObject<USphereComponent>(GetOwner(), CAMERA_RANGE);
		cameraCol->RegisterComponent();
		
	}
	cameraCol->AttachToComponent(camera, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	cameraCol->SetRelativeLocation(FVector::ZeroVector);
	cameraCol->SetSphereRadius(radius);
	//cameraCol->bDynamicObstacle = true;
	//cameraCol->UpdateOverlaps();
	SetThroughableComponent(cameraCol);
	portalTree->InitPortalTree(rootCapture, GetOwner(), camera, backMat, this);
	rootCapture->bCaptureEveryFrame = false;
	portalTree->maxLayer = maxLayer;
}

// Called when the game starts
void UPPortalComponent::BeginPlay()
{
	Super::BeginPlay();
	
	/*if (ownerPawn != nullptr && portalTree != nullptr && rootCapture != nullptr)
	{
			
		portalTree->BuildPortalTree();
		portalTree->RenderPortalTree();
	}*/
	// ...
	
}

void UPPortalComponent::OnSetThroughableComponent(USceneComponent * oldOne, USceneComponent * newOne)
{
	if (newOne != nullptr && Cast<UCameraComponent>(newOne->GetAttachParent()) != nullptr)
	{
		
		
	}
}


// Called every frame
void UPPortalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (ownerPawn!= nullptr && ownerPawn->IsLocallyControlled() && portalTree != nullptr && rootCapture != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString("Dynamic ") + FString::FromInt(nearPortals.Num()));
		portalTree->BuildPortalTree();
		portalTree->RenderPortalTree();
	}
	
	// ...
}


