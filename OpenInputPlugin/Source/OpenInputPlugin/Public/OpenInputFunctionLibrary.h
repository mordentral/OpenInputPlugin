// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HAL/FileManager.h"
#include "DrawDebugHelpers.h"
#include "Misc/Paths.h"
#include "Engine/EngineTypes.h"

//#include "Engine/Texture.h"
//#include "Engine/EngineTypes.h"
//#include "VRExpansionFunctionLibrary.h"

#define STEAMVR_SUPPORTED_PLATFORM (PLATFORM_MAC || (PLATFORM_LINUX && PLATFORM_CPU_X86_FAMILY && PLATFORM_64BITS) || (PLATFORM_WINDOWS && WINVER > 0x0502))

#if STEAMVR_SUPPORTED_PLATFORM
#include "openvr.h"
#endif // STEAMVR_SUPPORTED_PLATFORM

#include "OpenInputFunctionLibrary.generated.h"

namespace OpenInputFunctionLibraryStatics
{
	const FString LeftHand_SkeletalActionName("/actions/main/in/skeletonleft");
	const FString RightHand_SkeletalActionName("/actions/main/in/skeletonright");
}

// Holds the action handle in a BP friendly form
USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRActionHandle
{
	GENERATED_BODY()
public:

	vr::VRActionHandle_t ActionHandle;
};

//const BoneIndex_t INVALID_BONEINDEX = -1;
UENUM(BlueprintType)
enum class EVROpenInputBones : uint8
{
	eBone_Root = 0,
	eBone_Wrist,
	eBone_Thumb0,
	eBone_Thumb1,
	eBone_Thumb2,
	eBone_Thumb3,
	eBone_IndexFinger0,
	eBone_IndexFinger1,
	eBone_IndexFinger2,
	eBone_IndexFinger3,
	eBone_IndexFinger4,
	eBone_MiddleFinger0,
	eBone_MiddleFinger1,
	eBone_MiddleFinger2,
	eBone_MiddleFinger3,
	eBone_MiddleFinger4,
	eBone_RingFinger0,
	eBone_RingFinger1,
	eBone_RingFinger2,
	eBone_RingFinger3,
	eBone_RingFinger4,
	eBone_PinkyFinger0,
	eBone_PinkyFinger1,
	eBone_PinkyFinger2,
	eBone_PinkyFinger3,
	eBone_PinkyFinger4,
	eBone_Aux_Thumb,
	eBone_Aux_IndexFinger,
	eBone_Aux_MiddleFinger,
	eBone_Aux_RingFinger,
	eBone_Aux_PinkyFinger,
	eBone_Count
};

UENUM(BlueprintType)
enum class EVROpenInputReferencePose : uint8
{
	VRSkeletalReferencePose_BindPose = 0,
	VRSkeletalReferencePose_OpenHand,
	VRSkeletalReferencePose_Fist,
	VRSkeletalReferencePose_GripLimit
};

UENUM(BlueprintType)
enum class EVROpenInputSkeletalTrackingLevel : uint8
{
	// body part location can’t be directly determined by the device. Any skeletal pose provided by 
	// the device is estimated by assuming the position required to active buttons, triggers, joysticks, 
	// or other input sensors. 
	// E.g. Vive Controller, Gamepad
	VRSkeletalTracking_Estimated = 0,

	// body part location can be measured directly but with fewer degrees of freedom than the actual body 
	// part. Certain body part positions may be unmeasured by the device and estimated from other input data. 
	// E.g. Knuckles, gloves that only measure finger curl
	VRSkeletalTracking_Partial,

	// Body part location can be measured directly throughout the entire range of motion of the body part. 
	// E.g. Mocap suit for the full body, gloves that measure rotation of each finger segment
	VRSkeletalTracking_Full,

	// Max value, I also use it for uninitialized
	VRSkeletalTrackingLevel_Max
};

UENUM(BlueprintType)
enum class EVROpenInputFingerIndexType : uint8
{
	VRFinger_Thumb = 0,
	VRFinger_Index,
	VRFinger_Middle,
	VRFinger_Ring,
	VRFinger_Pinky,
	VRFingerSplay_Thumb_Index,
	VRFingerSplay_Index_Middle,
	VRFingerSplay_Middle_Ring,
	VRFingerSplay_Ring_Pinky
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRGesturePoseData
{
	GENERATED_BODY()
public:

	// Not a static array because it is BP accessible
	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = Default)
		TArray<float> PoseFingerCurls;

	// Not a static array because it is BP accessible
	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = Default)
		TArray<float> PoseFingerSplays;
};

UENUM(BlueprintType)
enum class EVRActionHand : uint8
{
	EActionHand_Left = 0,
	EActionHand_Right
};


USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRActionSkeletalData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		EVRActionHand TargetHand;

	//UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
	//	bool bGetTransformsInParentSpace;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bAllowDeformingMesh;

	// If true then the hand mesh will be mirrored, generally used for full body rigs to fix the left hands axis
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bMirrorHand;

	// If true then the bones will be mirrored from left/right, to allow you to swap a hand mesh
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bMirrorLeftRight;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		TArray<FTransform> SkeletalTransforms;

	// The rotation required to rotate the finger bones back to X+
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FTransform AdditionTransform;

	FBPOpenVRActionSkeletalData()
	{
		//bGetTransformsInParentSpace = false;
		AdditionTransform = FTransform(FRotator(0.f, 90.f, 90.f), FVector::ZeroVector, FVector(1.f));
		bAllowDeformingMesh = true;
		bMirrorHand = false;
		bMirrorLeftRight = false;
		TargetHand = EVRActionHand::EActionHand_Right;
	}
};

