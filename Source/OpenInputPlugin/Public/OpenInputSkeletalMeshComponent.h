// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"

#if USE_WITH_VR_EXPANSION
#include "VRBPDatatypes.h"
#endif

#include "OpenInputFunctionLibrary.h"
#include "Engine/DataAsset.h"

#include "OpenInputSkeletalMeshComponent.generated.h"

/*
USTRUCT(BlueprintType, Category = "OpenInputLibrary|SkeletalTransform")
struct FSkeletalTransform_NetQuantize : public FTransform_NetQuantize
{
	GENERATED_USTRUCT_BODY()
public:

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		FVector rTranslation;
		//FVector rScale3D;
		//FQuat rRotation;
		FRotator rRotation;

		uint16 ShortPitch = 0;
		uint16 ShortYaw = 0;
		uint16 ShortRoll = 0;

		if (Ar.IsSaving())
		{
			// Because transforms can be vectorized or not, need to use the inline retrievers
			rTranslation = this->GetTranslation();
			//rScale3D = this->GetScale3D();
			rRotation = this->Rotator();//this->GetRotation();

				// Translation set to 2 decimal precision
				bOutSuccess &= SerializePackedVector<100, 12>(rTranslation, Ar);

				// Rotation converted to FRotator and short compressed, see below for conversion reason
				// FRotator already serializes compressed short by default but I can save a func call here
				rRotation.SerializeCompressedShort(Ar);
		}
		else // If loading
		{

			bOutSuccess &= SerializePackedVector<100, 12>(rTranslation, Ar);
			rRotation.SerializeCompressedShort(Ar);

			// Set it
			this->SetComponents(rRotation.Quaternion(), rTranslation, FVector(1.f));
		}

		return bOutSuccess;
	}
};

template<>
struct TStructOpsTypeTraits< FSkeletalTransform_NetQuantize > : public TStructOpsTypeTraitsBase2<FSkeletalTransform_NetQuantize>
{
	enum
	{
		WithNetSerializer = true
	};
};
*/

USTRUCT(BlueprintType, Category = "VRGestures")
struct OPENINPUTPLUGIN_API FOpenInputGestureFingerPosition
{
	GENERATED_BODY()
public:

	// The Finger index, not editable
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VRGesture")
		EVROpenInputFingerIndexType IndexType;

	// The value of this element 0.f - 1.f
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
	float Value;

	// The threshold within which this finger value will be detected as matching (1.0 would be always matching, IE: finger doesn't count)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Threshold;

	FOpenInputGestureFingerPosition(float CurlOrSplay, EVROpenInputFingerIndexType Type)
	{
		IndexType = Type;
		Value = CurlOrSplay;
		Threshold = 0.1f;
	}
	FOpenInputGestureFingerPosition()
	{
		IndexType = EVROpenInputFingerIndexType::VRFinger_Index;
		Value = 0.f;
		Threshold = 0.1f;
	}
};

USTRUCT(BlueprintType, Category = "VRGestures")
struct OPENINPUTPLUGIN_API FOpenInputGesture
{
	GENERATED_BODY()
public:

	// Name of the recorded gesture
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		FName Name;

	// Samples in the recorded gesture
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		TArray<FOpenInputGestureFingerPosition> FingerValues;

	// If we should only use the curl element for gesture detection
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		bool bUseFingerCurlOnly;

	FOpenInputGesture()
	{
		bUseFingerCurlOnly = false;
		InitPoseValues();
		Name = NAME_None;
	}

	FOpenInputGesture(bool bOnlyFingerCurl)
	{
		bUseFingerCurlOnly = bOnlyFingerCurl;
		InitPoseValues();
		Name = NAME_None;
	}

	void InitPoseValues()
	{
		int PoseCount = vr::VRFinger_Count;
		if (!bUseFingerCurlOnly)
			PoseCount += vr::VRFingerSplay_Count;

		for (int i = 0; i < PoseCount; ++i)
		{
			FingerValues.Add(FOpenInputGestureFingerPosition(0.f, (EVROpenInputFingerIndexType)i));
		}
	}
};

/**
* Items Database DataAsset, here we can save all of our game items
*/
UCLASS(BlueprintType, Category = "VRGestures")
class OPENINPUTPLUGIN_API UOpenInputGestureDatabase : public UDataAsset
{
	GENERATED_BODY()
public:

	// Gestures in this database
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		TArray <FOpenInputGesture> Gestures;

