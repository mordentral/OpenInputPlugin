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
	HandSkeletalAction.SkeletalData.TargetHand = EVRActionHand::EActionHand_Right;
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
	// Don't run this logic if we aren't parented to a controller
	if(!GetAttachParent() || !GetAttachParent()->IsA(UMotionControllerComponent::StaticClass()))
		return;

	// Need to rep this offset to the server and then down to remote clients as well, otherwise it will not perform correctly
	if (bOffsetByControllerProfile)
	{
		UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();

		if (VRSettings == nullptr)
			return;

		FTransform NewControllerProfileTransform = FTransform::Identity;

		if (HandSkeletalAction.SkeletalData.TargetHand == EVRActionHand::EActionHand_Left || !VRSettings->bUseSeperateHandTransforms)
		{
			NewControllerProfileTransform = VRSettings->CurrentControllerProfileTransform;
		}
		else if (HandSkeletalAction.SkeletalData.TargetHand == EVRActionHand::EActionHand_Right)
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

	if (UOpenInputSkeletalMeshBothHands * OwningMesh = Cast<UOpenInputSkeletalMeshBothHands>(InAnimInstance->GetOwningComponent()))
	{
		if (HandSkeletalActionData.Num() < 2)
		{
			HandSkeletalActionData.Add(OwningMesh->HandSkeletalAction.SkeletalData);
			HandSkeletalActionData.Add(OwningMesh->HandSkeletalActionLeft.SkeletalData);
		}
		else
		{
			HandSkeletalActionData[0] = OwningMesh->HandSkeletalAction.SkeletalData;
			HandSkeletalActionData[1] = OwningMesh->HandSkeletalActionLeft.SkeletalData;
		}
	}
	else if (UOpenInputSkeletalMeshComponent * OwningMesh = Cast<UOpenInputSkeletalMeshComponent>(InAnimInstance->GetOwningComponent()))
	{
		if (HandSkeletalActionData.Num() < 1)
		{
			HandSkeletalActionData.Add(OwningMesh->HandSkeletalAction.SkeletalData);
		}
		else
		{
			HandSkeletalActionData[0] = OwningMesh->HandSkeletalAction.SkeletalData;
		}
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

	if (UMotionControllerComponent * MotionParent = Cast<UMotionControllerComponent>(GetAttachParent()))
	{
		EControllerHand HandType;
		if (!FXRMotionControllerBase::GetHandEnumForSourceName(MotionParent->MotionSource, HandType))
		{
			HandType = EControllerHand::Left;
		}

		if (HandType == EControllerHand::Left || HandType == EControllerHand::AnyHand)
			HandSkeletalAction.SkeletalData.TargetHand = EVRActionHand::EActionHand_Left;
		else
			HandSkeletalAction.SkeletalData.TargetHand = EVRActionHand::EActionHand_Right;

	}

	switch (HandSkeletalAction.SkeletalData.TargetHand)
	{
	case EVRActionHand::EActionHand_Left:
	{
		if (HandSkeletalAction.ActionName.IsEmpty())
			HandSkeletalAction.ActionName = FString("/actions/main/in/lefthand_skeleton");
	}break;
	case EVRActionHand::EActionHand_Right:
	{
		if (HandSkeletalAction.ActionName.IsEmpty())
			HandSkeletalAction.ActionName = FString("/actions/main/in/righthand_skeleton");
	}break;
	default:break;
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

		UOpenInputFunctionLibrary::GetActionPose(HandSkeletalAction, this, bGetSkeletalTransforms_WithController, bGetCompressedTransforms, GesturesDB != nullptr);

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
			DetectCurrentPose(HandSkeletalAction, LastHandGesture);
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UOpenInputSkeletalMeshBothHands::UOpenInputSkeletalMeshBothHands(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HandSkeletalAction.SkeletalData.TargetHand = EVRActionHand::EActionHand_Right;
	HandSkeletalAction.SkeletalData.bGetTransformsInParentSpace = true;
	HandSkeletalAction.SkeletalData.bAllowDeformingMesh = false;

	HandSkeletalActionLeft.SkeletalData.TargetHand = EVRActionHand::EActionHand_Left;
	HandSkeletalActionLeft.SkeletalData.bGetTransformsInParentSpace = true;
	HandSkeletalActionLeft.SkeletalData.bAllowDeformingMesh = false;
	HandSkeletalActionLeft.SkeletalData.bMirrorHand = true;
}

void UOpenInputSkeletalMeshBothHands::BeginPlay()
{
	if (HandSkeletalAction.ActionName.IsEmpty())
	{
		HandSkeletalAction.SkeletalData.TargetHand = EVRActionHand::EActionHand_Right;
		HandSkeletalAction.ActionName = FString("/actions/main/in/righthand_skeleton");
	}

	if (HandSkeletalActionLeft.ActionName.IsEmpty())
	{
		HandSkeletalActionLeft.SkeletalData.TargetHand = EVRActionHand::EActionHand_Left;
		HandSkeletalActionLeft.ActionName = FString("/actions/main/in/lefthand_skeleton");
	}

	Super::Super::BeginPlay();
}

void UOpenInputSkeletalMeshBothHands::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (!IsLocallyControlled())
	{
		Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
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

		UOpenInputFunctionLibrary::GetActionPose(HandSkeletalAction, this, bGetSkeletalTransforms_WithController, bGetCompressedTransforms, GesturesDB != nullptr);

		if (GetNetMode() == NM_Client/* && !IsTornOff()*/)
		{
			// Need to htz limit this
			if (bGetCompressedTransforms && bReplicateSkeletalData && HandSkeletalAction.CompressedTransforms.Num() > 0)
				Server_SendSkeletalTransforms(HandSkeletalAction);
		}
	}

	if (!HandSkeletalActionLeft.ActionName.IsEmpty())
	{
		bool bGetCompressedTransforms = false;
		/*if (bReplicateSkeletalData)
		{
			SkeletalNetUpdateCount += DeltaTime;
			if (SkeletalNetUpdateCount >= (1.0f / ReplicationRateForSkeletalAnimations))
			{
				SkeletalNetUpdateCount = 0.0f;
				bGetCompressedTransforms = true;
			}
		}*/

		UOpenInputFunctionLibrary::GetActionPose(HandSkeletalActionLeft, this, bGetSkeletalTransforms_WithController, bGetCompressedTransforms, GesturesDB != nullptr);

	/*	if (GetNetMode() == NM_Client)
		{
			// Need to htz limit this
			if (bGetCompressedTransforms && bReplicateSkeletalData && HandSkeletalAction.CompressedTransforms.Num() > 0)
				Server_SendSkeletalTransforms(HandSkeletalAction);
		}*/
	}


	if (bDetectGestures && GesturesDB != nullptr && GesturesDB->Gestures.Num() > 0)
	{
		if (HandSkeletalAction.bHasValidData)
			DetectCurrentPose(HandSkeletalAction, LastHandGesture);
		if (HandSkeletalActionLeft.bHasValidData)
			DetectCurrentPose(HandSkeletalActionLeft, LastHandGesture);
	}

	Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}