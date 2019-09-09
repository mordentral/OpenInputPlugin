// Fill out your copyright notice in the Description page of Project Settings.
#include "AnimNode_ApplyOpenInputTransform.h"
//#include "EngineMinimal.h"
//#include "Engine/Engine.h"
//#include "CoreMinimal.h"
#include "OpenInputFunctionLibrary.h"
#include "AnimNode_ApplyOpenInputTransform.h"
#include "AnimationRuntime.h"
#include "OpenInputSkeletalMeshComponent.h"
#include "Runtime/Engine/Public/Animation/AnimInstanceProxy.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
	
FAnimNode_ApplyOpenInputTransform::FAnimNode_ApplyOpenInputTransform()
	: FAnimNode_SkeletalControlBase()
{
	WorldIsGame = false;
	Alpha = 1.f;
	SkeletonType = EVROpenVRSkeletonType::OVR_SkeletonType_UE4Default_Right;
	bIsOpenInputAnimationInstance = false;
	bSkipRootBone = false;
}

void FAnimNode_ApplyOpenInputTransform::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	if (const UOpenInputAnimInstance * animInst = Cast<UOpenInputAnimInstance>(InAnimInstance))
	{
		bIsOpenInputAnimationInstance = true;
	}
}

void FAnimNode_ApplyOpenInputTransform::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	if (!MappedBonePairs.bInitialized)
	{
		USkeleton * AssetSkeleton = RequiredBones.GetSkeletonAsset();

		if (AssetSkeleton)
		{

			// If our bone pairs are empty, then setup our sane defaults
			if(!MappedBonePairs.BonePairs.Num())
				MappedBonePairs.ConstructDefaultMappings(SkeletonType, bSkipRootBone);

			FBPOpenVRSkeletalPair WristPair;
			FBPOpenVRSkeletalPair IndexPair;
			FBPOpenVRSkeletalPair PinkyPair;

			for (FBPOpenVRSkeletalPair& BonePair : MappedBonePairs.BonePairs)
			{
				// Fill in the bone name for the reference
				BonePair.ReferenceToConstruct.BoneName = BonePair.BoneToTarget;

				// Init the reference
				BonePair.ReferenceToConstruct.Initialize(AssetSkeleton);
				BonePair.ReferenceToConstruct.CachedCompactPoseIndex = BonePair.ReferenceToConstruct.GetCompactPoseIndex(RequiredBones);

				if ((BonePair.ReferenceToConstruct.CachedCompactPoseIndex != INDEX_NONE))
				{
					// Get our parent bones index
					BonePair.ParentReference = RequiredBones.GetParentBoneIndex(BonePair.ReferenceToConstruct.CachedCompactPoseIndex);
				}

				if (BonePair.OpenVRBone == EVROpenInputBones::eBone_Wrist)
				{
					WristPair = BonePair;
				}
				else if (BonePair.OpenVRBone == EVROpenInputBones::eBone_IndexFinger1)
				{
					IndexPair = BonePair;
				}
				else if (BonePair.OpenVRBone == EVROpenInputBones::eBone_PinkyFinger1)
				{
					PinkyPair = BonePair;
				}
			}

			MappedBonePairs.bInitialized = true;

			if (WristPair.ReferenceToConstruct.HasValidSetup() && IndexPair.ReferenceToConstruct.HasValidSetup() && PinkyPair.ReferenceToConstruct.HasValidSetup())
			{
				TArray<FTransform> RefBones = AssetSkeleton->GetReferenceSkeleton().GetRefBonePose();
				TArray<FMeshBoneInfo> RefBonesInfo = AssetSkeleton->GetReferenceSkeleton().GetRefBoneInfo();

				FTransform WristPose = GetRefBoneInCS(RefBones, RefBonesInfo, WristPair.ReferenceToConstruct.BoneIndex);
				FTransform MiddleFingerPose = GetRefBoneInCS(RefBones, RefBonesInfo, PinkyPair.ReferenceToConstruct.BoneIndex);

				FVector BoneForwardVector = MiddleFingerPose.GetTranslation() - WristPose.GetTranslation();
				SetVectorToMaxElement(BoneForwardVector);
				BoneForwardVector.Normalize();

				FTransform IndexFingerPose = GetRefBoneInCS(RefBones, RefBonesInfo, IndexPair.ReferenceToConstruct.BoneIndex);
				FTransform PinkyFingerPose = GetRefBoneInCS(RefBones, RefBonesInfo, PinkyPair.ReferenceToConstruct.BoneIndex);
				FVector BoneUpVector = IndexFingerPose.GetTranslation() - PinkyFingerPose.GetTranslation();
				SetVectorToMaxElement(BoneUpVector);
				BoneUpVector.Normalize();

				FVector BoneRightVector = FVector::CrossProduct(BoneUpVector, BoneForwardVector);
				BoneRightVector.Normalize();

				FQuat ForwardAdjustment = FQuat::FindBetweenNormals(FVector::ForwardVector, BoneForwardVector);

				FVector NewRightVector = ForwardAdjustment * FVector::RightVector;
				NewRightVector.Normalize();

				FQuat TwistAdjustment = FQuat::FindBetweenNormals(NewRightVector, BoneRightVector);
				MappedBonePairs.AdjustmentQuat = TwistAdjustment * ForwardAdjustment;
				MappedBonePairs.AdjustmentQuat.Normalize();
				//FRotator adjustmentrot = MappedBonePairs.AdjustmentQuat.Rotator();
			}
		}
	}
}