UENUM()
enum class EVRSkeletalReplicationType : uint8
{
	/*Replicate the curl values only, you can blend between open and closed with it then*/
	Rep_CurlOnly = 0,
	/*Replicate curl AND splay, this is only useful for a fully tracked glove or full skeleton like from image processing*/
	Rep_CurlAndSplay,
	/*Replicate the given transforms, this is more costly than any other method, I suggest NOT having bAllowDeformingMesh enabled with this active*/
	Rep_HardTransforms,
	/*Replicates using the built in SteamVR compression technique, this cannot be used cross platform but is the best choice when you know everyone will be launch with steamVR*/
	Rep_SteamVRCompressedTransforms
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRActionInfo
{
	GENERATED_BODY()
public:

	// If true with get the controller hand pose, otherwise will get the plain hand pose
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bGetSkeletalTransforms_WithController;

	// Optional action name, if you are not using the default manifest generation then you can
	// set a custom name here, otherwise it will automatically be filled out.
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FString ActionName;

	// This is where you set the hand type / settings for the skeletal data
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default, meta = (AllowPrivateAccess = "true"))
		FBPOpenVRActionSkeletalData SkeletalData;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		TArray<int32> BoneParentIndexes;

	// Not a static array because it is BP accessible
	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		FBPOpenVRGesturePoseData PoseFingerData;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		TArray<FTransform> OldSkeletalTransforms;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		bool bHasValidData;

	// The level of tracking that the OpenInputdevice has (only valid value if this bHasValidData)
	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		EVROpenInputSkeletalTrackingLevel SkeletalTrackingLevel;

	FBPOpenVRActionHandle ActionHandleContainer;
	FName LastHandGesture;
	int32 LastHandGestureIndex;

	UPROPERTY()
	TArray<uint8> CompressedTransforms;
	UPROPERTY(NotReplicated)
	uint32 CompressedSize;
	UPROPERTY()
	int8 BoneCount;

