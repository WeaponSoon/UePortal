// Fill out your copyright notice in the Description page of Project Settings.
#include "PPortalComponent.h"
#include "GameFramework/Pawn.h"

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
	if (portalTree != nullptr)
	{
		portalTree = nullptr;
	}
	if (capture == nullptr)
		return;
	rootCapture = const_cast<USceneCaptureComponent2D*>(capture);
	portalTree = NewObject<UPPortalTree>();
	portalTree->InitPortalTree(rootCapture, GetOwner(), camera, backMat);
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


// Called every frame
void UPPortalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (ownerPawn!= nullptr && ownerPawn->IsLocallyControlled() && portalTree != nullptr && rootCapture != nullptr)
	{
		portalTree->BuildPortalTree();
		portalTree->RenderPortalTree();
	}
	
	// ...
}

