// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "PXR_DPManager.h"
#include "Templates/IsUEnumClass.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ReflectedTypeAccessors.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef PICOXRDPHMD_PXR_DPManager_generated_h
#error "PXR_DPManager.generated.h already included, missing '#pragma once' in PXR_DPManager.h"
#endif
#define PICOXRDPHMD_PXR_DPManager_generated_h

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPManager_h


#define FOREACH_ENUM_EEYESIDE(op) \
	op(EEyeSide::Left) \
	op(EEyeSide::Right) \
	op(EEyeSide::Both) 

enum class EEyeSide : uint8;
template<> struct TIsUEnumClass<EEyeSide> { enum { Value = true }; };
template<> PICOXRDPHMD_API UEnum* StaticEnum<EEyeSide>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