	FBPOpenVRActionInfo()
	{
		ActionHandleContainer.ActionHandle = vr::k_ulInvalidActionHandle;
		bHasValidData = false;
		CompressedSize = 0;
		BoneCount = 0;
		SkeletalTrackingLevel = EVROpenInputSkeletalTrackingLevel::VRSkeletalTrackingLevel_Max;
		bGetSkeletalTransforms_WithController = false;
		LastHandGestureIndex = INDEX_NONE;
		LastHandGesture = NAME_None;
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPSkeletalRepContainer
{
	GENERATED_BODY()
public:

	UPROPERTY(Transient, NotReplicated)
		EVRActionHand TargetHand;

	UPROPERTY(Transient, NotReplicated)
		EVRSkeletalReplicationType ReplicationType;

	UPROPERTY(Transient, NotReplicated)
		FBPOpenVRGesturePoseData PoseFingerData;

	UPROPERTY(Transient, NotReplicated)
		bool bAllowDeformingMesh;

	UPROPERTY(Transient, NotReplicated)
		TArray<FTransform> SkeletalTransforms;

	UPROPERTY(Transient, NotReplicated)
		uint8 BoneCount;

	UPROPERTY(Transient, NotReplicated)
		TArray<uint8> CompressedTransforms;


	FBPSkeletalRepContainer()
	{
		TargetHand = EVRActionHand::EActionHand_Left;
		ReplicationType = EVRSkeletalReplicationType::Rep_CurlAndSplay;
		bAllowDeformingMesh = false;
		BoneCount = 0;
	}

	bool bHasValidData()
	{
		return CompressedTransforms.Num() > 0 || SkeletalTransforms.Num() > 0 || PoseFingerData.PoseFingerCurls.Num() > 0;
	}

	void CopyForReplication(FBPOpenVRActionInfo& Other, EVRSkeletalReplicationType RepType)
	{
		TargetHand = Other.SkeletalData.TargetHand;
		ReplicationType = RepType;

		if (!Other.bHasValidData)
			return;

		switch (ReplicationType)
		{
		case EVRSkeletalReplicationType::Rep_CurlOnly:
		case EVRSkeletalReplicationType::Rep_CurlAndSplay:
		{
			PoseFingerData = Other.PoseFingerData;	
		}break;

		case EVRSkeletalReplicationType::Rep_HardTransforms:
		{
			bAllowDeformingMesh = Other.SkeletalData.bAllowDeformingMesh;

			// Instead of doing this, we likely need to lerp but this is for testing
			//SkeletalTransforms = Other.SkeletalData.SkeletalTransforms;

			if (Other.SkeletalData.SkeletalTransforms.Num() < (uint8)EVROpenInputBones::eBone_Count)
			{
				SkeletalTransforms.Empty();
				return;
			}

			if (SkeletalTransforms.Num() != (uint8)EVROpenInputBones::eBone_Count - 11)
			{
				SkeletalTransforms.Reset((uint8)EVROpenInputBones::eBone_Count - 11); // Minus bones we don't need
				SkeletalTransforms.AddUninitialized((uint8)EVROpenInputBones::eBone_Count - 11);
			}

			// Root is always identity
			//SkeletalTransforms[0] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Root]; // This has no pos right? Need to skip pos on it
			SkeletalTransforms[0] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Wrist];
			SkeletalTransforms[1] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb0];
			SkeletalTransforms[2] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb1];
			SkeletalTransforms[3] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb2];
			SkeletalTransforms[4] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger0];
			SkeletalTransforms[5] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger1];
			SkeletalTransforms[6] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger2];
			SkeletalTransforms[7] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger3];
			SkeletalTransforms[8] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger0];
			SkeletalTransforms[9] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger1];
			SkeletalTransforms[10] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger2];
			SkeletalTransforms[11] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger3];
			SkeletalTransforms[12] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger0];
			SkeletalTransforms[13] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger1];
			SkeletalTransforms[14] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger2];
			SkeletalTransforms[15] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger3];
			SkeletalTransforms[16] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger0];
			SkeletalTransforms[17] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger1];
			SkeletalTransforms[18] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger2];
			SkeletalTransforms[19] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger3];

		}break;

		case EVRSkeletalReplicationType::Rep_SteamVRCompressedTransforms:
		{
			BoneCount = Other.BoneCount;
			CompressedTransforms = Other.CompressedTransforms;
		}break;
		}
	}

	static void CopyReplicatedTo(const FBPSkeletalRepContainer & Container, FBPOpenVRActionInfo& Other)
	{
		switch (Container.ReplicationType)
		{
		case EVRSkeletalReplicationType::Rep_CurlOnly:
		case EVRSkeletalReplicationType::Rep_CurlAndSplay:
		{
			Other.PoseFingerData = Container.PoseFingerData;
			Other.bHasValidData = true;
		}break;

		case EVRSkeletalReplicationType::Rep_HardTransforms:
		{
			if (Container.SkeletalTransforms.Num() < ((uint8)EVROpenInputBones::eBone_Count - 11))
			{
				Other.SkeletalData.SkeletalTransforms.Empty();
				Other.bHasValidData = false;
				return;
			}

			Other.SkeletalData.bAllowDeformingMesh = Container.bAllowDeformingMesh;

			// Instead of doing this, we likely need to lerp but this is for testing
			//Other.SkeletalData.SkeletalTransforms = Container.SkeletalTransforms;
			
			if (Other.SkeletalData.SkeletalTransforms.Num() != (uint8)EVROpenInputBones::eBone_Count)
			{
				Other.SkeletalData.SkeletalTransforms.Reset((uint8)EVROpenInputBones::eBone_Count);
				Other.SkeletalData.SkeletalTransforms.AddUninitialized((uint8)EVROpenInputBones::eBone_Count);
			}

			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Root] = FTransform::Identity; // Always identity
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Wrist] = Container.SkeletalTransforms[0];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb0] = Container.SkeletalTransforms[1];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb1] = Container.SkeletalTransforms[2];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb2] = Container.SkeletalTransforms[3];

			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger0] = Container.SkeletalTransforms[4];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger1] = Container.SkeletalTransforms[5];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger2] = Container.SkeletalTransforms[6];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger3] = Container.SkeletalTransforms[7];

			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger0] = Container.SkeletalTransforms[8];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger1] = Container.SkeletalTransforms[9];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger2] = Container.SkeletalTransforms[10];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger3] = Container.SkeletalTransforms[11];

			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger0] = Container.SkeletalTransforms[12];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger1] = Container.SkeletalTransforms[13];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger2] = Container.SkeletalTransforms[14];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger3] = Container.SkeletalTransforms[15];

			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger0] = Container.SkeletalTransforms[16];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger1] = Container.SkeletalTransforms[17];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger2] = Container.SkeletalTransforms[18];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger3] = Container.SkeletalTransforms[19];


			// These are "tip" bones and serve no animation purpose, we need to project them from the last bone forward by a set amount.
			// For now I am just setting to the last bone until I finalize things.
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb3] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb2];// Need to project this from last joint
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger4] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger3];// Need to project this from last joint
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger4] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger3];// Need to project this from last joint
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger4] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger3];// Need to project this from last joint
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger4] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger3];// Need to project this from last joint

			// These are copied from the 3rd joints as they use the same transform but a different root
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Aux_Thumb] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Thumb2];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Aux_IndexFinger] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_IndexFinger3];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Aux_MiddleFinger] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_MiddleFinger3];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Aux_RingFinger] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_RingFinger3];
			Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_Aux_PinkyFinger] = Other.SkeletalData.SkeletalTransforms[(uint8)EVROpenInputBones::eBone_PinkyFinger3];
			Other.bHasValidData = true;
		}break;

		case EVRSkeletalReplicationType::Rep_SteamVRCompressedTransforms:
		{
			Other.BoneCount = Container.BoneCount;
			Other.CompressedTransforms = Container.CompressedTransforms;
			// This is handled in the decompression
			//Other.bHasValidData = true;
		}break;
		}
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		Ar.SerializeBits(&TargetHand, 1);
		Ar.SerializeBits(&ReplicationType, 2);

		switch (ReplicationType)
		{
		case EVRSkeletalReplicationType::Rep_CurlOnly:
		case EVRSkeletalReplicationType::Rep_CurlAndSplay:
		{

			bool bHasCurlData = PoseFingerData.PoseFingerCurls.Num() > 0;
			Ar.SerializeBits(&bHasCurlData, 1);

			if (bHasCurlData)
			{
				int NumFingers = PoseFingerData.PoseFingerCurls.Num();
				Ar.SerializeBits(&NumFingers, 8);

				if (PoseFingerData.PoseFingerCurls.Num() < NumFingers)
				{
					PoseFingerData.PoseFingerCurls.Reset(NumFingers);
					PoseFingerData.PoseFingerCurls.AddUninitialized(NumFingers);
				}

				for (int i = 0; i < NumFingers; i++)
				{
					bOutSuccess &= WriteFixedCompressedFloat<1, 16>(PoseFingerData.PoseFingerCurls[i], Ar);
				}

				if (ReplicationType == EVRSkeletalReplicationType::Rep_CurlAndSplay)
				{
					int NumSplays = PoseFingerData.PoseFingerSplays.Num();
					Ar.SerializeBits(&NumSplays, 8);

					if (PoseFingerData.PoseFingerSplays.Num() < NumSplays)
					{
						PoseFingerData.PoseFingerSplays.Reset(NumSplays);
						PoseFingerData.PoseFingerSplays.AddUninitialized(NumSplays);
					}

					for (int i = 0; i < NumSplays; i++)
					{
						bOutSuccess &= WriteFixedCompressedFloat<1, 16>(PoseFingerData.PoseFingerSplays[i], Ar);
					}
				}
			}

			//PoseFingerData.NetSerialize(Ar, Map, bOutSuccess);
		}break;

		case EVRSkeletalReplicationType::Rep_HardTransforms:
		{
			//Ar.SerializeBits(SkeletalTrackingLevel, 2);
			Ar.SerializeBits(&bAllowDeformingMesh, 1);

			uint8 TransformCount = SkeletalTransforms.Num();

			Ar << TransformCount;

			if (Ar.IsLoading())
			{
				SkeletalTransforms.Reset(TransformCount);
			}

			FVector Position = FVector::ZeroVector;
			FRotator Rot = FRotator::ZeroRotator;

			for (int i = 0; i < TransformCount; i++)
			{
				if (Ar.IsSaving())
				{
					if (bAllowDeformingMesh)
						Position = SkeletalTransforms[i].GetLocation();

					Rot = SkeletalTransforms[i].Rotator();
				}

				if (bAllowDeformingMesh)
					bOutSuccess &= SerializePackedVector<10, 11>(Position, Ar);

				Rot.SerializeCompressed(Ar); // Short? 10 bit?

				if (Ar.IsLoading())
				{
					if (bAllowDeformingMesh)
						SkeletalTransforms.Add(FTransform(Rot, Position));
					else
						SkeletalTransforms.Add(FTransform(Rot));
				}
			}
		}break;

		case EVRSkeletalReplicationType::Rep_SteamVRCompressedTransforms:
		{
			Ar << BoneCount;
			Ar << CompressedTransforms;
		}break;
		default:break;
		}

		return bOutSuccess;
	}
};

