// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HAL/FileManager.h"
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

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bGetTransformsInParentSpace;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bAllowDeformingMesh;

	// If true then the hand mesh will be mirrored, generally used for full body rigs to fix the left hands axis
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bMirrorHand;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		TArray<FTransform> SkeletalTransforms;

	// The rotation required to rotate the finger bones back to X+
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FTransform AdditionTransform;

	FBPOpenVRActionSkeletalData()
	{
		bGetTransformsInParentSpace = false;
		AdditionTransform = FTransform(FRotator(0.f, 90.f, 90.f), FVector::ZeroVector, FVector(1.f));
		bAllowDeformingMesh = true;
		bMirrorHand = false;
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRActionInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		bool bGetSkeletalTransforms_WithController;

	// Optional action name, if you are not using the default manifest generation then you can
	// set a custom name here, otherwise it will automatically be filled out.
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FString ActionName;

	// This is where you set the hand type / settings for the skeletal data
	UPROPERTY(EditAnywhere, NotReplicated, Transient, BlueprintReadWrite, Category = Default, meta = (AllowPrivateAccess = "true"))
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

	// The level of trackin that the OpenInputdevice has (only valid value if this bHasValidData)
	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		EVROpenInputSkeletalTrackingLevel SkeletalTrackingLevel;

	FBPOpenVRActionHandle ActionHandleContainer;
	FString LastHandGesture;

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
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		//Ar << CompressedSize;
		Ar << BoneCount;
		Ar << CompressedTransforms;

		return bOutSuccess;
	}
};

template<>
struct TStructOpsTypeTraits< FBPOpenVRActionInfo > : public TStructOpsTypeTraitsBase2<FBPOpenVRActionInfo>
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

	UFUNCTION(BlueprintPure)
	static FTransform GetOpenVRBoneTransform(EVROpenInputBones BoneToGet, FBPOpenVRActionInfo HandSkeletalAction)
	{
		uint8 index = (uint8)BoneToGet;
		if (HandSkeletalAction.SkeletalData.SkeletalTransforms.Num() - 1 >= index)
			return HandSkeletalAction.SkeletalData.SkeletalTransforms[index];

		return FTransform::Identity;
	}

#if STEAMVR_SUPPORTED_PLATFORM

	FORCEINLINE static FVector CONVERT_STEAMVECTOR_TO_FVECTOR(const vr::HmdVector4_t InVector)
	{
		return FVector(-InVector.v[2], InVector.v[0], InVector.v[1]);
	}

	FORCEINLINE static FVector CONVERT_STEAMVECTOR_TO_FVECTOR_Y(const vr::HmdVector4_t InVector)
	{
		return FVector(-InVector.v[0], -InVector.v[2], InVector.v[1]);
	}
	
	FORCEINLINE static FQuat CONVERT_STEAMQUAT_TO_FQUAT(const vr::HmdQuaternionf_t InQuat)
	{
		return FQuat(-InQuat.z, InQuat.x, InQuat.y, -InQuat.w);
	}
	
	FORCEINLINE static FTransform CONVERT_STEAMTRANS_TO_FTRANS(const vr::VRBoneTransform_t InTrans, float WorldToMeters)
	{
		return FTransform(
			FQuat(-InTrans.orientation.z, InTrans.orientation.x, InTrans.orientation.y, -InTrans.orientation.w),
			FVector(-InTrans.position.v[2], InTrans.position.v[0], InTrans.position.v[1]) * WorldToMeters
		);
	}

#endif

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	/*UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true"))
	static bool SetActionsManifest(int32 DeviceIndex)
{
#if !STEAMVR_SUPPORTED_PLATFORM
	return false;
#else

		vr::EVRInitError Initerror;
		vr::IVRInput * VRInput = (vr::IVRInput*)vr::VR_GetGenericInterface(vr::IVRInput_Version, &Initerror);

		if (!VRInput)
			return false;

		FString manifestPath = FPaths::ProjectDir() + "legacy_actions.json";
		manifestPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*manifestPath);
		auto buff = StringCast<ANSICHAR>(*manifestPath);
		vr::EVRInputError InputError = VRInput->SetActionManifestPath(buff.Get());

		if (true)
			int val = 0;
		return true;

#endif
}*/

	// Decompresses compressed bone data from OpenInput
	static bool DecompressSkeletalData(FBPOpenVRActionInfo & Action, UWorld * WorldToUseForScale)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		return false;
