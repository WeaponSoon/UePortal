// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "PPortalTree.h"
#include "Throughable.h"
#include "PPortalComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEPORTAL_API UPPortalComponent : public UActorComponent, public IThroughable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPPortalComponent();
	UPROPERTY()
		USceneCaptureComponent2D* rootCapture;
	UPROPERTY()
		UPPortalTree* portalTree;
	UPROPERTY()
		class UPortalDoorComponent* inRangePortalDoor;
	UPROPERTY()
		class UCameraComponent* mainCamera;

	UFUNCTION(BlueprintCallable, Category="Portal Component")
		void SetPortalTree(const USceneCaptureComponent2D* capture, int32 maxLayer, class UCameraComponent* sceneCamera, class UMaterialInterface* backMat);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