template<>
struct TStructOpsTypeTraits< FBPSkeletalRepContainer > : public TStructOpsTypeTraitsBase2<FBPSkeletalRepContainer>
{
	enum
	{
		WithNetSerializer = true
	};
};


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class OPENINPUTPLUGIN_API UOpenInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UOpenInputFunctionLibrary(const FObjectInitializer& ObjectInitializer);

	~UOpenInputFunctionLibrary();
public:

	/** Converts a FBPOpenInputActioInfo into a FBPSkeletalRepContainer */
	UFUNCTION(BlueprintCallable, Category = "OpenInputReplication")
	static void FillRepContainerFromActionInfo(UPARAM(ref) FBPOpenVRActionInfo& ActionInfo, UPARAM(ref) FBPSkeletalRepContainer & TargetRepContainer, EVRSkeletalReplicationType ReplicationType)
	{
		TargetRepContainer.CopyForReplication(ActionInfo, ReplicationType);
	}

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext), Category = "OpenInputReplication")
	static void FillActionInfoFromRepContainer(UObject* WorldContextObject, UPARAM(ref) FBPOpenVRActionInfo& ActionInfo, UPARAM(ref) FBPSkeletalRepContainer& TargetRepContainer)
	{
		TargetRepContainer.CopyReplicatedTo(TargetRepContainer, ActionInfo);

		if (ActionInfo.CompressedTransforms.Num() > 0)
		{
			UOpenInputFunctionLibrary::DecompressSkeletalData(ActionInfo, WorldContextObject->GetWorld());
			ActionInfo.CompressedTransforms.Reset();
		}
	}


	UFUNCTION(BlueprintPure)
	static FTransform GetOpenVRBoneTransform(EVROpenInputBones BoneToGet, FBPOpenVRActionInfo HandSkeletalAction)
	{
		uint8 index = (uint8)BoneToGet;
		if (HandSkeletalAction.SkeletalData.SkeletalTransforms.Num() - 1 >= index)
			return HandSkeletalAction.SkeletalData.SkeletalTransforms[index];

		return FTransform::Identity;
	}

#if STEAMVR_SUPPORTED_PLATFORM

	FORCEINLINE static FTransform CONVERT_STEAMTRANS_TO_FTRANS(const vr::VRBoneTransform_t InTrans, float WorldToMeters)
	{
		return FTransform(
			FQuat(-InTrans.orientation.z, InTrans.orientation.x, InTrans.orientation.y, -InTrans.orientation.w),
			FVector(-InTrans.position.v[2], InTrans.position.v[0], InTrans.position.v[1]) * WorldToMeters
		);
	}


	static void MIRROR_OPENINPUT_BONES(TArray<vr::VRBoneTransform_t> &BoneTransforms)
	{

		/*
		Function code referenced

		Copyright 2019 Valve Corporation under https://opensource.org/licenses/BSD-3-Clause
		This code includes modifications by Joshua Statzer (MordenTral)

		Redistribution and use in source and binary forms, with or without modification,
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, this
		   list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice,
		   this list of conditions and the following disclaimer in the documentation
		   and/or other materials provided with the distribution.

		3. Neither the name of the copyright holder nor the names of its contributors
		   may be used to endorse or promote products derived from this software
		   without specific prior written permission.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
		ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
		WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
		INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
		BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
		OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
		WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
		ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
		POSSIBILITY OF SUCH DAMAGE.
		*/

		static const uint8 PartialMirror[] = {
			(uint8)EVROpenInputBones::eBone_Wrist,
			(uint8)EVROpenInputBones::eBone_Aux_Thumb,
			(uint8)EVROpenInputBones::eBone_Aux_IndexFinger,
			(uint8)EVROpenInputBones::eBone_Aux_MiddleFinger,
			(uint8)EVROpenInputBones::eBone_Aux_RingFinger,
			(uint8)EVROpenInputBones::eBone_Aux_PinkyFinger,
		};

		static const uint8 FullMirror[] = {
			(uint8)EVROpenInputBones::eBone_Thumb0,
			(uint8)EVROpenInputBones::eBone_IndexFinger0,
			(uint8)EVROpenInputBones::eBone_MiddleFinger0,
			(uint8)EVROpenInputBones::eBone_RingFinger0,
			(uint8)EVROpenInputBones::eBone_PinkyFinger0
		};

		static const uint8 TranslationOnly[] = {
			(uint8)EVROpenInputBones::eBone_Thumb1,
			(uint8)EVROpenInputBones::eBone_Thumb2,
			(uint8)EVROpenInputBones::eBone_Thumb3,
			(uint8)EVROpenInputBones::eBone_IndexFinger1,
			(uint8)EVROpenInputBones::eBone_IndexFinger2,
			(uint8)EVROpenInputBones::eBone_IndexFinger3,
			(uint8)EVROpenInputBones::eBone_IndexFinger4,
			(uint8)EVROpenInputBones::eBone_MiddleFinger1,
			(uint8)EVROpenInputBones::eBone_MiddleFinger2,
			(uint8)EVROpenInputBones::eBone_MiddleFinger3,
			(uint8)EVROpenInputBones::eBone_MiddleFinger4,
			(uint8)EVROpenInputBones::eBone_RingFinger1,
			(uint8)EVROpenInputBones::eBone_RingFinger2,
			(uint8)EVROpenInputBones::eBone_RingFinger3,
			(uint8)EVROpenInputBones::eBone_RingFinger4,
			(uint8)EVROpenInputBones::eBone_PinkyFinger1,
			(uint8)EVROpenInputBones::eBone_PinkyFinger2,
			(uint8)EVROpenInputBones::eBone_PinkyFinger3,
			(uint8)EVROpenInputBones::eBone_PinkyFinger4
		};

		const int32 TranslationOnlyBoneCount = sizeof(TranslationOnly) / sizeof(uint8);
		for (int32 i = 0; i < TranslationOnlyBoneCount && i < BoneTransforms.Num(); ++i)
		{
			const int32 BoneIndex = TranslationOnly[i];
			vr::HmdVector4_t& Position = BoneTransforms[BoneIndex].position;
			Position.v[0] *= -1.f;
			Position.v[1] *= -1.f;
			Position.v[2] *= -1.f;
		}

		const int32 FullMirrorCount = sizeof(FullMirror) / sizeof(uint8);
		for (int32 i = 0; i < FullMirrorCount && i < BoneTransforms.Num(); ++i)
		{
			const int32 BoneIndex = FullMirror[i];

			vr::VRBoneTransform_t& BoneTransform = BoneTransforms[BoneIndex];
			BoneTransform.position.v[0] *= -1.f;
			vr::HmdQuaternionf_t OriginalRotation = BoneTransform.orientation;
			BoneTransform.orientation.w = OriginalRotation.x;
			BoneTransform.orientation.x = -OriginalRotation.w;
			BoneTransform.orientation.y = OriginalRotation.z;
			BoneTransform.orientation.z = -OriginalRotation.y;
		}

		const int32 PartialMirrorCount = sizeof(PartialMirror) / sizeof(uint8);
		for (int32 i = 0; i < PartialMirrorCount && i < BoneTransforms.Num(); ++i)
		{
			const int32 BoneIndex = PartialMirror[i];
			vr::VRBoneTransform_t& BoneTransform = BoneTransforms[BoneIndex];
			BoneTransform.position.v[0] *= -1.f;
			BoneTransform.orientation.y *= -1.f;
			BoneTransform.orientation.z *= -1.f;
		}
	}

