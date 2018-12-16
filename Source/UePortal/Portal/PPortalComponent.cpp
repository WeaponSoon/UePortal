// Fill out your copyright notice in the Description page of Project Settings.

#include "PPortalComponent.h"

// Sets default values for this component's properties
UPPortalComponent::UPPortalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}


void UPPortalComponent::SetPortalTree(const USceneCaptureComponent2D * capture)
{
	rootCapture = const_cast<USceneCaptureComponent2D*>(capture);
	portalTree = NewObject<UPPortalTree>();
	portalTree->InitPortalTree(rootCapture);
}

// Called when the game starts
void UPPortalComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	
}


// Called every frame
void UPPortalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	portalTree->BuildPortalTree();
	portalTree->RenderPortalTree();
	// ...
}

