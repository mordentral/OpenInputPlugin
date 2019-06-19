// Fill out your copyright notice in the Description page of Project Settings.
#include "OpenInputFunctionLibrary.h"
//#include "EngineMinimal.h"
#include "Engine/Engine.h"
#include "CoreMinimal.h"
//#include "IXRTrackingSystem.h"
//#include "IHeadMountedDisplay.h"


//General Log
//DEFINE_LOG_CATEGORY(OpenVRExpansionFunctionLibraryLog);

UOpenInputFunctionLibrary::UOpenInputFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

//=============================================================================
UOpenInputFunctionLibrary::~UOpenInputFunctionLibrary()
{
#if STEAMVR_SUPPORTED_PLATFORM
	//if(VRGetGenericInterfaceFn)
	//	UnloadOpenVRModule();
#endif
}