#endif

	// Decompresses compressed bone data from OpenInput
	static bool DecompressSkeletalData(FBPOpenVRActionInfo & Action, UWorld * WorldToUseForScale)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		return false;
#else
		if (Action.CompressedTransforms.Num() < 1 || !WorldToUseForScale)
			return false;

		vr::IVRInput * VRInput =  vr::VRInput();

		Action.bHasValidData = false;

		if (!VRInput)
			return false;

		TArray<vr::VRBoneTransform_t> BoneTransforms;
		BoneTransforms.AddZeroed(Action.BoneCount);

		Action.CompressedSize = Action.CompressedTransforms.Num();

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;
		vr::EVRSkeletalTransformSpace TransSpace = /*Action.SkeletalData.bGetTransformsInParentSpace ?*/ vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent;// : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model;
		InputError = VRInput->DecompressSkeletalBoneData(Action.CompressedTransforms.GetData(), Action.CompressedSize, TransSpace, BoneTransforms.GetData(), Action.BoneCount);

		if (InputError != vr::EVRInputError::VRInputError_None)
			return false;

		if (Action.SkeletalData.SkeletalTransforms.Num() > 0)
		{
			Action.OldSkeletalTransforms = Action.SkeletalData.SkeletalTransforms;
		}

		if (Action.SkeletalData.SkeletalTransforms.Num() != Action.BoneCount)
		{
			Action.SkeletalData.SkeletalTransforms.Reset(Action.BoneCount);
			Action.SkeletalData.SkeletalTransforms.AddUninitialized(Action.BoneCount);
		}

		float WorldToMeters = ((WorldToUseForScale != nullptr) ? WorldToMeters = WorldToUseForScale->GetWorldSettings()->WorldToMeters : 100.f);

		if (Action.SkeletalData.bMirrorLeftRight)
			MIRROR_OPENINPUT_BONES(BoneTransforms);

		for (int i = 0; i < BoneTransforms.Num(); ++i)
			Action.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(BoneTransforms[i], WorldToMeters);

		Action.bHasValidData = true;
		return true;
#endif
	}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool GetActionPose(UPARAM(ref)FBPOpenVRActionInfo & Action, class UObject* WorldContextObject, bool bGetCompressedData = false, bool bGetGestureValues = true)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		Action.bHasValidData = false;
		return false;
