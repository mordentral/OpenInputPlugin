// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimNode_ApplyOpenInputTransform.h"

#if USE_WITH_VR_EXPANSION
#include "VRBPDatatypes.h"
#endif

#include "OpenInputFunctionLibrary.h"
#include "Engine/DataAsset.h"

#include "OpenInputSkeletalMeshComponent.generated.h"

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
		// #TODO: Convert to new 4.22 HasLocalNetOwner eventually
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkeletalData|Actions")
		TArray<FBPOpenVRActionInfo> HandSkeletalActions;

	UPROPERTY(Replicated, Transient, ReplicatedUsing = OnRep_SkeletalTransformLeft)
		FBPSkeletalRepContainer LeftHandRep;

	UPROPERTY(Replicated, Transient, ReplicatedUsing = OnRep_SkeletalTransformRight)
		FBPSkeletalRepContainer RightHandRep;

	// This one specifically sends out the new relative location for a retain secondary grip
	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendSkeletalTransforms(const FBPSkeletalRepContainer& SkeletalInfo);

	bool bLerpingPositionLeft;
	bool bReppedOnceLeft;

	bool bLerpingPositionRight;
	bool bReppedOnceRight;

	struct FTransformLerpManager
	{
		bool bReplicatedOnce;
		bool bLerping;
		float UpdateCount;
		float UpdateRate;
		TArray<FTransform> NewTransforms;

		FTransformLerpManager()
		{
			bReplicatedOnce = false;
			bLerping = false;
			UpdateCount = 0.0f;
			UpdateRate = 0.0f;
		}

		void NotifyNewData(FBPOpenVRActionInfo& ActionInfo, int NetUpdateRate)
		{
			UpdateRate = (1.0f / NetUpdateRate);
			if (bReplicatedOnce)
			{
				bLerping = true;
				UpdateCount = 0.0f;
				NewTransforms = ActionInfo.SkeletalData.SkeletalTransforms;
			}
			else
			{
				bReplicatedOnce = true;
			}
		}

		void UpdateManager(float DeltaTime, FBPOpenVRActionInfo& ActionInfo)
		{
			if (bLerping)
			{
				UpdateCount += DeltaTime;
				float LerpVal = FMath::Clamp(UpdateCount / UpdateRate, 0.0f, 1.0f);

				if (LerpVal >= 1.0f)
				{
					bLerping = false;
					UpdateCount = 0.0f;
					ActionInfo.SkeletalData.SkeletalTransforms = NewTransforms;
				}
				else
				{
					if (NewTransforms.Num() != ActionInfo.SkeletalData.SkeletalTransforms.Num() || NewTransforms.Num() != ActionInfo.OldSkeletalTransforms.Num())
					{
						return;
					}

					for (int i = 0; i < ActionInfo.SkeletalData.SkeletalTransforms.Num(); i++)
					{
						ActionInfo.SkeletalData.SkeletalTransforms[i].Blend(ActionInfo.OldSkeletalTransforms[i], NewTransforms[i], LerpVal);
					}
				}
			}
		}

	}; 
	
	FTransformLerpManager LeftHandRepManager;
	FTransformLerpManager RightHandRepManager;

	UFUNCTION()
	virtual void OnRep_SkeletalTransformLeft()
	{
		for (int i = 0; i < HandSkeletalActions.Num(); i++)
		{
			if (HandSkeletalActions[i].SkeletalData.TargetHand == LeftHandRep.TargetHand)
			{
				if (LeftHandRep.ReplicationType != EVRSkeletalReplicationType::Rep_SteamVRCompressedTransforms)
					HandSkeletalActions[i].OldSkeletalTransforms = HandSkeletalActions[i].SkeletalData.SkeletalTransforms;

				FBPSkeletalRepContainer::CopyReplicatedTo(LeftHandRep, HandSkeletalActions[i]);

				if (HandSkeletalActions[i].CompressedTransforms.Num() > 0)
				{
					UOpenInputFunctionLibrary::DecompressSkeletalData(HandSkeletalActions[i], GetWorld());
					HandSkeletalActions[i].CompressedTransforms.Reset();
				}
				
				if(bSmoothReplicatedSkeletalData)
					LeftHandRepManager.NotifyNewData(HandSkeletalActions[i], ReplicationRateForSkeletalAnimations);
				break;
			}
		}
	}

	UFUNCTION()
	virtual void OnRep_SkeletalTransformRight()
	{
		for (int i = 0; i < HandSkeletalActions.Num(); i++)
		{
			if (HandSkeletalActions[i].SkeletalData.TargetHand == RightHandRep.TargetHand)
			{
				if (RightHandRep.ReplicationType != EVRSkeletalReplicationType::Rep_SteamVRCompressedTransforms)
					HandSkeletalActions[i].OldSkeletalTransforms = HandSkeletalActions[i].SkeletalData.SkeletalTransforms;

				FBPSkeletalRepContainer::CopyReplicatedTo(RightHandRep, HandSkeletalActions[i]);

				if (HandSkeletalActions[i].CompressedTransforms.Num() > 0)
				{
					UOpenInputFunctionLibrary::DecompressSkeletalData(HandSkeletalActions[i], GetWorld());
					HandSkeletalActions[i].CompressedTransforms.Reset();
				}
				
				if (bSmoothReplicatedSkeletalData)
					RightHandRepManager.NotifyNewData(HandSkeletalActions[i], ReplicationRateForSkeletalAnimations);
				break;
			}
		}
	}

	// If we should replicate the skeletal transform data
	UPROPERTY(EditAnywhere, Category = SkeletalData)
		bool bReplicateSkeletalData;

	// If true we will lerp between updates of the skeletal mesh transforms and smooth the result
	UPROPERTY(EditAnywhere, Category = SkeletalData)
		bool bSmoothReplicatedSkeletalData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalData)
		float ReplicationRateForSkeletalAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalData)
		EVRSkeletalReplicationType ReplicationType;

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

	UFUNCTION(BlueprintCallable, Category = "BoneMappings")
	void InitializeCustomBoneMapping(UPARAM(ref) FBPSkeletalMappingData & SkeletalMappingData)
	{
		USkeleton* AssetSkeleton = this->CurrentSkeleton;//RequiredBones.GetSkeletonAsset();

		if (AssetSkeleton)
		{
			FBoneContainer &RequiredBones = this->GetRequiredBones();
			for (FBPOpenVRSkeletalPair& BonePair : SkeletalMappingData.BonePairs)
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
			}

			SkeletalMappingData.bInitialized = true;
			return;
		}

		SkeletalMappingData.bInitialized = false;
	}


};