void FAnimNode_ApplyOpenInputTransform::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	if (!MappedBonePairs.bInitialized)
		return;

	const FBPOpenVRActionSkeletalData *StoredActionInfoPtr = nullptr;
	if (bIsOpenInputAnimationInstance)
	{
		const FOpenInputAnimInstanceProxy* OpenInputAnimInstance = (FOpenInputAnimInstanceProxy*)Output.AnimInstanceProxy;
		if (OpenInputAnimInstance->HandSkeletalActionData.Num())
		{
			for (int i = 0; i <OpenInputAnimInstance->HandSkeletalActionData.Num(); ++i)
			{
				if (OpenInputAnimInstance->HandSkeletalActionData[i].TargetHand == MappedBonePairs.TargetHand)
				{
					StoredActionInfoPtr = &OpenInputAnimInstance->HandSkeletalActionData[i];
					break;
				}
			}
		}
	}
	else if (OptionalStoredActionInfo.SkeletalTransforms.Num())
	{
		StoredActionInfoPtr = &OptionalStoredActionInfo;
	}

	// Currently not blending correctly
	const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	uint8 BoneTransIndex = 0;
	uint8 NumBones = StoredActionInfoPtr ? StoredActionInfoPtr->SkeletalTransforms.Num() : 0;

	if (NumBones < 1)
	{
		for (const FBPOpenVRSkeletalPair& BonePair : MappedBonePairs.BonePairs)
		{
			if (BonePair.ReferenceToConstruct.CachedCompactPoseIndex == INDEX_NONE)
				continue;

			if (!BonePair.ReferenceToConstruct.IsValidToEvaluate(BoneContainer))
			{
				continue;
			}
			
			OutBoneTransforms.Emplace(BonePair.ReferenceToConstruct.CachedCompactPoseIndex, Output.Pose.GetComponentSpaceTransform(BonePair.ReferenceToConstruct.CachedCompactPoseIndex));
		}

		return;
	}

	FTransform trans = FTransform::Identity;
	OutBoneTransforms.Reserve(MappedBonePairs.BonePairs.Num());
	TArray<FBoneTransform> TransBones;
	FTransform AdditionTransform = StoredActionInfoPtr->AdditionTransform;
	if (StoredActionInfoPtr->bMirrorHand)
	{
		AdditionTransform.Mirror(EAxis::X, EAxis::Y);
	}


	FTransform TempTrans = FTransform::Identity;

	TMap<int32, FTransform> ParentTransformArray;
	ParentTransformArray.Reserve(MappedBonePairs.BonePairs.Num()); // Maximum value would be each bone having a unique parent somehow
	FTransform ParentTrans = FTransform::Identity;
	FTransform * ParentTransPtr = nullptr;

	for (const FBPOpenVRSkeletalPair& BonePair : MappedBonePairs.BonePairs)
	{
		BoneTransIndex = (int8)BonePair.OpenVRBone;
		ParentTrans = FTransform::Identity;

		if (BoneTransIndex >= NumBones || BonePair.ReferenceToConstruct.CachedCompactPoseIndex == INDEX_NONE)
			continue;		

		if (!BonePair.ReferenceToConstruct.IsValidToEvaluate(BoneContainer))
		{
			continue;
		}
		
		trans = Output.Pose.GetComponentSpaceTransform(BonePair.ReferenceToConstruct.CachedCompactPoseIndex);

		if (StoredActionInfoPtr->bGetTransformsInParentSpace && BonePair.ParentReference != INDEX_NONE)
		{
			ParentTransPtr = ParentTransformArray.Find(BonePair.ParentReference.GetInt());
			if (ParentTransPtr != nullptr)
			{
				ParentTrans = *ParentTransPtr;
				ParentTrans.SetScale3D(FVector(1.f));
			}
			else
			{
				ParentTrans = Output.Pose.GetComponentSpaceTransform(BonePair.ParentReference);
				ParentTrans.SetScale3D(FVector(1.f));
				ParentTransformArray.Add(BonePair.ParentReference.GetInt(), ParentTrans);
			}
		}

		if (MappedBonePairs.bMergeMissingBonesUE4 && BoneTransIndex <= 1)
		{
			if (BoneTransIndex == 1)
				TempTrans = (StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex - 1] * StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex]) * ParentTrans;
			else if (BoneTransIndex == 0)
				TempTrans = (StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex] * StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex + 1]) * ParentTrans;

			if (StoredActionInfoPtr->bAllowDeformingMesh)
				trans.SetTranslation(TempTrans.GetTranslation());

			trans.SetRotation(TempTrans.GetRotation());

			if (StoredActionInfoPtr->bGetTransformsInParentSpace)
				ParentTransformArray.Add(BonePair.ReferenceToConstruct.CachedCompactPoseIndex.GetInt(), trans);
		}
		else
		{
			if (StoredActionInfoPtr->bGetTransformsInParentSpace && MappedBonePairs.bMergeMissingBonesUE4)
			{
				EVROpenInputBones CurrentBone = (EVROpenInputBones)BoneTransIndex;
				if (CurrentBone == EVROpenInputBones::eBone_MiddleFinger1 ||
					CurrentBone == EVROpenInputBones::eBone_IndexFinger1 ||
					CurrentBone == EVROpenInputBones::eBone_PinkyFinger1 ||
					CurrentBone == EVROpenInputBones::eBone_RingFinger1
					)
				{
					TempTrans = (StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex] * StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex - 1]);// *ParentTrans;
				}
				else
				{
					TempTrans = (StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex]);// *ParentTrans;
				}
			}
			else
				TempTrans = (StoredActionInfoPtr->SkeletalTransforms[BoneTransIndex]);// *ParentTrans;
			
			if (StoredActionInfoPtr->bMirrorHand)
			{
				FMatrix M = TempTrans.ToMatrixWithScale();
				M.Mirror(EAxis::Z, EAxis::X);
				M.Mirror(EAxis::X, EAxis::Z);
				TempTrans.SetFromMatrix(M);
			}

			TempTrans = TempTrans * ParentTrans;

			if (StoredActionInfoPtr->bAllowDeformingMesh)
				trans.SetTranslation(TempTrans.GetTranslation());

			trans.SetRotation(TempTrans.GetRotation());

			if (StoredActionInfoPtr->bGetTransformsInParentSpace)
				ParentTransformArray.Add(BonePair.ReferenceToConstruct.CachedCompactPoseIndex.GetInt(), trans);

			if (StoredActionInfoPtr->bAllowDeformingMesh)
			{
				trans = (AdditionTransform)* trans;

				if (!StoredActionInfoPtr->bGetTransformsInParentSpace)
				{
					trans.SetTranslation(MappedBonePairs.AdjustmentQuat.RotateVector(trans.GetTranslation()));
					trans.SetRotation((MappedBonePairs.AdjustmentQuat * trans.GetRotation()).GetNormalized());
				}
			}
			else
			{
				trans.ConcatenateRotation(AdditionTransform.GetRotation());
				if (!StoredActionInfoPtr->bGetTransformsInParentSpace)
					trans.SetRotation((MappedBonePairs.AdjustmentQuat * trans.GetRotation()).GetNormalized());
			}
		}
		
		TransBones.Add(FBoneTransform(BonePair.ReferenceToConstruct.CachedCompactPoseIndex, trans));

		// Need to do it per bone so future bones are correct
		if ((!StoredActionInfoPtr->bAllowDeformingMesh || StoredActionInfoPtr->bGetTransformsInParentSpace) && TransBones.Num())
		{
			Output.Pose.LocalBlendCSBoneTransforms(TransBones, BlendWeight);
			TransBones.Reset();
		}
	}

	// Fine doing it at the end, all bones are self authoritative, this is a perf savings
	if (StoredActionInfoPtr->bAllowDeformingMesh && !StoredActionInfoPtr->bGetTransformsInParentSpace && TransBones.Num())
	{
		Output.Pose.LocalBlendCSBoneTransforms(TransBones, BlendWeight);
		TransBones.Reset();
	}
}

bool FAnimNode_ApplyOpenInputTransform::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return(/*MappedBonePairs.bInitialized && */MappedBonePairs.BonePairs.Num() > 0);
}