#else
		// An exported valid hand pose for testing, don't mine me
		/*Action.bHasValidData = true;
		Action.BoneCount = 31;
		Action.SkeletalData.SkeletalTransforms.Reset(Action.BoneCount);

		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-6.12323426e-17, -6.12323426e-17, -3.74939939e-33, -1.00000000), FVector(0.000000, 0.000000, 0.000000)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.078608, -0.920279, 0.379296, 0.055147), FVector(0.034038, 0.036503, 0.164722)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.032980, 0.642452, 0.177896, -0.744661), FVector(0.015893, 0.037202, 0.129626)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.062613, -0.641222, -0.081999, 0.760388), FVector(0.011399, 0.049619, 0.091439)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.202745, -0.594267, -0.249441, 0.737239), FVector(0.006059, 0.056285, 0.060064)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.202745, -0.594267, -0.249441, 0.737239), FVector(0.000903, 0.074831, 0.036452)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.602551, 0.647012, 0.373282, -0.281014), FVector(0.029019, 0.044768, 0.135503)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.669277, -0.604607, -0.318717, 0.291441), FVector(0.042551, 0.002262, 0.075993)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.626055, -0.648056, -0.305172, 0.308137), FVector(0.039663, -0.025130, 0.041770)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.623527, -0.663809, -0.293734, 0.290331), FVector(0.040416, -0.043018, 0.019345)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.623527, -0.663809, -0.293734, 0.290331), FVector(0.041644, -0.058017, 0.002189)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.569604, 0.714872, 0.237212, -0.328998), FVector(0.034164, 0.030176, 0.147938)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.681727, -0.647574, -0.284311, 0.187252), FVector(0.043159, -0.017431, 0.096086)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.711895, -0.608206, -0.269531, 0.225043), FVector(0.043175, -0.050903, 0.068921)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.678062, -0.659285, -0.265683, 0.187047), FVector(0.039354, -0.075674, 0.047048)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.703971, -0.631547, -0.257964, 0.197557), FVector(0.039625, -0.096250, 0.031333)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.604345, 0.717512, 0.205713, -0.278612), FVector(0.035078, 0.020420, 0.157607)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.687871, -0.683120, -0.164126, 0.182327), FVector(0.040958, -0.029928, 0.115490)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.708515, -0.661749, -0.160099, 0.185643), FVector(0.040436, -0.065739, 0.096164)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.736793, -0.634757, -0.143936, 0.183037), FVector(0.038340, -0.090987, 0.082579)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.736793, -0.634757, -0.143936, 0.183037), FVector(0.034914, -0.110785, 0.072609)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.649283, 0.709908, 0.228285, -0.149493), FVector(0.034509, 0.012210, 0.167464)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.729502, -0.662042, -0.169427, 0.028676), FVector(0.038718, -0.041785, 0.135390)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.796114, -0.580514, -0.164037, 0.047926), FVector(0.036724, -0.070681, 0.126773)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.759130, -0.638501, -0.126582, 0.003951), FVector(0.031774, -0.087206, 0.121011)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.759130, -0.638501, -0.126582, 0.003951), FVector(0.029025, -0.104654, 0.117457)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(0.202745, -0.594267, -0.249441, 0.737238), FVector(0.006059, 0.056285, 0.060064)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.623527, -0.663809, -0.293734, 0.290331), FVector(0.040416, -0.043018, 0.019345)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.678062, -0.659285, -0.265683, 0.187047), FVector(0.039354, -0.075674, 0.047048)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.736793, -0.634757, -0.143936, 0.183037), FVector(0.038340, -0.090987, 0.082579)));
		Action.SkeletalData.SkeletalTransforms.Add(FTransform(FQuat(-0.759130, -0.638501, -0.126581, 0.003950), FVector(0.031774, -0.087205, 0.121011)));

		for (int i = 0; i < Action.SkeletalData.SkeletalTransforms.Num(); i++)
		{

			vr::VRBoneTransform_t trans;
			FQuat rot = Action.SkeletalData.SkeletalTransforms[i].GetRotation();
			FVector pos = Action.SkeletalData.SkeletalTransforms[i].GetTranslation();

			trans.orientation.w = rot.W;
			trans.orientation.x = rot.X;
			trans.orientation.y = rot.Y;
			trans.orientation.z = rot.Z;

			trans.position.v[0] = pos.X;
			trans.position.v[1] = pos.Y;
			trans.position.v[2] = pos.Z;

			Action.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(trans, 100.0f);
		}

		return true;*/


		vr::IVRInput * VRInput =  vr::VRInput();

		Action.bHasValidData = false;

		if (!VRInput)
			return false;

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;

		if (Action.ActionHandleContainer.ActionHandle == vr::k_ulInvalidActionHandle)
		{

			// Not filling in the field as that is a waste, just assuming from the data sent in
			// Still allowing overriding manually though

			if (Action.ActionName.IsEmpty())
			{
				switch (Action.SkeletalData.TargetHand)
				{
				case EVRActionHand::EActionHand_Left:
				{
					Action.ActionName = OpenInputFunctionLibraryStatics::LeftHand_SkeletalActionName;
				}break;
				case EVRActionHand::EActionHand_Right:
				{
					Action.ActionName = OpenInputFunctionLibraryStatics::RightHand_SkeletalActionName;
				}break;
				default:break;
				}
			}

			InputError = VRInput->GetActionHandle(TCHAR_TO_UTF8(*Action.ActionName), &Action.ActionHandleContainer.ActionHandle);
			if (InputError != vr::EVRInputError::VRInputError_None)
			{
				Action.ActionHandleContainer.ActionHandle = vr::k_ulInvalidActionHandle;
				return false;
			}
		}

		uint32_t boneCount = 0;
		InputError = VRInput->GetBoneCount(Action.ActionHandleContainer.ActionHandle, &boneCount);

		// If the handle doesn't map to a correct action handle
		// Should likely throw an error here and stop getting a handle
		if (InputError == vr::EVRInputError::VRInputError_InvalidHandle)
		{
			Action.ActionHandleContainer.ActionHandle = vr::k_ulInvalidActionHandle;
			return false;

		}

		vr::InputSkeletalActionData_t SkeletalData;
		uint32_t ActionDataSize = sizeof(SkeletalData);
		InputError = VRInput->GetSkeletalActionData(Action.ActionHandleContainer.ActionHandle, &SkeletalData, ActionDataSize);

		if (InputError != vr::EVRInputError::VRInputError_None || !SkeletalData.bActive || boneCount < 1)
			return false;

		// Set bone count so we can reference it later
		Action.BoneCount = boneCount;

		// On first tick using this, load the bone parent index array
		if (!Action.BoneParentIndexes.Num())
		{
			Action.BoneParentIndexes.AddZeroed(Action.BoneCount);
			InputError = VRInput->GetBoneHierarchy(Action.ActionHandleContainer.ActionHandle, Action.BoneParentIndexes.GetData(), Action.BoneCount);

			if (InputError != vr::EVRInputError::VRInputError_None)
				return false;
		}

		// On first tick using this, load the skeletal tracking level
		if (Action.SkeletalTrackingLevel == EVROpenInputSkeletalTrackingLevel::VRSkeletalTrackingLevel_Max)
		{
			vr::EVRSkeletalTrackingLevel TrackingLevel;
			VRInput->GetSkeletalTrackingLevel(Action.ActionHandleContainer.ActionHandle, &TrackingLevel);

			if (InputError != vr::EVRInputError::VRInputError_None)
				return false;

			Action.SkeletalTrackingLevel = (EVROpenInputSkeletalTrackingLevel)TrackingLevel;
		}

		// If we are supposed to get gesture values, load them too
		if (bGetGestureValues)
		{
			vr::VRSkeletalSummaryData_t SkeletalSummaryData;
			VRInput->GetSkeletalSummaryData(Action.ActionHandleContainer.ActionHandle, vr::EVRSummaryType::VRSummaryType_FromAnimation, &SkeletalSummaryData);

			if (InputError != vr::EVRInputError::VRInputError_None)
				return false;

			Action.PoseFingerData.PoseFingerCurls.Reset(vr::VRFinger_Count);
			for (int i = 0; i < vr::VRFinger_Count; ++i)
			{
				Action.PoseFingerData.PoseFingerCurls.Add(SkeletalSummaryData.flFingerCurl[i]);
			}

			if (Action.SkeletalTrackingLevel == EVROpenInputSkeletalTrackingLevel::VRSkeletalTracking_Full)
			{
				Action.PoseFingerData.PoseFingerSplays.Reset(vr::VRFingerSplay_Count);
				for (int i = 0; i < vr::VRFingerSplay_Count; ++i)
				{
					Action.PoseFingerData.PoseFingerSplays.Add(SkeletalSummaryData.flFingerSplay[i]);
				}
			}
		}

		TArray<vr::VRBoneTransform_t> BoneTransforms;
		BoneTransforms.AddZeroed(Action.BoneCount);

		vr::EVRSkeletalMotionRange MotionTypeToGet = Action.bGetSkeletalTransforms_WithController ? vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController : vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithoutController;
		
		{
			InputError = VRInput->GetSkeletalBoneData(Action.ActionHandleContainer.ActionHandle, /*Action.SkeletalData.bGetTransformsInParentSpace ? */vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent/* : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model*/, MotionTypeToGet, BoneTransforms.GetData(), Action.BoneCount);
			Action.CompressedSize = 0;
			Action.CompressedTransforms.Empty(bGetCompressedData ? Action.BoneCount : 0);
		}
		
		// We got the transforms normally for the local player as they don't have the artifacts, but we get the compressed ones for remote sending
		if (bGetCompressedData)
		{			
			int32 MaxArraySize = ((sizeof(vr::VRBoneTransform_t) * Action.BoneCount) + 2);
			TArray<uint8> TempBuffer;
			TempBuffer.AddUninitialized(MaxArraySize);

			InputError = VRInput->GetSkeletalBoneDataCompressed(Action.ActionHandleContainer.ActionHandle, MotionTypeToGet, TempBuffer.GetData(), MaxArraySize, &Action.CompressedSize);
			Action.CompressedTransforms.Reset(Action.CompressedSize);
			Action.CompressedTransforms.AddUninitialized(Action.CompressedSize);
			FMemory::Memcpy(Action.CompressedTransforms.GetData(), TempBuffer.GetData(), Action.CompressedSize);
		}

		if (InputError != vr::EVRInputError::VRInputError_None)
			return false;

		if (Action.SkeletalData.SkeletalTransforms.Num() > 0)
		{
			Action.OldSkeletalTransforms = Action.SkeletalData.SkeletalTransforms;
		}

		if (Action.SkeletalData.SkeletalTransforms.Num() != Action.BoneCount)
		{
			Action.SkeletalData.SkeletalTransforms.Reset(Action.BoneCount);
			Action.SkeletalData.SkeletalTransforms.AddUninitialized(Action.BoneCount);
		}

		UWorld* World = (WorldContextObject) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		float WorldToMeters = ((World != nullptr) ? WorldToMeters = World->GetWorldSettings()->WorldToMeters : 100.f);


		if(Action.SkeletalData.bMirrorLeftRight)
			MIRROR_OPENINPUT_BONES(BoneTransforms);

		for (int i = 0; i < BoneTransforms.Num(); ++i)
		{
			Action.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(BoneTransforms[i], WorldToMeters);
		}

		/*if (UPrimitiveComponent* PrimPar = Cast<UPrimitiveComponent>(WorldContextObject))
		{
			FTransform SkeleTrans = Action.SkeletalData.SkeletalTransforms[1];
			SkeleTrans.SetRotation(SkeleTrans.GetRotation() * FRotator(0.f, 90.f, 90.f).Quaternion());
			FTransform WorldTrans = (SkeleTrans * PrimPar->GetComponentTransform());
			DrawDebugSphere(WorldContextObject->GetWorld(), WorldTrans.GetLocation(), 4.f, 32.f, FColor::White);
			DrawDebugLine(WorldContextObject->GetWorld(), WorldTrans.GetLocation(), WorldTrans.GetLocation() + (WorldTrans.GetRotation().GetForwardVector() * 100.f), FColor::Red);
			DrawDebugLine(WorldContextObject->GetWorld(), WorldTrans.GetLocation(), WorldTrans.GetLocation() + (WorldTrans.GetRotation().GetRightVector() * 100.f), FColor::Green);
			DrawDebugLine(WorldContextObject->GetWorld(), WorldTrans.GetLocation(), WorldTrans.GetLocation() + (WorldTrans.GetRotation().GetUpVector() * 100.f), FColor::Blue);
		}*/

		Action.bHasValidData = true;
		return true;

