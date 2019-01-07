// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalWorldSettings.h"
#include "Engine/World.h"
#include "Engine.h"
#include "Physics/PhysicsFiltering.h"
#include "PPortalComponent.h"

const TArray<APortalWorldSettings::IgnorePair>& APortalWorldSettings::GetIgnoreComponents()
{
	return ignoreComponents;
}

bool APortalWorldSettings::IgnoreBetween(USceneComponent* a, USceneComponent* b)
{
	
	TArray<int32> recoveryPair;
	for (int32 i = 0; i < ignoreComponents.Num(); ++i)
	{
		if (!ignoreComponents[i].obj1.IsValid() || !ignoreComponents[i].obj2.IsValid())
		{
			recoveryPair.Add(i);
		}
	}
	for (int32 i = 0; i < recoveryPair.Num(); ++i)
	{
		ignoreComponents.RemoveAt(recoveryPair[i]);
	}
	recoveryPair.Empty();
	for (int32 i = 0; i < ignoreComponents.Num(); ++i)
	{
		if ((ignoreComponents[i].obj1.Get() == a && ignoreComponents[i].obj2.Get() == b) ||
			(ignoreComponents[i].obj2.Get() == a && ignoreComponents[i].obj1.Get() == b))
		{
			recoveryPair.Add(i);
		}
	}
	for (int32 i = 0; i < recoveryPair.Num(); ++i)
	{
		ignoreComponents.RemoveAt(recoveryPair[i]);
	}
	if (a != nullptr && b != nullptr)
	{
		IgnorePair temp;
		temp.obj1 = a;
		temp.obj2 = b;
		ignoreComponents.Add(temp);
		return true;
	}
	return false;
}

bool APortalWorldSettings::RemoveIgnoreBetween(USceneComponent * a, USceneComponent * b)
{
	IgnoreBetween(nullptr, nullptr);
	TArray<int32> willDels;
	for (int32 i = 0; i < ignoreComponents.Num(); ++i)
	{
		if ((ignoreComponents[i].obj1.Get() == a && ignoreComponents[i].obj2.Get() == b) ||
			(ignoreComponents[i].obj2.Get() == a && ignoreComponents[i].obj1.Get() == b))
		{
			willDels.Add(i);
		}
	}
	bool res = false;
	for (int32 i = 0; i < willDels.Num(); ++i)
	{
		ignoreComponents.RemoveAt(willDels[i]);
		res = true;
	}
	return res;
}

static PxFilterFlags PhysXSimFilterShaderPortal(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	//UE_LOG(LogPhysics, Log, TEXT("filterData0 (%s): %x %x %x %x"), *ObjTypeToString(attributes0), filterData0.word0, filterData0.word1, filterData0.word2, filterData0.word3);
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "New Phyics Scene");



	APortalWorldSettings* settings = nullptr;
	if (UPPortalComponent::currentWorld && UPPortalComponent::currentWorld->GetWorldSettings())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, UPPortalComponent::currentWorld->GetWorldSettings()->GetClass()->GetName());
		settings = Cast<APortalWorldSettings>(UPPortalComponent::currentWorld->GetWorldSettings());
	}
	if (settings != nullptr)
	{
		const auto& ignoreComponents = settings->GetIgnoreComponents();

		for (int i = 0; i < ignoreComponents.Num(); ++i)
		{
			const auto& pair = ignoreComponents[i];
			if (pair.obj1.IsValid() && pair.obj2.IsValid())
			{
				if ((pair.obj1->GetUniqueID() == filterData0.word2 && pair.obj2->GetUniqueID() == filterData1.word2) ||
					(pair.obj2->GetUniqueID() == filterData0.word2 && pair.obj1->GetUniqueID() == filterData1.word2))
				{
					return PxFilterFlag::eSUPPRESS;
				}
			}
		}
	}

	

	//return PxFilterFlag::eKILL;
	bool k0 = PxFilterObjectIsKinematic(attributes0);
	bool k1 = PxFilterObjectIsKinematic(attributes1);

	PxU32 FilterFlags0 = (filterData0.word3 & 0xFFFFFF);
	PxU32 FilterFlags1 = (filterData1.word3 & 0xFFFFFF);

	if (k0 && k1)
	{
		//Ignore kinematic kinematic pairs unless they are explicitly requested
		if (!(FilterFlags0&EPDF_KinematicKinematicPairs) && !(FilterFlags1&EPDF_KinematicKinematicPairs))
		{
			return PxFilterFlag::eSUPPRESS;	//NOTE: Waiting on physx fix for refiltering on aggregates. For now use supress which automatically tests when changes to simulation happen
		}
	}

	bool s0 = PxGetFilterObjectType(attributes0) == PxFilterObjectType::eRIGID_STATIC;
	bool s1 = PxGetFilterObjectType(attributes1) == PxFilterObjectType::eRIGID_STATIC;

	//ignore static-kinematic (this assumes that statics can't be flagged as kinematics)
	// should return eSUPPRESS here instead eKILL so that kinematics vs statics will still be considered once kinematics become dynamic (dying ragdoll case)
	if ((k0 || k1) && (s0 || s1))
	{
		return PxFilterFlag::eSUPPRESS;
	}

	// if these bodies are from the same component, use the disable table to see if we should disable collision. This case should only happen for things like skeletalmesh and destruction. The table is only created for skeletal mesh components at the moment
