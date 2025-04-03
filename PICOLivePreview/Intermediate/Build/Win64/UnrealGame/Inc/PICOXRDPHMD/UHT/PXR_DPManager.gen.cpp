// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "../../PICOXRDPHMD/Public/PXR_DPManager.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePXR_DPManager() {}
// Cross Module References
	PICOXRDPHMD_API UEnum* Z_Construct_UEnum_PICOXRDPHMD_EEyeSide();
	UPackage* Z_Construct_UPackage__Script_PICOXRDPHMD();
// End Cross Module References
	static FEnumRegistrationInfo Z_Registration_Info_UEnum_EEyeSide;
	static UEnum* EEyeSide_StaticEnum()
	{
		if (!Z_Registration_Info_UEnum_EEyeSide.OuterSingleton)
		{
			Z_Registration_Info_UEnum_EEyeSide.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_PICOXRDPHMD_EEyeSide, (UObject*)Z_Construct_UPackage__Script_PICOXRDPHMD(), TEXT("EEyeSide"));
		}
		return Z_Registration_Info_UEnum_EEyeSide.OuterSingleton;
	}
	template<> PICOXRDPHMD_API UEnum* StaticEnum<EEyeSide>()
	{
		return EEyeSide_StaticEnum();
	}
	struct Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics
	{
		static const UECodeGen_Private::FEnumeratorParam Enumerators[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[];
#endif
		static const UECodeGen_Private::FEnumParams EnumParams;
	};
	const UECodeGen_Private::FEnumeratorParam Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::Enumerators[] = {
		{ "EEyeSide::Left", (int64)EEyeSide::Left },
		{ "EEyeSide::Right", (int64)EEyeSide::Right },
		{ "EEyeSide::Both", (int64)EEyeSide::Both },
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::Enum_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Both.Name", "EEyeSide::Both" },
		{ "Left.Name", "EEyeSide::Left" },
		{ "ModuleRelativePath", "Public/PXR_DPManager.h" },
		{ "Right.Name", "EEyeSide::Right" },
	};
#endif
	const UECodeGen_Private::FEnumParams Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::EnumParams = {
		(UObject*(*)())Z_Construct_UPackage__Script_PICOXRDPHMD,
		nullptr,
		"EEyeSide",
		"EEyeSide",
		Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::Enumerators,
		RF_Public|RF_Transient|RF_MarkAsNative,
		UE_ARRAY_COUNT(Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::Enumerators),
		EEnumFlags::None,
		(uint8)UEnum::ECppForm::EnumClass,
		METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::Enum_MetaDataParams), Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::Enum_MetaDataParams)
	};
	UEnum* Z_Construct_UEnum_PICOXRDPHMD_EEyeSide()
	{
		if (!Z_Registration_Info_UEnum_EEyeSide.InnerSingleton)
		{
			UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_EEyeSide.InnerSingleton, Z_Construct_UEnum_PICOXRDPHMD_EEyeSide_Statics::EnumParams);
		}
		return Z_Registration_Info_UEnum_EEyeSide.InnerSingleton;
	}
	struct Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPManager_h_Statics
	{
		static const FEnumRegisterCompiledInInfo EnumInfo[];
	};
	const FEnumRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPManager_h_Statics::EnumInfo[] = {
		{ EEyeSide_StaticEnum, TEXT("EEyeSide"), &Z_Registration_Info_UEnum_EEyeSide, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 846390184U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPManager_h_687684751(TEXT("/Script/PICOXRDPHMD"),
		nullptr, 0,
		nullptr, 0,
		Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPManager_h_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPManager_h_Statics::EnumInfo));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
