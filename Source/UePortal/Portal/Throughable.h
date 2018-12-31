// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PortalDoorComponent.h"
#include "Throughable.generated.h"

// This class does not need to be modified.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEPORTAL_API UThroughableComponent : public UActorComponent
{
	GENERATED_BODY()
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	TSet<TWeakObjectPtr<UPortalDoorComponent>> nearPortals;
	TWeakObjectPtr<UPortalDoorComponent> passingPortal;
	TWeakObjectPtr<USceneComponent> throughableComponent;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
		void AddNearPortalDoor(UPortalDoorComponent* nearPortal);
	UFUNCTION()
		void RemoveNearPortalDoor(UPortalDoorComponent* nearPortal);
	UFUNCTION()
		void UpdatePassingPortal();
	UFUNCTION(BlueprintCallable, Category="Portal Component")
		void SetThroughableComponent(class USceneComponent* throughable);
	static const TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>>& GetThroughableMap();
protected:
	UFUNCTION()
		virtual void OnSetThroughableComponent(class USceneComponent* oldOne, class USceneComponent* newOne);
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginPlay() override;

private:
	static TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>> throughableMap;
};
