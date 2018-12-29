// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CustomMeshComponent.h"
#include "PortalDoorComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEPORTAL_API UPortalDoorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPortalDoorComponent();
	UPROPERTY(BlueprintReadOnly, Category = "Portal Component")
		class UMaterial* originMat;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal Component")
		bool bIsDoorOpenSelf;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal Component")
		USceneCaptureComponent2D* doorCamera;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal Component")
		class UCustomMeshComponent* doorShowSelf;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal Component")
		FVector2D portalSize;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal Component")
		float portalRangeZLength;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Portal Component")
		TArray<FCustomMeshTriangle> meshTriggles;
private:
	static TArray<UPortalDoorComponent*> portals;
	const static FName PORTAL_RANGE_NAME;
	UPROPERTY()
		UPortalDoorComponent* otherDoor;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static const TArray<UPortalDoorComponent*>& GetAllPortals();

	static void BuildProjectionMatrix(FIntPoint RenderTargetSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, FMatrix& ProjectionMatrix);
	static FVector ProjectWorldToScreen(const FVector& worldPos, const FMatrix& projMatrix, bool bKeepZ = false);
	static FBox GetSceneComponentScreenBox(const UCustomMeshComponent* sceneCom, const TArray<FCustomMeshTriangle>& triggles,USceneCaptureComponent2D* capture);

	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		void InitPortalDoor(const USceneCaptureComponent2D* camera, const UCustomMeshComponent* model, const UMaterial* mat, bool isOpenSelfFirst);
	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		void SetOtherDoor(UPortalDoorComponent* other);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal Component")
		UPortalDoorComponent* GetOtherDoor() const;
	UFUNCTION(BlueprintGetter, Category = "Portal Component")
		bool bIsDoorOpen() const;
	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		bool ShouldRender(USceneCaptureComponent2D* capture, FBox lastBox) const;
	UFUNCTION(BlueprintCallable, Category = "Portal Component")
		void InstanceMaterial();
	UFUNCTION(BlueprintGetter, Category = "Portal Component")
		int GetNowNowPortalDoorNum();
	UFUNCTION(BlueprintSetter, Category = "Portal Component")
		void OriginMaterial(const UMaterial* origin);
	UFUNCTION()
		void TestDynamicAddComponent(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
