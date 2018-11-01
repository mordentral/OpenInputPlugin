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
	// No prefix = 1000 series
	//Prop_TrackingSystemName_String_1000				UMETA(DisplayName = "Prop_TrackingSystemName_String"),


USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENINPUTPLUGIN_API FBPOpenVRActionInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FString ActionName;
	
	//UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bGetTransformsInParentSpace;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bAllowDeformingMesh;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = Default)
		TArray<FTransform> SkeletalTransforms;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = Default)
		TArray<FTransform> OldSkeletalTransforms;

	// The rotation required to rotate the root bone back to X+
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FTransform AdditionTransform;

	// The rotation required to rotate the root bone back to X+
	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FTransform RootAdditionTransform;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = Default)
		bool bHasValidData;

	vr::VRActionHandle_t ActionHandle;

	UPROPERTY()
	TArray<uint8> CompressedTransforms;
	UPROPERTY(NotReplicated)
	uint32 CompressedSize;
	UPROPERTY()
	int8 BoneCount;

	FBPOpenVRActionInfo()
	{
		ActionHandle = vr::k_ulInvalidActionHandle;
		bHasValidData = false;
		CompressedSize = 0;
		BoneCount = 0;
		bGetTransformsInParentSpace = false;
		AdditionTransform = FTransform(FRotator(0.f, 90.f, 90.f),FVector::ZeroVector, FVector(1.f));
		RootAdditionTransform = FTransform::Identity;
		bAllowDeformingMesh = true;
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

#if STEAMVR_SUPPORTED_PLATFORM

	FORCEINLINE static FVector CONVERT_STEAMVECTOR_TO_FVECTOR(const vr::HmdVector4_t InVector)
	{
		return FVector(-InVector.v[2], InVector.v[0], InVector.v[1]);
	}

#endif

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true"))
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
}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true"))
		static bool SetupActionHandles(FBPOpenVRActionInfo & Action)
	{
		vr::EVRInitError Initerror;
		vr::IVRInput * VRInput = (vr::IVRInput*)vr::VR_GetGenericInterface(vr::IVRInput_Version, &Initerror);

		if (!VRInput)
			return false;

		vr::EVRInputError InputError = vr::EVRInputError::VRInputError_None;

		if (Action.ActionHandle == vr::k_ulInvalidActionHandle)
		{
			InputError = VRInput->GetActionHandle(TCHAR_TO_UTF8(*Action.ActionName), &Action.ActionHandle);
			if (InputError != vr::EVRInputError::VRInputError_None)
			{
				Action.ActionHandle = vr::k_ulInvalidActionHandle;
				return false;
			}
		}

		return true;
	}

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
		vr::EVRSkeletalTransformSpace TransSpace = Action.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model;
		InputError = VRInput->DecompressSkeletalBoneData(Action.CompressedTransforms.GetData(), Action.CompressedSize, &TransSpace, BoneTransforms.GetData(), Action.BoneCount);

		if (InputError != vr::EVRInputError::VRInputError_None)
			return false;

		if (Action.SkeletalTransforms.Num() > 0)
		{
			Action.OldSkeletalTransforms = Action.SkeletalTransforms;
		}

		if (Action.SkeletalTransforms.Num() != Action.BoneCount)
		{
			Action.SkeletalTransforms.Reset(Action.BoneCount);
			Action.SkeletalTransforms.AddUninitialized(Action.BoneCount);
		}

		float WorldToMeters = ((WorldToUseForScale != nullptr) ? WorldToMeters = WorldToUseForScale->GetWorldSettings()->WorldToMeters : 100.f);

		for (int i = 0; i < BoneTransforms.Num(); i++)
		{
			FQuat Orientation(-BoneTransforms[i].orientation.z, BoneTransforms[i].orientation.x, BoneTransforms[i].orientation.y, -BoneTransforms[i].orientation.w);
			Action.SkeletalTransforms[i] = FTransform(Orientation, CONVERT_STEAMVECTOR_TO_FVECTOR(BoneTransforms[i].position) * WorldToMeters);
		}


		return true;