#endif
	}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool GetReferencePose(UPARAM(ref)FBPOpenVRActionInfo & BlankActionToFill, FBPOpenVRActionHandle ActionHandleToQuery, /*bool bGetTransformsInParentSpace,*/ class UObject* WorldContextObject, EVROpenInputReferencePose PoseTypeToRetreive)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		Action.bHasValidData = false;
		return false;
#else

		vr::IVRInput * VRInput =  vr::VRInput();

		BlankActionToFill.bHasValidData = false;
		//BlankActionToFill.SkeletalData.bGetTransformsInParentSpace = bGetTransformsInParentSpace;
		BlankActionToFill.ActionHandleContainer = ActionHandleToQuery;

		if (!VRInput)
			return false;

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;

		uint32_t boneCount = 0;
		InputError = VRInput->GetBoneCount(BlankActionToFill.ActionHandleContainer.ActionHandle, &boneCount);

		// If the handle doesn't map to a correct action handle
		// Should likely throw an error here and stop getting a handle
		if (InputError == vr::EVRInputError::VRInputError_InvalidHandle)
		{
			BlankActionToFill.ActionHandleContainer.ActionHandle = vr::k_ulInvalidActionHandle;
			return false;
		}

		vr::InputSkeletalActionData_t SkeletalData;
		uint32_t ActionDataSize = sizeof(SkeletalData);
		InputError = VRInput->GetSkeletalActionData(BlankActionToFill.ActionHandleContainer.ActionHandle, &SkeletalData, ActionDataSize);

		if (InputError != vr::EVRInputError::VRInputError_None || !SkeletalData.bActive || boneCount < 1)
			return false;

		// Set bone count so we can reference it later
		BlankActionToFill.BoneCount = boneCount;

		TArray<vr::VRBoneTransform_t> BoneTransforms;
		BoneTransforms.AddZeroed(BlankActionToFill.BoneCount);

		{
			InputError = VRInput->GetSkeletalReferenceTransforms(
				BlankActionToFill.ActionHandleContainer.ActionHandle, 
				vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent,
				//BlankActionToFill.SkeletalData.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, 
				(vr::EVRSkeletalReferencePose)PoseTypeToRetreive,
				BoneTransforms.GetData(), 
				BlankActionToFill.BoneCount);

			BlankActionToFill.CompressedSize = 0;
			BlankActionToFill.CompressedTransforms.Empty();
		}

		if (InputError != vr::EVRInputError::VRInputError_None)
			return false;

		if (BlankActionToFill.SkeletalData.SkeletalTransforms.Num() > 0)
		{
			BlankActionToFill.OldSkeletalTransforms = BlankActionToFill.SkeletalData.SkeletalTransforms;
		}

		if (BlankActionToFill.SkeletalData.SkeletalTransforms.Num() != BlankActionToFill.BoneCount)
		{
			BlankActionToFill.SkeletalData.SkeletalTransforms.Reset(BlankActionToFill.BoneCount);
			BlankActionToFill.SkeletalData.SkeletalTransforms.AddUninitialized(BlankActionToFill.BoneCount);
		}

		UWorld* World = (WorldContextObject) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		float WorldToMeters = ((World != nullptr) ? WorldToMeters = World->GetWorldSettings()->WorldToMeters : 100.f);

		if (BlankActionToFill.SkeletalData.bMirrorLeftRight)
			MIRROR_OPENINPUT_BONES(BoneTransforms);

		for (int i = 0; i < BoneTransforms.Num(); ++i)
			BlankActionToFill.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(BoneTransforms[i], WorldToMeters);

		BlankActionToFill.bHasValidData = true;
		return true;
