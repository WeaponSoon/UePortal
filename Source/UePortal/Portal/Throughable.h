// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PortalDoorComponent.h"
#include "Throughable.generated.h"

class UThroughableComponent;

USTRUCT()
struct FThroughablePrePhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

		/** CharacterMovementComponent that is the target of this tick **/
		class UThroughableComponent* Target;

	/**
	 * Abstract function actually execute the tick.
	 * @param DeltaTime - frame time to advance, in seconds
	 * @param TickType - kind of tick for this frame
	 * @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
	 * @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completion of this task until certain child tasks are complete.
	 **/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FThroughablePrePhysicsTickFunction> : public TStructOpsTypeTraitsBase2<FThroughablePrePhysicsTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

USTRUCT()
struct FThroughablePostPhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

		/** CharacterMovementComponent that is the target of this tick **/
		class UThroughableComponent* Target;

	/**
	 * Abstract function actually execute the tick.
	 * @param DeltaTime - frame time to advance, in seconds
	 * @param TickType - kind of tick for this frame
	 * @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
	 * @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completion of this task until certain child tasks are complete.
	 **/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FThroughablePostPhysicsTickFunction> : public TStructOpsTypeTraitsBase2<FThroughablePostPhysicsTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

// This class does not need to be modified.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEPORTAL_API UThroughableComponent : public UActorComponent
{
	GENERATED_BODY()
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UThroughableComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY()
		struct FThroughablePostPhysicsTickFunction PostPhysicsTickFunction;
	UPROPERTY()
		struct FThroughablePrePhysicsTickFunction PrePhysicsTickFunction;


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
	virtual void PrePhysics(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent);
	virtual void PostPhysics(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent);
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
protected:
	UFUNCTION()
		virtual void OnSetThroughableComponent(class USceneComponent* oldOne, class USceneComponent* newOne);
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginPlay() override;
	
private:
	static TMap<TWeakObjectPtr<USceneComponent>, TWeakObjectPtr<UThroughableComponent>> throughableMap;
};
