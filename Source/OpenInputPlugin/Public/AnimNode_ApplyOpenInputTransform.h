// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "OpenInputFunctionLibrary.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
//#include "Skeleton/BodyStateSkeleton.h"
//#include "BodyStateAnimInstance.h"

#include "AnimNode_ApplyOpenInputTransform.generated.h"

UENUM(BlueprintType)
enum class EVROpenVRSkeletonType : uint8
{
	OVR_SkeletonType_UE4Default_Left,
	OVR_SkeletonType_UE4Default_Right,
	OVR_SkeletonType_OpenVRDefault_Left,
	OVR_SkeletonType_OpenVRDefault_Right,
	OVERSkeletonType_Custom
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRSkeletalPair
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EVROpenInputBones OpenVRBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FName BoneToTarget;

	FBoneReference ReferenceToConstruct;

	FBPOpenVRSkeletalPair()
	{
		OpenVRBone = EVROpenInputBones::eBone_Root;
		BoneToTarget = NAME_None;
	}

	FBPOpenVRSkeletalPair(EVROpenInputBones Bone, FString TargetBone)
	{
		OpenVRBone = Bone;
		BoneToTarget = FName(*TargetBone);
		ReferenceToConstruct.BoneName = BoneToTarget;
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPSkeletalMappingData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		TArray<FBPOpenVRSkeletalPair> BonePairs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		bool bMergeRootAndWristBones;

	bool bInitialized;

	void ConstructDefaultMappings(EVROpenVRSkeletonType SkeletonType)
	{

		switch (SkeletonType)
		{

		case EVROpenVRSkeletonType::OVR_SkeletonType_OpenVRDefault_Left:
		case EVROpenVRSkeletonType::OVR_SkeletonType_OpenVRDefault_Right:
		{
			bMergeRootAndWristBones = false;

			FString HandDelimiterS = SkeletonType == EVROpenVRSkeletonType::OVR_SkeletonType_OpenVRDefault_Left ? "l" : "r";
			const TCHAR* HandDelimiter = *HandDelimiterS;
			// Default OpenVR bones mapping

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Root, FString::Printf(TEXT("Root"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Wrist, FString::Printf(TEXT("wrist_%s"), HandDelimiter)));


			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb0, FString::Printf(TEXT("finger_thumb_0_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb1, FString::Printf(TEXT("finger_thumb_1_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb2, FString::Printf(TEXT("finger_thumb_2_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb3, FString::Printf(TEXT("finger_thumb_%s_end"), HandDelimiter)));


			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger0, FString::Printf(TEXT("finger_index_meta_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger1, FString::Printf(TEXT("finger_index_0_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger2, FString::Printf(TEXT("finger_index_1_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger3, FString::Printf(TEXT("finger_index_2_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger4, FString::Printf(TEXT("finger_index_%s_end"), HandDelimiter)));


			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger0, FString::Printf(TEXT("finger_middle_meta_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger1, FString::Printf(TEXT("finger_middle_0_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger2, FString::Printf(TEXT("finger_middle_1_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger3, FString::Printf(TEXT("finger_middle_2_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger4, FString::Printf(TEXT("finger_middle_%s_end"), HandDelimiter)));
			

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger0, FString::Printf(TEXT("finger_ring_meta_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger1, FString::Printf(TEXT("finger_ring_0_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger2, FString::Printf(TEXT("finger_ring_1_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger3, FString::Printf(TEXT("finger_ring_2_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger4, FString::Printf(TEXT("finger_ring_%s_end"), HandDelimiter)));


			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger0, FString::Printf(TEXT("finger_pinky_meta_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger1, FString::Printf(TEXT("finger_pinky_0_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger2, FString::Printf(TEXT("finger_pinky_1_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger3, FString::Printf(TEXT("finger_pinky_2_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger4, FString::Printf(TEXT("finger_pinky_%s_end"), HandDelimiter)));

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Aux_Thumb, FString::Printf(TEXT("finger_thumb_%s_aux"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Aux_IndexFinger, FString::Printf(TEXT("finger_index_%s_aux"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Aux_MiddleFinger, FString::Printf(TEXT("finger_middle_%s_aux"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Aux_RingFinger, FString::Printf(TEXT("finger_ring_%s_aux"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Aux_PinkyFinger, FString::Printf(TEXT("finger_pinky_%s_aux"), HandDelimiter)));
		}break;

		case EVROpenVRSkeletonType::OVR_SkeletonType_UE4Default_Left:
		case EVROpenVRSkeletonType::OVR_SkeletonType_UE4Default_Right:
		default:
		{
			bMergeRootAndWristBones = true;

			FString HandDelimiterS = SkeletonType == EVROpenVRSkeletonType::OVR_SkeletonType_UE4Default_Left ? "l" : "r";
			const TCHAR* HandDelimiter = *HandDelimiterS;

			// Default ue4 skeleton hand to the OpenVR bones, skipping the extra joint and the aux joints

			//BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Root, FString::Printf(TEXT("hand_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Wrist, FString::Printf(TEXT("hand_%s"), HandDelimiter)));

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger1, FString::Printf(TEXT("index_01_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger2, FString::Printf(TEXT("index_02_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_IndexFinger3, FString::Printf(TEXT("index_03_%s"), HandDelimiter)));

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger1, FString::Printf(TEXT("middle_01_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger2, FString::Printf(TEXT("middle_02_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_MiddleFinger3, FString::Printf(TEXT("middle_03_%s"), HandDelimiter)));

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger1, FString::Printf(TEXT("pinky_01_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger2, FString::Printf(TEXT("pinky_02_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_PinkyFinger3, FString::Printf(TEXT("pinky_03_%s"), HandDelimiter)));

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger1, FString::Printf(TEXT("ring_01_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger2, FString::Printf(TEXT("ring_02_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_RingFinger3, FString::Printf(TEXT("ring_03_%s"), HandDelimiter)));

			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb0, FString::Printf(TEXT("thumb_01_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb1, FString::Printf(TEXT("thumb_02_%s"), HandDelimiter)));
			BonePairs.Add(FBPOpenVRSkeletalPair(EVROpenInputBones::eBone_Thumb2, FString::Printf(TEXT("thumb_03_%s"), HandDelimiter)));

		}break;
		}
	}

	FBPSkeletalMappingData()
	{
		bInitialized = false;
		bMergeRootAndWristBones = false;
	}
};

USTRUCT()
struct OPENINPUTPLUGIN_API FAnimNode_ApplyOpenInputTransform : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

public:

	/** All combined settings required for this node to process mapped bones */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BodyState, meta = (PinShownByDefault))
	//FMappedBoneAnimData MappedBoneAnimData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skeletal/*, meta = (PinShownByDefault)*/)
		FBPSkeletalMappingData MappedBonePairs;

	// Generally used when not passing in custom bone mappings, defines the auto mapping style
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skeletal, meta = (PinShownByDefault))
		EVROpenVRSkeletonType SkeletonType;
	
	// Generally used when not passing in custom bone mappings, defines the auto mapping style
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skeletal, meta = (PinShownByDefault))
	FBPOpenVRActionInfo OptionalStoredActionInfo;

	bool bIsOpenInputAnimationInstance;

	// FAnimNode_SkeletalControlBase interface
	//virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;

	// Constructor 
	FAnimNode_ApplyOpenInputTransform();

protected:
	bool WorldIsGame;
	AActor* OwningActor;

private:
};