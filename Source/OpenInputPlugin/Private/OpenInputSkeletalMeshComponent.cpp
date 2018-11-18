// Fill out your copyright notice in the Description page of Project Settings.
#include "OpenInputSkeletalMeshComponent.h"
#include "GripMotionControllerComponent.h"
#include "Net/UnrealNetwork.h"
#include "VRGlobalSettings.h"
#include "XRMotionControllerBase.h" // for GetHandEnumForSourceName()
//#include "EngineMinimal.h"

UOpenInputSkeletalMeshComponent::UOpenInputSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//PrimaryComponentTick.bCanEverTick = false;
	//PrimaryComponentTick.bStartWithTickEnabled = false;

	bGetSkeletalTransforms_WithController = false;

	ReplicationRateForSkeletalAnimations = 60.f;
	bReplicateSkeletalData = false;
	bOffsetByControllerProfile = true;
	SkeletalNetUpdateCount = 0.f;

	bDetectGestures = true;
}

void UOpenInputSkeletalMeshComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Skipping the owner with this as the owner will use the controllers location directly
	DOREPLIFETIME_CONDITION(UOpenInputSkeletalMeshComponent, HandSkeletalAction, COND_SkipOwner);
}

void UOpenInputSkeletalMeshComponent::Server_SendSkeletalTransforms_Implementation(const FBPOpenVRActionInfo &ActionInfo)
{
	// Beta code, server sided reps will be triggering more often than we want currently
	HandSkeletalAction.CompressedTransforms = ActionInfo.CompressedTransforms;
	HandSkeletalAction.BoneCount = ActionInfo.BoneCount;
	HandSkeletalAction.CompressedSize = ActionInfo.CompressedSize;
	OnRep_SkeletalTransforms();
}

bool UOpenInputSkeletalMeshComponent::Server_SendSkeletalTransforms_Validate(const FBPOpenVRActionInfo &ActionInfo)
{
	return true;
}

void UOpenInputSkeletalMeshComponent::NewControllerProfileLoaded()
{
	GetCurrentProfileTransform(false);
}

void UOpenInputSkeletalMeshComponent::GetCurrentProfileTransform(bool bBindToNoticationDelegate)
{
	// Need to rep this offset to the server and then down to remote clients as well, otherwise it will not perform correctly
	if (bOffsetByControllerProfile)
	{
		UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();

		if (VRSettings == nullptr)
			return;

		FTransform NewControllerProfileTransform = FTransform::Identity;

		if (!bIsForRightHand || !VRSettings->bUseSeperateHandTransforms)
		{
			NewControllerProfileTransform = VRSettings->CurrentControllerProfileTransform;
		}
		else if (bIsForRightHand)
		{
			NewControllerProfileTransform = VRSettings->CurrentControllerProfileTransformRight;
		}

		if (bBindToNoticationDelegate && !NewControllerProfileEvent_Handle.IsValid())
		{
			NewControllerProfileEvent_Handle = VRSettings->OnControllerProfileChangedEvent.AddUObject(this, &UOpenInputSkeletalMeshComponent::NewControllerProfileLoaded);
		}

		if (!NewControllerProfileTransform.Equals(CurrentControllerProfileTransform))
		{
			FTransform OriginalControllerProfileTransform = CurrentControllerProfileTransform;
			CurrentControllerProfileTransform = NewControllerProfileTransform;

			// Auto adjust ourselves
			this->SetRelativeTransform(CurrentControllerProfileTransform.Inverse() * OriginalControllerProfileTransform);// *(OriginalControllerProfileTransform.Inverse() * this->GetRelativeTransform()));
		}
	}
}

void FOpenInputAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	Super::PreUpdate(InAnimInstance, DeltaSeconds);

	if (const UOpenInputSkeletalMeshComponent * OwningMesh = Cast<UOpenInputSkeletalMeshComponent>(InAnimInstance->GetOwningComponent()))
	{
		HandSkeletalActionData = OwningMesh->HandSkeletalAction;
	}
}

FOpenInputAnimInstanceProxy::FOpenInputAnimInstanceProxy(UAnimInstance* InAnimInstance)
	: FAnimInstanceProxy(InAnimInstance)
{
}


void UOpenInputSkeletalMeshComponent::OnUnregister()
{
	if (NewControllerProfileEvent_Handle.IsValid())
	{
		UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();
		if (VRSettings != nullptr)
		{
			VRSettings->OnControllerProfileChangedEvent.Remove(NewControllerProfileEvent_Handle);
			NewControllerProfileEvent_Handle.Reset();
		}
	}
	Super::OnUnregister();
}

void UOpenInputSkeletalMeshComponent::BeginPlay()
{

	if (UGripMotionControllerComponent * MotionParent = Cast<UGripMotionControllerComponent>(GetAttachParent()))
	{
		EControllerHand HandType;
		MotionParent->GetHandType(HandType);

		if (HandType == EControllerHand::Left || HandType == EControllerHand::AnyHand)
		{
			bIsForRightHand = false;
		}
		else
			bIsForRightHand = true;
	}
	Super::BeginPlay();
}

void UOpenInputSkeletalMeshComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UOpenInputSkeletalMeshComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
}

void UOpenInputSkeletalMeshComponent::Deactivate()
{
	Super::Deactivate();
}


void UOpenInputSkeletalMeshComponent::OnUpdateSkeletalData()
{
}


void UOpenInputSkeletalMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (!IsLocallyControlled())
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	if (bOffsetByControllerProfile && !NewControllerProfileEvent_Handle.IsValid())
	{
		GetCurrentProfileTransform(true);
	}

	if (!HandSkeletalAction.ActionName.IsEmpty())
	{
		bool bGetCompressedTransforms = false;
		if (bReplicateSkeletalData)
		{
			SkeletalNetUpdateCount += DeltaTime;
			if (SkeletalNetUpdateCount >= (1.0f / ReplicationRateForSkeletalAnimations))
			{
				SkeletalNetUpdateCount = 0.0f;
				bGetCompressedTransforms = true;
			}
		}

		UOpenInputFunctionLibrary::GetActionPose(HandSkeletalAction, this, bGetSkeletalTransforms_WithController, bGetCompressedTransforms);

		if (GetNetMode() == NM_Client/* && !IsTornOff()*/)
		{
			// Need to htz limit this
			if (bGetCompressedTransforms && bReplicateSkeletalData && HandSkeletalAction.CompressedTransforms.Num() > 0)
				Server_SendSkeletalTransforms(HandSkeletalAction);
		}
	}


	if (bDetectGestures && GesturesDB != nullptr && GesturesDB->Gestures.Num() > 0)
	{
		if (HandSkeletalAction.bHasValidData)
			DetectCurrentPose(HandSkeletalAction, LastHandGesture, bIsForRightHand);
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}