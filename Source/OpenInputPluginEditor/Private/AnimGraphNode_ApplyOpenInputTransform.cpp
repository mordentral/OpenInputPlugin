// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "AnimGraphNode_ApplyOpenInputTransform.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_ModifyBSHand

UAnimGraphNode_ApplyOpenInputTransform::UAnimGraphNode_ApplyOpenInputTransform(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

//Title Color!
FLinearColor UAnimGraphNode_ApplyOpenInputTransform::GetNodeTitleColor() const
{
	return FLinearColor(12, 12, 0, 1);
}

//Node Category
FString UAnimGraphNode_ApplyOpenInputTransform::GetNodeCategory() const
{
	return FString("OpenVR");
}
FText UAnimGraphNode_ApplyOpenInputTransform::GetControllerDescription() const
{
	return FText::FromString("Apply OpenInput Transform");
}

FText UAnimGraphNode_ApplyOpenInputTransform::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText Result = GetControllerDescription();
	return Result;
}