#if !WITH_APEIRON && !PHYSICS_INTERFACE_LLIMMEDIATE
	if (filterData0.word2 == filterData1.word2)
	{
		check(constantBlockSize == sizeof(FPhysSceneShaderInfo));
		const FPhysSceneShaderInfo* PhysSceneShaderInfo = (const FPhysSceneShaderInfo*)constantBlock;
		check(PhysSceneShaderInfo);
		FPhysScene * PhysScene = PhysSceneShaderInfo->PhysScene;
		check(PhysScene);

		const TMap<uint32, TMap<FRigidBodyIndexPair, bool> *> & CollisionDisableTableLookup = PhysScene->GetCollisionDisableTableLookup();
		TMap<FRigidBodyIndexPair, bool>* const * DisableTablePtrPtr = CollisionDisableTableLookup.Find(filterData1.word2);
		if (DisableTablePtrPtr)		//Since collision table is deferred during sub-stepping it's possible that we won't get the collision disable table until the next frame
		{
			TMap<FRigidBodyIndexPair, bool>* DisableTablePtr = *DisableTablePtrPtr;
			FRigidBodyIndexPair BodyPair(filterData0.word0, filterData1.word0); // body indexes are stored in word 0
			if (DisableTablePtr->Find(BodyPair))
			{
				return PxFilterFlag::eKILL;
			}

		}
	}
#endif

	// Find out which channels the objects are in
	ECollisionChannel Channel0 = GetCollisionChannel(filterData0.word3);
	ECollisionChannel Channel1 = GetCollisionChannel(filterData1.word3);

	// see if 0/1 would like to block the other 
	PxU32 BlockFlagTo1 = (ECC_TO_BITFIELD(Channel1) & filterData0.word1);
	PxU32 BlockFlagTo0 = (ECC_TO_BITFIELD(Channel0) & filterData1.word1);

	bool bDoesWantToBlock = (BlockFlagTo1 && BlockFlagTo0);

	// if don't want to block, suppress
	if (!bDoesWantToBlock)
	{
		return PxFilterFlag::eSUPPRESS;
	}



	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	//todo enabling CCD objects against everything else for now
	if (!(k0 && k1) && ((FilterFlags0&EPDF_CCD) || (FilterFlags1&EPDF_CCD)))
	{
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT | PxPairFlag::eSOLVE_CONTACT;
	}


	if ((FilterFlags0&EPDF_ContactNotify) || (FilterFlags1&EPDF_ContactNotify))
	{
		pairFlags |= (PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_CONTACT_POINTS);
	}


	if ((FilterFlags0&EPDF_ModifyContacts) || (FilterFlags1&EPDF_ModifyContacts))
	{
		pairFlags |= (PxPairFlag::eMODIFY_CONTACTS);
	}

	return PxFilterFlags();
}

APortalWorldSettings::APortalWorldSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GSimulationFilterShader = PhysXSimFilterShaderPortal;
}
