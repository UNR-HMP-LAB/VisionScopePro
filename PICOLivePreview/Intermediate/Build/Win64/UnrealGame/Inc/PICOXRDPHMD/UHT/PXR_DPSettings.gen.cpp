// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "../../PICOXRDPHMD/Public/PXR_DPSettings.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePXR_DPSettings() {}
// Cross Module References
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	PICOXRDPHMD_API UClass* Z_Construct_UClass_UPICOXRDPSettings();
	PICOXRDPHMD_API UClass* Z_Construct_UClass_UPICOXRDPSettings_NoRegister();
	PICOXRDPHMD_API UEnum* Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality();
	UPackage* Z_Construct_UPackage__Script_PICOXRDPHMD();
// End Cross Module References
	static FEnumRegistrationInfo Z_Registration_Info_UEnum_EGraphicQuality;
	static UEnum* EGraphicQuality_StaticEnum()
	{
		if (!Z_Registration_Info_UEnum_EGraphicQuality.OuterSingleton)
		{
			Z_Registration_Info_UEnum_EGraphicQuality.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality, (UObject*)Z_Construct_UPackage__Script_PICOXRDPHMD(), TEXT("EGraphicQuality"));
		}
		return Z_Registration_Info_UEnum_EGraphicQuality.OuterSingleton;
	}
	template<> PICOXRDPHMD_API UEnum* StaticEnum<EGraphicQuality>()
	{
		return EGraphicQuality_StaticEnum();
	}
	struct Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics
	{
		static const UECodeGen_Private::FEnumeratorParam Enumerators[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[];
#endif
		static const UECodeGen_Private::FEnumParams EnumParams;
	};
	const UECodeGen_Private::FEnumeratorParam Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::Enumerators[] = {
		{ "EGraphicQuality::High", (int64)EGraphicQuality::High },
		{ "EGraphicQuality::Medium", (int64)EGraphicQuality::Medium },
		{ "EGraphicQuality::Low", (int64)EGraphicQuality::Low },
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::Enum_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "High.Name", "EGraphicQuality::High" },
		{ "Low.Name", "EGraphicQuality::Low" },
		{ "Medium.Name", "EGraphicQuality::Medium" },
		{ "ModuleRelativePath", "Public/PXR_DPSettings.h" },
	};
#endif
	const UECodeGen_Private::FEnumParams Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::EnumParams = {
		(UObject*(*)())Z_Construct_UPackage__Script_PICOXRDPHMD,
		nullptr,
		"EGraphicQuality",
		"EGraphicQuality",
		Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::Enumerators,
		RF_Public|RF_Transient|RF_MarkAsNative,
		UE_ARRAY_COUNT(Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::Enumerators),
		EEnumFlags::None,
		(uint8)UEnum::ECppForm::EnumClass,
		METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::Enum_MetaDataParams), Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::Enum_MetaDataParams)
	};
	UEnum* Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality()
	{
		if (!Z_Registration_Info_UEnum_EGraphicQuality.InnerSingleton)
		{
			UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_EGraphicQuality.InnerSingleton, Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality_Statics::EnumParams);
		}
		return Z_Registration_Info_UEnum_EGraphicQuality.InnerSingleton;
	}
	void UPICOXRDPSettings::StaticRegisterNativesUPICOXRDPSettings()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UPICOXRDPSettings);
	UClass* Z_Construct_UClass_UPICOXRDPSettings_NoRegister()
	{
		return UPICOXRDPSettings::StaticClass();
	}
	struct Z_Construct_UClass_UPICOXRDPSettings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const UECodeGen_Private::FBytePropertyParams NewProp_GraphicQuality_Underlying;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_GraphicQuality_MetaData[];
#endif
		static const UECodeGen_Private::FEnumPropertyParams NewProp_GraphicQuality;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UPICOXRDPSettings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_PICOXRDPHMD,
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UPICOXRDPSettings_Statics::DependentSingletons) < 16);
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPICOXRDPSettings_Statics::Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "PXR_DPSettings.h" },
		{ "ModuleRelativePath", "Public/PXR_DPSettings.h" },
	};
#endif
	const UECodeGen_Private::FBytePropertyParams Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, nullptr, METADATA_PARAMS(0, nullptr) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality_MetaData[] = {
		{ "Category", "PICOXR LivePreview Graphic" },
		{ "ConfigRestartRequired", "TRUE" },
		{ "DisplayName", "Display Graphic Quality" },
		{ "ModuleRelativePath", "Public/PXR_DPSettings.h" },
	};
#endif
	const UECodeGen_Private::FEnumPropertyParams Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality = { "GraphicQuality", nullptr, (EPropertyFlags)0x0010000000004001, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UPICOXRDPSettings, GraphicQuality), Z_Construct_UEnum_PICOXRDPHMD_EGraphicQuality, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality_MetaData), Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality_MetaData) }; // 2330006668
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UPICOXRDPSettings_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality_Underlying,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPICOXRDPSettings_Statics::NewProp_GraphicQuality,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UPICOXRDPSettings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UPICOXRDPSettings>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UPICOXRDPSettings_Statics::ClassParams = {
		&UPICOXRDPSettings::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UPICOXRDPSettings_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_UPICOXRDPSettings_Statics::PropPointers),
		0,
		0x001000A6u,
		METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UPICOXRDPSettings_Statics::Class_MetaDataParams), Z_Construct_UClass_UPICOXRDPSettings_Statics::Class_MetaDataParams)
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UPICOXRDPSettings_Statics::PropPointers) < 2048);
	UClass* Z_Construct_UClass_UPICOXRDPSettings()
	{
		if (!Z_Registration_Info_UClass_UPICOXRDPSettings.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UPICOXRDPSettings.OuterSingleton, Z_Construct_UClass_UPICOXRDPSettings_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UPICOXRDPSettings.OuterSingleton;
	}
	template<> PICOXRDPHMD_API UClass* StaticClass<UPICOXRDPSettings>()
	{
		return UPICOXRDPSettings::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UPICOXRDPSettings);
	UPICOXRDPSettings::~UPICOXRDPSettings() {}
	struct Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics
	{
		static const FEnumRegisterCompiledInInfo EnumInfo[];
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FEnumRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics::EnumInfo[] = {
		{ EGraphicQuality_StaticEnum, TEXT("EGraphicQuality"), &Z_Registration_Info_UEnum_EGraphicQuality, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 2330006668U) },
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UPICOXRDPSettings, UPICOXRDPSettings::StaticClass, TEXT("UPICOXRDPSettings"), &Z_Registration_Info_UClass_UPICOXRDPSettings, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UPICOXRDPSettings), 1184133551U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_832237571(TEXT("/Script/PICOXRDPHMD"),
		Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics::ClassInfo),
		nullptr, 0,
		Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_tavak_Documents_Unreal_Projects_MyProject2VR2PICO_Plugins_PICOLivePreview_Source_PICOXRDPHMD_Public_PXR_DPSettings_h_Statics::EnumInfo));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