#else
		if (Action.CompressedTransforms.Num() < 1 || Action.CompressedSize < 1 || !WorldToUseForScale)
			return false;

		vr::EVRInitError Initerror;
		vr::IVRInput * VRInput = (vr::IVRInput*)vr::VR_GetGenericInterface(vr::IVRInput_Version, &Initerror);

		Action.bHasValidData = false;

		if (!VRInput)
			return false;

		TArray<vr::VRBoneTransform_t> BoneTransforms;
		BoneTransforms.AddZeroed(Action.BoneCount);

		Action.CompressedSize = Action.CompressedTransforms.Num();

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;
		vr::EVRSkeletalTransformSpace TransSpace = Action.SkeletalData.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model;
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

		for (int i = 0; i < BoneTransforms.Num(); ++i)
			Action.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(BoneTransforms[i], WorldToMeters);


		return true;
#endif
	}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool GetActionPose(UPARAM(ref)FBPOpenVRActionInfo & Action, class UObject* WorldContextObject, bool bGetCompressedData = false, bool bGetGestureValues = false)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		Action.bHasValidData = false;
		return false;
#else
		vr::EVRInitError Initerror;
		vr::IVRInput * VRInput = (vr::IVRInput*)vr::VR_GetGenericInterface(vr::IVRInput_Version, &Initerror);

		Action.bHasValidData = false;

		if (!VRInput)
			return false;

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;

		if (Action.ActionHandleContainer.ActionHandle == vr::k_ulInvalidActionHandle)
		{

			// Not filling in the field as that is a waste, just assuming from the data sent in
			// Still allowing overriding manually though
			FString ActionNameToTarget = Action.ActionName;

			if (ActionNameToTarget.IsEmpty())
			{
				switch (Action.SkeletalData.TargetHand)
				{
				case EVRActionHand::EActionHand_Left:
				{
					ActionNameToTarget = FString("/actions/main/in/lefthand_skeleton");
				}break;
				case EVRActionHand::EActionHand_Right:
				{
					ActionNameToTarget = FString("/actions/main/in/righthand_skeleton");
				}break;
				default:break;
				}
			}

			InputError = VRInput->GetActionHandle(TCHAR_TO_UTF8(*ActionNameToTarget), &Action.ActionHandleContainer.ActionHandle);
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
			VRInput->GetSkeletalSummaryData(Action.ActionHandleContainer.ActionHandle, &SkeletalSummaryData);

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
			InputError = VRInput->GetSkeletalBoneData(Action.ActionHandleContainer.ActionHandle, Action.SkeletalData.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, MotionTypeToGet, BoneTransforms.GetData(), Action.BoneCount);
			Action.CompressedSize = 0;
			Action.CompressedTransforms.Empty(bGetCompressedData ? Action.BoneCount : 0);
		}
		
		// We got the transforms normally for the local player as they don't have the artifacts, but we get the compressed ones for remote sending
		if (bGetCompressedData)
		{			
			int32 MaxArraySize = (sizeof(vr::VRBoneTransform_t) * (Action.BoneCount + 2));
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

		for (int i = 0; i < BoneTransforms.Num(); ++i)
		{
			Action.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(BoneTransforms[i], WorldToMeters);
		}

		Action.bHasValidData = true;
		return true;

#endif
	}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool GetReferencePose(UPARAM(ref)FBPOpenVRActionInfo & BlankActionToFill, FBPOpenVRActionHandle ActionHandleToQuery, bool bGetTransformsInParentSpace, class UObject* WorldContextObject, EVROpenInputReferencePose PoseTypeToRetreive)
	{
#if !STEAMVR_SUPPORTED_PLATFORM
		Action.bHasValidData = false;
		return false;
#else
		vr::EVRInitError Initerror;
		vr::IVRInput * VRInput = (vr::IVRInput*)vr::VR_GetGenericInterface(vr::IVRInput_Version, &Initerror);

		BlankActionToFill.bHasValidData = false;
		BlankActionToFill.SkeletalData.bGetTransformsInParentSpace = bGetTransformsInParentSpace;
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
				BlankActionToFill.SkeletalData.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, 
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

		for (int i = 0; i < BoneTransforms.Num(); ++i)
			BlankActionToFill.SkeletalData.SkeletalTransforms[i] = CONVERT_STEAMTRANS_TO_FTRANS(BoneTransforms[i], WorldToMeters);

		BlankActionToFill.bHasValidData = true;
		return true;
#endif
	}
};	