#endif
	}

	// Checks if a specific OpenVR device is connected, index names are assumed, they may not be exact
	UFUNCTION(BlueprintCallable, Category = "OpenInputFunctions|SteamVR", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool GetActionPose(UPARAM(ref)FBPOpenVRActionInfo & Action, class UObject* WorldContextObject,bool bGetControllerSkeleton = false, bool bGetCompressedData = false)
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

		if (Action.ActionHandle == vr::k_ulInvalidActionHandle)
		{
			InputError = VRInput->GetActionHandle(TCHAR_TO_UTF8(*Action.ActionName), &Action.ActionHandle);
			if (InputError != vr::EVRInputError::VRInputError_None)
			{
				Action.ActionHandle = vr::k_ulInvalidActionHandle;
				return false;
			}
		}

		vr::InputSkeletalActionData_t SkeletalData;
		uint32_t ActionDataSize = sizeof(SkeletalData);
		InputError = VRInput->GetSkeletalActionData(Action.ActionHandle, &SkeletalData, ActionDataSize, vr::k_ulInvalidInputValueHandle);
		
		// If the handle doesn't map to a correct action handle
		// Should likely throw an error here and stop getting a handle
		if (InputError == vr::EVRInputError::VRInputError_InvalidHandle)
		{
			Action.ActionHandle = vr::k_ulInvalidActionHandle;
			return false;

		}

		if (InputError != vr::EVRInputError::VRInputError_None || !SkeletalData.bActive || SkeletalData.boneCount < 1)
			return false;

		// Set bone count so we can reference it later
		Action.BoneCount = SkeletalData.boneCount;

		TArray<vr::VRBoneTransform_t> BoneTransforms;
		BoneTransforms.AddZeroed(Action.BoneCount);

		vr::EVRSkeletalMotionRange MotionTypeToGet = bGetControllerSkeleton ? vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController : vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithoutController;
		
		{
			InputError = VRInput->GetSkeletalBoneData(Action.ActionHandle, Action.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, MotionTypeToGet, BoneTransforms.GetData(), SkeletalData.boneCount, vr::k_ulInvalidInputValueHandle);
			Action.CompressedSize = 0;
			Action.CompressedTransforms.Empty();
		}
		
		// We got the transforms normally for the local player as they don't have the artifacts, but we get the compressed ones for remote sending
		if (bGetCompressedData)
		{			
			int32 MaxArraySize = (sizeof(vr::VRBoneTransform_t) * (SkeletalData.boneCount + 2));
			TArray<uint8> TempBuffer;
			TempBuffer.AddUninitialized(MaxArraySize);

			InputError = VRInput->GetSkeletalBoneDataCompressed(Action.ActionHandle, Action.bGetTransformsInParentSpace ? vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Parent : vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, MotionTypeToGet, TempBuffer.GetData(), MaxArraySize, &Action.CompressedSize, vr::k_ulInvalidInputValueHandle);
			Action.CompressedTransforms.Reset(Action.CompressedSize);
			Action.CompressedTransforms.AddUninitialized(Action.CompressedSize);
			FMemory::Memcpy(Action.CompressedTransforms.GetData(), TempBuffer.GetData(), Action.CompressedSize);
		}

		// If the handle doesn't map to a correct action handle
		// Should likely throw an error here and stop getting a handle
		if (InputError == vr::EVRInputError::VRInputError_InvalidHandle)
		{
			Action.ActionHandle = vr::k_ulInvalidActionHandle;
			return false;
		}

		if (InputError != vr::EVRInputError::VRInputError_None)
			return false;

		if (Action.SkeletalTransforms.Num() > 0)
		{
			Action.OldSkeletalTransforms = Action.SkeletalTransforms;
		}

		if (Action.SkeletalTransforms.Num() != SkeletalData.boneCount)
		{
			Action.SkeletalTransforms.Reset(SkeletalData.boneCount);
			Action.SkeletalTransforms.AddUninitialized(SkeletalData.boneCount);
		}

		/*struct HmdQuaternionf_t
		{
		float w, x, y, z;
		};
		OutOrientation.X = -Orientation.Z;
		OutOrientation.Y = Orientation.X;
		OutOrientation.Z = Orientation.Y;
		OutOrientation.W = -Orientation.W;*/
		UWorld* World = (WorldContextObject) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		float WorldToMeters = ((World != nullptr) ? WorldToMeters = World->GetWorldSettings()->WorldToMeters : 100.f);

		for (int i = 0; i < BoneTransforms.Num(); i++)
		{
			FQuat Orientation(-BoneTransforms[i].orientation.z, BoneTransforms[i].orientation.x, BoneTransforms[i].orientation.y, -BoneTransforms[i].orientation.w);
			Action.SkeletalTransforms[i] = FTransform(Orientation, CONVERT_STEAMVECTOR_TO_FVECTOR(BoneTransforms[i].position) * WorldToMeters);
		}

		Action.bHasValidData = true;
		return true;

#endif
	}

};	