#endif
	}

	// Gets the curl and splay values of a hand, this is generally only used if you aren't getting the full pose and filling out an ActionInfo structure.
	// If the optional custom action name is blank then it will use the plugins default values
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool GetHandCurlAndSplayValues(EVRActionHand TargetHand, UPARAM(ref) FBPOpenVRGesturePoseData & CurlAndSplayValuesOut, class UObject* WorldContextObject, FString OptionalCustomActionName)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		return false;
#else
		vr::IVRInput * VRInput =  vr::VRInput();

		if (!VRInput)
			return false;

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;

		if (OptionalCustomActionName.IsEmpty())
		{
			switch (TargetHand)
			{
			case EVRActionHand::EActionHand_Left:
			{
				OptionalCustomActionName = OpenInputFunctionLibraryStatics::LeftHand_SkeletalActionName;
			}break;
			case EVRActionHand::EActionHand_Right:
			{
				OptionalCustomActionName = OpenInputFunctionLibraryStatics::RightHand_SkeletalActionName;
			}break;
			default:break;
			}
		}

		vr::VRActionHandle_t ActionHandle;
		InputError = VRInput->GetActionHandle(TCHAR_TO_UTF8(*OptionalCustomActionName), &ActionHandle);
		if (InputError != vr::EVRInputError::VRInputError_None || ActionHandle == vr::k_ulInvalidActionHandle)
		{
			return false;
		}

		vr::VRSkeletalSummaryData_t SkeletalSummaryData;
		VRInput->GetSkeletalSummaryData(ActionHandle, vr::EVRSummaryType::VRSummaryType_FromAnimation, &SkeletalSummaryData);

		if (InputError != vr::EVRInputError::VRInputError_None)
			return false;

		CurlAndSplayValuesOut.PoseFingerCurls.Reset(vr::VRFinger_Count);
		for (int i = 0; i < vr::VRFinger_Count; ++i)
		{
			CurlAndSplayValuesOut.PoseFingerCurls.Add(SkeletalSummaryData.flFingerCurl[i]);
		}

		CurlAndSplayValuesOut.PoseFingerSplays.Reset(vr::VRFingerSplay_Count);
		for (int i = 0; i < vr::VRFingerSplay_Count; ++i)
		{
			CurlAndSplayValuesOut.PoseFingerSplays.Add(SkeletalSummaryData.flFingerSplay[i]);
		}

		return true;
#endif
	}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
	static bool GetSkeletalTrackingLevel(EVROpenInputSkeletalTrackingLevel & SkeletalTrackingLevelOut, EVRActionHand HandToRetreive)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		SkeletalTrackingLevelOut = EVROpenInputSkeletalTrackingLevel::VRSkeletalTrackingLevel_Max;
		return false;
#else
		vr::IVRInput * VRInput =  vr::VRInput();

		if (!VRInput)
			return false;

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;

		vr::VRActionHandle_t ActionHandle = vr::k_ulInvalidActionHandle;
		FString ActionName;

		switch (HandToRetreive)
		{
		case EVRActionHand::EActionHand_Left:
		{
			ActionName = OpenInputFunctionLibraryStatics::LeftHand_SkeletalActionName;
		}break;
		case EVRActionHand::EActionHand_Right:
		{
			ActionName = OpenInputFunctionLibraryStatics::RightHand_SkeletalActionName;
		}break;
		default:break;
		}

		InputError = VRInput->GetActionHandle(TCHAR_TO_UTF8(*ActionName), &ActionHandle);
		if (InputError != vr::EVRInputError::VRInputError_None)
		{
			SkeletalTrackingLevelOut = EVROpenInputSkeletalTrackingLevel::VRSkeletalTrackingLevel_Max;
			return false;
		}

		vr::EVRSkeletalTrackingLevel TrackingLevel;
		VRInput->GetSkeletalTrackingLevel(ActionHandle, &TrackingLevel);

		if (InputError != vr::EVRInputError::VRInputError_None)
		{
			SkeletalTrackingLevelOut = EVROpenInputSkeletalTrackingLevel::VRSkeletalTrackingLevel_Max;
			return false;
		}

		SkeletalTrackingLevelOut = (EVROpenInputSkeletalTrackingLevel)TrackingLevel;
		return true;

#endif
	}
};	