	UOpenInputGestureDatabase()
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOpenVRGestureDetected, const FName &, GestureDetected, int32, GestureIndex, EVRActionHand, ActionHandType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOpenVRGestureEnded, const FName &, GestureEnded, int32, GestureIndex, EVRActionHand, ActionHandType);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class OPENINPUTPLUGIN_API UOpenInputSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UOpenInputSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer);

	// Says whether we should run gesture detection
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		bool bDetectGestures;

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void SetDetectGestures(bool bNewDetectGestures)
	{
		bDetectGestures = bNewDetectGestures;
	}

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
	bool GetFingerCurlAndSplayData(EVRActionHand TargetHand, FBPOpenVRGesturePoseData & OutFingerPoseData)
	{
		for (int i = 0; i < HandSkeletalActions.Num(); ++i)
		{
			if (HandSkeletalActions[i].SkeletalData.TargetHand == TargetHand)
			{
				OutFingerPoseData = HandSkeletalActions[i].PoseFingerData;
				return true;
			}
		}
		
		return false;
	}

	UPROPERTY(BlueprintAssignable, Category = "VRGestures")
		FOpenVRGestureDetected OnNewGestureDetected;

	UPROPERTY(BlueprintAssignable, Category = "VRGestures")
		FOpenVRGestureEnded OnGestureEnded;

	// Known sequences
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		UOpenInputGestureDatabase *GesturesDB;

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void SaveCurrentPose(FName RecordingName, bool bUseFingerCurlOnly = true, EVRActionHand HandToSave = EVRActionHand::EActionHand_Right);

	UFUNCTION(BlueprintCallable, Category = "VRGestures", meta = (DisplayName = "DetectCurrentPose"))
		bool K2_DetectCurrentPose(FBPOpenVRActionInfo &SkeletalAction, FOpenInputGesture & GestureOut);

	// This version throws events
	bool DetectCurrentPose(FBPOpenVRActionInfo &SkeletalAction);

	// Need this as I can't think of another way for an actor component to make sure it isn't on the server
	inline bool IsLocallyControlled() const
	{
		// I like epics new authority check more than mine
		const AActor* MyOwner = GetOwner();
		const APawn* MyPawn = Cast<APawn>(MyOwner);
		return MyPawn ? MyPawn->IsLocallyControlled() : (MyOwner && MyOwner->Role == ENetRole::ROLE_Authority);

		// HasLocalNetOwner
	}

	// Using tick and not timers because skeletal components tick anyway, kind of a waste to make another tick by adding a timer over that
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Offset by a VRExpansionPlugin controller profile, does nothing if not using that plugin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControllerProfile")
		bool bOffsetByControllerProfile;

	// Stores current transform so we don't have to keep casting
	FTransform CurrentControllerProfileTransform;

	FDelegateHandle NewControllerProfileEvent_Handle;
	UFUNCTION()
		void NewControllerProfileLoaded();
	void GetCurrentProfileTransform(bool bBindToNoticationDelegate);

	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkeletalData|Actions"/*, Replicated, ReplicatedUsing = OnRep_SkeletalTransforms*/)
		TArray<FBPOpenVRActionInfo> HandSkeletalActions;

	// This one specifically sends out the new relative location for a retain secondary grip
	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendSkeletalTransforms(const FBPOpenVRActionInfo &ActionInfo);

	UFUNCTION()
		virtual void OnRep_SkeletalTransforms()
	{
		
		//UOpenInputFunctionLibrary::DecompressSkeletalData(HandSkeletalAction, GetWorld());

		// Convert to skeletal data from compressed here
		//ReplicatedControllerTransform.Unpack();
	}

	UPROPERTY(EditAnywhere, Category = SkeletalData)
		bool bReplicateSkeletalData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalData)
		float ReplicationRateForSkeletalAnimations;

	// Used in Tick() to accumulate before sending updates, didn't want to use a timer in this case, also used for remotes to lerp position
	float SkeletalNetUpdateCount;
	// Used in Tick() to accumulate before sending updates, didn't want to use a timer in this case, also used for remotes to lerp position
	float SkeletalUpdateCount;
};	

USTRUCT()
struct OPENINPUTPLUGIN_API FOpenInputAnimInstanceProxy : public FAnimInstanceProxy
{
public:
	GENERATED_BODY()

		FOpenInputAnimInstanceProxy() {}
		FOpenInputAnimInstanceProxy(UAnimInstance* InAnimInstance);

		/** Called before update so we can copy any data we need */
		virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;

public:

	EVRActionHand TargetHand;
	TArray<FBPOpenVRActionSkeletalData> HandSkeletalActionData;

};

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class OPENINPUTPLUGIN_API UOpenInputAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(transient)
		UOpenInputSkeletalMeshComponent * OwningMesh;

	FOpenInputAnimInstanceProxy AnimInstanceProxy;

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override
	{
		return new FOpenInputAnimInstanceProxy(this);
		//return &AnimInstanceProxy;
	}

	virtual void NativeInitializeAnimation() override
	{
		Super::NativeInitializeAnimation();

		OwningMesh = Cast<UOpenInputSkeletalMeshComponent>(GetOwningComponent());

	}


};
