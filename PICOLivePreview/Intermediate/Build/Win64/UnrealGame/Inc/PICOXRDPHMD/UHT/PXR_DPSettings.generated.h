// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "PXR_DPSettings.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef PICOXRDPHMD_PXR_DPSettings_generated_h
#error "PXR_DPSettings.generated.h already included, missing '#pragma once' in PXR_DPSettings.h"
#endif
#define PICOXRDPHMD_PXR_DPSettings_generated_h

#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_SPARSE_DATA
#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_SPARSE_DATA_PROPERTY_ACCESSORS
#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_EDITOR_ONLY_SPARSE_DATA_PROPERTY_ACCESSORS
#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_RPC_WRAPPERS
#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_ACCESSORS
#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_INCLASS \
private: \
	static void StaticRegisterNativesUPICOXRDPSettings(); \
	friend struct Z_Construct_UClass_UPICOXRDPSettings_Statics; \
public: \
	DECLARE_CLASS(UPICOXRDPSettings, UObject, COMPILED_IN_FLAGS(0 | CLASS_DefaultConfig | CLASS_Config), CASTCLASS_None, TEXT("/Script/PICOXRDPHMD"), NO_API) \
	DECLARE_SERIALIZER(UPICOXRDPSettings) \
	static const TCHAR* StaticConfigName() {return TEXT("Engine");} \



#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UPICOXRDPSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UPICOXRDPSettings) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UPICOXRDPSettings); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UPICOXRDPSettings); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UPICOXRDPSettings(UPICOXRDPSettings&&); \
	NO_API UPICOXRDPSettings(const UPICOXRDPSettings&); \
public: \
	NO_API virtual ~UPICOXRDPSettings();


#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_20_PROLOG
#define FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_SPARSE_DATA \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_SPARSE_DATA_PROPERTY_ACCESSORS \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_EDITOR_ONLY_SPARSE_DATA_PROPERTY_ACCESSORS \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_RPC_WRAPPERS \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_ACCESSORS \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_INCLASS \
	FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_23_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> PICOXRDPHMD_API UClass* StaticClass<class UPICOXRDPSettings>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h


#define FOREACH_ENUM_EGRAPHICQUALITY(op) \
	op(EGraphicQuality::High) \
	op(EGraphicQuality::Medium) \
	op(EGraphicQuality::Low) 

enum class EGraphicQuality : uint8;
template<> struct TIsUEnumClass<EGraphicQuality> { enum { Value = true }; };
template<> PICOXRDPHMD_API UEnum* StaticEnum<EGraphicQuality>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
