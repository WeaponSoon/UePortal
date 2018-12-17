// Fill out your copyright notice in the Description page of Project Settings.

#include "PPortalNode.h"
#include "PPortalTree.h"

void UPPortalNode::SetRenderTexture(const UTextureRenderTarget2D* renderTarget2D)
{
	renderTexture = const_cast<UTextureRenderTarget2D*>(renderTarget2D);
}

UTextureRenderTarget2D* UPPortalNode::GetRenderTexture()
{
	return renderTexture;
}

void UPPortalNode::AddChild(const UPPortalNode* child)
{
	childrenNode.Add(const_cast<UPPortalNode*>(child));
}

void UPPortalNode::RecycleChildren()
{
	for (int32 i = 0; i < childrenNode.Num(); ++i)
	{
		childrenNode[i]->RecycleChildren();
		childrenNode[i]->childrenNode.Reset();
		childrenNode[i]->portalDoor = nullptr;
		motherTree->RecyclePortalNode(childrenNode[i]);
	}

	childrenNode.Reset();
}

void UPPortalNode::SetMotherTree(const UPPortalTree* mother)
{
	motherTree = const_cast<UPPortalTree*>(mother);
}