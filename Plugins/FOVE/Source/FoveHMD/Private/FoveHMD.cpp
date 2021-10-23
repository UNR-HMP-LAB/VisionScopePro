#include "FoveHMD.h"
#include "FoveHMDPrivatePCH.h"
#include "Core.h"
#include "Engine.h"
#include "FoveVRFunctionLibrary.h"
#include "FoveVRSettings.h"
#include "IFoveHMDPlugin.h"
#include "Interfaces/IPluginManager.h"
#include "PostProcessParameters.h"
#include "PostProcess/PostProcessHMD.h"
#include "RendererPrivate.h"
#include "ScenePrivate.h"
#include "Slate/SceneViewport.h"
#include "FoveAPI.h"

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20
#include "XRRenderBridge.h"
#define FOVEHMD_RENDERING_BRIDGE_BASE_CLASS FXRRenderBridge
#else
#define FOVEHMD_RENDERING_BRIDGE_BASE_CLASS FRHICustomPresent
#endif

// Define or include the LogHMD category, depending on whether we are in Unreal 4.17+ or not
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 17
#include "Logging/LogCategory.h"
#else
DEFINE_LOG_CATEGORY_STATIC(LogHMD, Log, All);
#endif

// 4.16+ uses PipelineStateCache
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 16
#define FOVE_USE_PIPLINE_STATE_CACHE
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#endif

#if WITH_EDITOR
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#endif

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <d3d11.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

#if PLATFORM_MAC
#include <Metal/Metal.h>
#else
#include "IHeadMountedDisplayVulkanExtensions.h"
#include "VulkanRHIPrivate.h"
#include "VulkanPendingState.h"
#include "VulkanContext.h"
#include "VulkanDynamicRHI.h"
#endif

#define LOCTEXT_NAMESPACE "FFoveHMD"

//---------------------------------------------------
// Helpers
//---------------------------------------------------

#ifdef _MSC_VER
#pragma region Helpers
#else
#pragma mark Helpers
#endif

static FQuat ToUnreal(const Fove::Quaternion Quat)
{
	return FQuat{Quat.z, Quat.x, Quat.y, Quat.w};
}

static FVector ToUnreal(const Fove::Vec3 Vec, const float Scale)
{
	return FVector{Vec.z * Scale, Vec.x * Scale, Vec.y * Scale};
}

static FVector2D ToUnreal(const Fove::Vec2 Vec)
{
	return FVector2D{Vec.x, Vec.y};
}

static FTransform ToUnreal(const Fove::Pose& Pose, const float Scale)
{
	FQuat FoveOrientation = ToUnreal(Pose.orientation);
	FVector FovePosition = ToUnreal(Pose.position, Scale);
	return FTransform{FoveOrientation, FovePosition};
}

static FMatrix ToUnreal(const Fove::Matrix44& M)
{
	return FMatrix{
		FPlane(M.mat[0][0], M.mat[1][0], M.mat[2][0], M.mat[3][0]),
		FPlane(M.mat[0][1], M.mat[1][1], M.mat[2][1], M.mat[3][1]),
		FPlane(M.mat[0][2], M.mat[1][2], M.mat[2][2], M.mat[3][2]),
		FPlane(M.mat[0][3], M.mat[1][3], M.mat[2][3], M.mat[3][3])};
}

static EFoveErrorCode ToUnreal(const Fove::ErrorCode& Err)
{
	switch (Err)
	{
	case Fove::ErrorCode::None:
		return EFoveErrorCode::None;
	case Fove::ErrorCode::Connect_NotConnected:
		return EFoveErrorCode::Connect_NotConnected;
	case Fove::ErrorCode::Connect_RuntimeVersionTooOld:
		return EFoveErrorCode::Connect_RuntimeVersionTooOld;
	case Fove::ErrorCode::Connect_ClientVersionTooOld:
		return EFoveErrorCode::Connect_ClientVersionTooOld;
	case Fove::ErrorCode::API_InvalidArgument:
		return EFoveErrorCode::API_InvalidArgument;
	case Fove::ErrorCode::API_NotRegistered:
		return EFoveErrorCode::API_NotRegistered;
	case Fove::ErrorCode::API_NullInPointer:
		return EFoveErrorCode::API_NullInPointer;
	case Fove::ErrorCode::API_InvalidEnumValue:
		return EFoveErrorCode::API_InvalidEnumValue;
	case Fove::ErrorCode::API_NullOutPointersOnly:
		return EFoveErrorCode::API_NullOutPointersOnly;
	case Fove::ErrorCode::API_OverlappingOutPointers:
		return EFoveErrorCode::API_OverlappingOutPointers;
	case Fove::ErrorCode::API_MissingArgument:
		return EFoveErrorCode::API_MissingArgument;
	case Fove::ErrorCode::API_Timeout:
		return EFoveErrorCode::API_Timeout;
	case Fove::ErrorCode::Data_Unreadable:
		return EFoveErrorCode::Data_Unreadable;
	case Fove::ErrorCode::Data_NoUpdate:
		return EFoveErrorCode::Data_NoUpdate;
	case Fove::ErrorCode::Data_Uncalibrated:
		return EFoveErrorCode::Data_Uncalibrated;
	case Fove::ErrorCode::Data_Unreliable:
		return EFoveErrorCode::Data_Unreliable;
	case Fove::ErrorCode::Data_LowAccuracy:
		return EFoveErrorCode::Data_LowAccuracy;
	case Fove::ErrorCode::Hardware_Disconnected:
		return EFoveErrorCode::Hardware_Disconnected;
	case Fove::ErrorCode::Hardware_WrongFirmwareVersion:
		return EFoveErrorCode::Hardware_WrongFirmwareVersion;
	case Fove::ErrorCode::Code_NotImplementedYet:
		return EFoveErrorCode::Code_NotImplementedYet;
	case Fove::ErrorCode::Code_FunctionDeprecated:
		return EFoveErrorCode::Code_FunctionDeprecated;
	case Fove::ErrorCode::Position_ObjectNotTracked:
		return EFoveErrorCode::Position_ObjectNotTracked;
	case Fove::ErrorCode::Compositor_NotSwapped:
		return EFoveErrorCode::Compositor_NotSwapped;
	case Fove::ErrorCode::Compositor_UnableToCreateDeviceAndContext:
		return EFoveErrorCode::Compositor_UnableToCreateDeviceAndContext;
	case Fove::ErrorCode::Compositor_UnableToUseTexture:
		return EFoveErrorCode::Compositor_UnableToUseTexture;
	case Fove::ErrorCode::Compositor_DeviceMismatch:
		return EFoveErrorCode::Compositor_DeviceMismatch;
	case Fove::ErrorCode::Compositor_DisconnectedFromRuntime:
		return EFoveErrorCode::Compositor_DisconnectedFromRuntime;
	case Fove::ErrorCode::Compositor_ErrorCreatingTexturesOnDevice:
		return EFoveErrorCode::Compositor_ErrorCreatingTexturesOnDevice;
	case Fove::ErrorCode::Compositor_NoEyeSpecifiedForSubmit:
		return EFoveErrorCode::Compositor_NoEyeSpecifiedForSubmit;
	case Fove::ErrorCode::UnknownError:
		return EFoveErrorCode::UnknownError;
	case Fove::ErrorCode::Object_AlreadyRegistered:
		return EFoveErrorCode::Object_AlreadyRegistered;
	case Fove::ErrorCode::Calibration_OtherRendererPrioritized:
		return EFoveErrorCode::Calibration_OtherRendererPrioritized;
	case Fove::ErrorCode::License_FeatureAccessDenied:
		return EFoveErrorCode::License_FeatureAccessDenied;
	case Fove::ErrorCode::Profile_DoesntExist:
		return EFoveErrorCode::Profile_DoesntExist;
	case Fove::ErrorCode::Profile_NotAvailable:
		return EFoveErrorCode::Profile_NotAvailable;
	case Fove::ErrorCode::Profile_InvalidName:
		return EFoveErrorCode::Profile_InvalidName;
	case Fove::ErrorCode::Config_DoesntExist:
		return EFoveErrorCode::Config_DoesntExist;
	case Fove::ErrorCode::Config_TypeMismatch:
		return EFoveErrorCode::Config_TypeMismatch;
	};

	UE_LOG(LogHMD, Warning, TEXT("Invalid Fove::ErrorCode: %d"), static_cast<int>(Err));
	return EFoveErrorCode::Unreal_UnknownError;
}

static EFoveCalibrationState ToUnreal(const Fove::CalibrationState State)
{
	switch (State)
	{
	case Fove::CalibrationState::NotStarted: return EFoveCalibrationState::NotStarted;
	case Fove::CalibrationState::HeadsetAdjustment: return EFoveCalibrationState::HeadsetAdjustment;
	case Fove::CalibrationState::WaitingForUser: return EFoveCalibrationState::WaitingForUser;
	case Fove::CalibrationState::CollectingData: return EFoveCalibrationState::CollectingData;
	case Fove::CalibrationState::ProcessingData: return EFoveCalibrationState::ProcessingData;
	case Fove::CalibrationState::Successful_HighQuality: return EFoveCalibrationState::Successful_HighQuality;
	case Fove::CalibrationState::Successful_MediumQuality: return EFoveCalibrationState::Successful_MediumQuality;
	case Fove::CalibrationState::Successful_LowQuality: return EFoveCalibrationState::Successful_LowQuality;
	case Fove::CalibrationState::Failed_Unknown: return EFoveCalibrationState::Failed_Unknown;
	case Fove::CalibrationState::Failed_InaccurateData: return EFoveCalibrationState::Failed_InaccurateData;
	case Fove::CalibrationState::Failed_NoUser: return EFoveCalibrationState::Failed_NoUser;
	case Fove::CalibrationState::Failed_Aborted: return EFoveCalibrationState::Failed_Aborted;
	};

	UE_LOG(LogHMD, Error, TEXT("Invalid Fove::CalibrationState: %d"), static_cast<int>(State));
	return EFoveCalibrationState::Failed_Unknown;
}

static EFoveGazeCastPolicy ToUnreal(const Fove::GazeCastPolicy Policy)
{
	switch (Policy)
	{
	case Fove::GazeCastPolicy::DismissBothEyeClosed: return EFoveGazeCastPolicy::DismissBothEyeClosed;
	case Fove::GazeCastPolicy::DismissOneEyeClosed: return EFoveGazeCastPolicy::DismissOneEyeClosed;
	case Fove::GazeCastPolicy::NeverDismiss: return EFoveGazeCastPolicy::NeverDismiss;
	};

	UE_LOG(LogHMD, Warning, TEXT("Invalid Fove::GazeCastPolicy: %d"), static_cast<int>(Policy));
	return EFoveGazeCastPolicy::DismissBothEyeClosed;
}

static FFoveProjectionParams ToUnreal(const Fove::ProjectionParams Params)
{
	FFoveProjectionParams Ret;
	Ret.Left = Params.left;
	Ret.Right = Params.right;
	Ret.Top = Params.top;
	Ret.Bottom = Params.bottom;
	return Ret;
}

static EFoveCalibrationMethod ToUnreal(const Fove::CalibrationMethod Method)
{
	switch (Method)
	{
	case Fove::CalibrationMethod::Default: return EFoveCalibrationMethod::Default;
	case Fove::CalibrationMethod::OnePoint: return EFoveCalibrationMethod::OnePoint;
	case Fove::CalibrationMethod::Spiral: return EFoveCalibrationMethod::Spiral;
	case Fove::CalibrationMethod::OnePointWithNoGlassesSpiralWithGlasses: return EFoveCalibrationMethod::OnePointWithNoGlassesSpiralWithGlasses;
	};

	UE_LOG(LogHMD, Error, TEXT("Invalid Fove::CalibrationMethod: %d"), static_cast<int>(Method));
	return EFoveCalibrationMethod::Default;
}

static FFoveCalibrationTarget ToUnreal(const Fove::CalibrationTarget Target, const float WorldToMetersScale)
{
	FFoveCalibrationTarget Ret;
	Ret.Position = ToUnreal(Target.position, WorldToMetersScale);
	Ret.RecommendedSize = Target.recommendedSize * WorldToMetersScale;
	return Ret;
}

static FFoveFrameTimestamp ToUnreal(Fove::FrameTimestamp& In)
{
	FFoveFrameTimestamp Ret;
	*reinterpret_cast<uint32*>(&Ret.IdLower) = static_cast<uint32>(In.id & 0xFFFFFFFF);
	*reinterpret_cast<uint32*>(&Ret.IdUpper) = static_cast<uint32>((In.id >> 32) & 0xFFFFFFFF);
	*reinterpret_cast<uint32*>(&Ret.TimestampLower) = static_cast<uint32>(In.timestamp & 0xFFFFFFFF);
	*reinterpret_cast<uint32*>(&Ret.TimestampUpper) = static_cast<uint32>((In.timestamp >> 32) & 0xFFFFFFFF);
	return Ret;
}

static FFoveCalibrationData ToUnreal(const Fove::CalibrationData& Data, const float WorldToMetersScale)
{
	FFoveCalibrationData Ret;
	Ret.Method = ToUnreal(Data.method);
	Ret.State = ToUnreal(Data.state);
	Ret.targetL = ToUnreal(Data.targetL, WorldToMetersScale);
	Ret.targetR = ToUnreal(Data.targetR, WorldToMetersScale);
	return Ret;
}

static EFoveEyeState ToUnreal(const Fove::EyeState State)
{
	switch (State)
	{
	case Fove::EyeState::Closed: return EFoveEyeState::Closed;
	case Fove::EyeState::Opened: return EFoveEyeState::Opened;
	case Fove::EyeState::NotDetected: return EFoveEyeState::NotDetected;
	};

	UE_LOG(LogHMD, Error, TEXT("Invalid Fove::EyeState: %d"), static_cast<int>(State));
	return EFoveEyeState::NotDetected;
}

static Fove::Quaternion ToFove(const FQuat Quat)
{
	return Fove::Quaternion{Quat.Y, Quat.X, Quat.Z, Quat.W};
}

static Fove::Vec3 ToFove(const FVector Vec, const float Scale)
{
	return Fove::Vec3{Vec.Y * Scale, Vec.Z * Scale, Vec.X * Scale};
}

static Fove::Eye ToFove(const EFoveEye Eye)
{
	switch (Eye)
	{
	case EFoveEye::Left: return Fove::Eye::Left;
	case EFoveEye::Right: return Fove::Eye::Right;
	};

	UE_LOG(LogHMD, Warning, TEXT("Invalid EFoveEye: %d"), static_cast<int>(Eye));
	return Fove::Eye::Left;
}

static Fove::CalibrationMethod ToFove(const EFoveCalibrationMethod Method)
{
	switch (Method)
	{
	case EFoveCalibrationMethod::Default: return Fove::CalibrationMethod::Default;
	case EFoveCalibrationMethod::OnePoint: return Fove::CalibrationMethod::OnePoint;
	case EFoveCalibrationMethod::Spiral: return Fove::CalibrationMethod::Spiral;
	case EFoveCalibrationMethod::OnePointWithNoGlassesSpiralWithGlasses: return Fove::CalibrationMethod::OnePointWithNoGlassesSpiralWithGlasses;
	};

	UE_LOG(LogHMD, Error, TEXT("Invalid EFoveCalibrationMethod: %d"), static_cast<int>(Method));
	return Fove::CalibrationMethod::Default;
}

static Fove::EyeByEyeCalibration ToFove(const EFoveEyeByEyeCalibration Method)
{
	switch (Method)
	{
	case EFoveEyeByEyeCalibration::Default: return Fove::EyeByEyeCalibration::Default;
	case EFoveEyeByEyeCalibration::Disabled: return Fove::EyeByEyeCalibration::Disabled;
	case EFoveEyeByEyeCalibration::Enabled: return Fove::EyeByEyeCalibration::Enabled;
	};

	UE_LOG(LogHMD, Warning, TEXT("Invalid EFoveEyeByEyeCalibration: %d"), static_cast<int>(Method));
	return Fove::EyeByEyeCalibration::Default;
}

static Fove::ObjectPose ToFove(const FFoveObjectPose Pose, const float WorldToMetersScale)
{
	Fove::ObjectPose Ret;
	Ret.scale = ToFove(Pose.Scale, 1.0f);
	Ret.rotation = ToFove(Pose.Rotation);
	Ret.position = ToFove(Pose.Position, WorldToMetersScale);
	Ret.velocity = ToFove(Pose.Position, WorldToMetersScale);
	return Ret;
}

static Fove::LogLevel ToFove(const EFoveLogLevel Level)
{
	switch (Level)
	{
	case EFoveLogLevel::Debug:
		return Fove::LogLevel::Debug;
	case EFoveLogLevel::Warning:
		return Fove::LogLevel::Warning;
	case EFoveLogLevel::Error:
		return Fove::LogLevel::Error;
	};

	UE_LOG(LogHMD, Error, TEXT("Invalid EFoveLogLevel: %d"), static_cast<int>(Level));
	return Fove::LogLevel::Error;
}

static Fove::GazeCastPolicy ToFove(const EFoveGazeCastPolicy Policy)
{
	if (Policy == EFoveGazeCastPolicy::DismissBothEyeClosed)
		return Fove::GazeCastPolicy::DismissBothEyeClosed;
	if (Policy == EFoveGazeCastPolicy::DismissOneEyeClosed)
		return Fove::GazeCastPolicy::DismissOneEyeClosed;
	if (Policy == EFoveGazeCastPolicy::NeverDismiss)
		return Fove::GazeCastPolicy::NeverDismiss;

	UE_LOG(LogHMD, Error, TEXT("Invalid EFoveGazeCastPolicy: %d"), static_cast<int>(Policy));
	return Fove::GazeCastPolicy::DismissBothEyeClosed;
}

static Fove::ObjectGroup ToFove(EFoveObjectGroup Group)
{
	switch (Group)
	{
	case EFoveObjectGroup::Group0: return Fove::ObjectGroup::Group0;
	case EFoveObjectGroup::Group1: return Fove::ObjectGroup::Group1;
	case EFoveObjectGroup::Group2: return Fove::ObjectGroup::Group2;
	case EFoveObjectGroup::Group3: return Fove::ObjectGroup::Group3;
	case EFoveObjectGroup::Group4: return Fove::ObjectGroup::Group4;
	case EFoveObjectGroup::Group5: return Fove::ObjectGroup::Group5;
	case EFoveObjectGroup::Group6: return Fove::ObjectGroup::Group6;
	case EFoveObjectGroup::Group7: return Fove::ObjectGroup::Group7;
	case EFoveObjectGroup::Group8: return Fove::ObjectGroup::Group8;
	case EFoveObjectGroup::Group9: return Fove::ObjectGroup::Group9;
	case EFoveObjectGroup::Group10: return Fove::ObjectGroup::Group10;
	case EFoveObjectGroup::Group11: return Fove::ObjectGroup::Group11;
	case EFoveObjectGroup::Group12: return Fove::ObjectGroup::Group12;
	case EFoveObjectGroup::Group13: return Fove::ObjectGroup::Group13;
	case EFoveObjectGroup::Group14: return Fove::ObjectGroup::Group14;
	case EFoveObjectGroup::Group15: return Fove::ObjectGroup::Group15;
	case EFoveObjectGroup::Group16: return Fove::ObjectGroup::Group16;
	case EFoveObjectGroup::Group17: return Fove::ObjectGroup::Group17;
	case EFoveObjectGroup::Group18: return Fove::ObjectGroup::Group18;
	case EFoveObjectGroup::Group19: return Fove::ObjectGroup::Group19;
	case EFoveObjectGroup::Group20: return Fove::ObjectGroup::Group20;
	case EFoveObjectGroup::Group21: return Fove::ObjectGroup::Group21;
	case EFoveObjectGroup::Group22: return Fove::ObjectGroup::Group22;
	case EFoveObjectGroup::Group23: return Fove::ObjectGroup::Group23;
	case EFoveObjectGroup::Group24: return Fove::ObjectGroup::Group24;
	case EFoveObjectGroup::Group25: return Fove::ObjectGroup::Group25;
	case EFoveObjectGroup::Group26: return Fove::ObjectGroup::Group26;
	case EFoveObjectGroup::Group27: return Fove::ObjectGroup::Group27;
	case EFoveObjectGroup::Group28: return Fove::ObjectGroup::Group28;
	case EFoveObjectGroup::Group29: return Fove::ObjectGroup::Group29;
	case EFoveObjectGroup::Group30: return Fove::ObjectGroup::Group30;
	case EFoveObjectGroup::Group31: return Fove::ObjectGroup::Group31;
	};

	UE_LOG(LogHMD, Error, TEXT("Invalid EFoveObjectGroup: %d"), static_cast<int>(Group));
	return Fove::ObjectGroup::Group0;
}

static Fove::CameraObject ToFove(const FFoveCameraObject& Camera, const float WorldToMetersScale)
{
	Fove::CameraObject Ret;
	Ret.groupMask = static_cast<Fove::ObjectGroup>(Camera.GroupMask);
	Ret.id = Camera.Id;
	Ret.pose = ToFove(Camera.Pose, WorldToMetersScale);
	return Ret;
}

static Fove::ErrorCode ToFove(const EFoveErrorCode& Err)
{
	switch (Err)
	{
	case EFoveErrorCode::None:
		return Fove::ErrorCode::None;
	case EFoveErrorCode::Connect_NotConnected:
		return Fove::ErrorCode::Connect_NotConnected;
	case EFoveErrorCode::Connect_RuntimeVersionTooOld:
		return Fove::ErrorCode::Connect_RuntimeVersionTooOld;
	case EFoveErrorCode::Connect_ClientVersionTooOld:
		return Fove::ErrorCode::Connect_ClientVersionTooOld;
	case EFoveErrorCode::API_InvalidArgument:
		return Fove::ErrorCode::API_InvalidArgument;
	case EFoveErrorCode::API_NotRegistered:
		return Fove::ErrorCode::API_NotRegistered;
	case EFoveErrorCode::API_NullInPointer:
		return Fove::ErrorCode::API_NullInPointer;
	case EFoveErrorCode::API_InvalidEnumValue:
		return Fove::ErrorCode::API_InvalidEnumValue;
	case EFoveErrorCode::API_NullOutPointersOnly:
		return Fove::ErrorCode::API_NullOutPointersOnly;
	case EFoveErrorCode::API_OverlappingOutPointers:
		return Fove::ErrorCode::API_OverlappingOutPointers;
	case EFoveErrorCode::API_MissingArgument:
		return Fove::ErrorCode::API_MissingArgument;
	case EFoveErrorCode::API_Timeout:
		return Fove::ErrorCode::API_Timeout;
	case EFoveErrorCode::Data_Unreadable:
		return Fove::ErrorCode::Data_Unreadable;
	case EFoveErrorCode::Data_NoUpdate:
		return Fove::ErrorCode::Data_NoUpdate;
	case EFoveErrorCode::Data_Uncalibrated:
		return Fove::ErrorCode::Data_Uncalibrated;
	case EFoveErrorCode::Data_Unreliable:
		return Fove::ErrorCode::Data_Unreliable;
	case EFoveErrorCode::Data_LowAccuracy:
		return Fove::ErrorCode::Data_LowAccuracy;
	case EFoveErrorCode::Hardware_Disconnected:
		return Fove::ErrorCode::Hardware_Disconnected;
	case EFoveErrorCode::Hardware_WrongFirmwareVersion:
		return Fove::ErrorCode::Hardware_WrongFirmwareVersion;
	case EFoveErrorCode::Code_NotImplementedYet:
		return Fove::ErrorCode::Code_NotImplementedYet;
	case EFoveErrorCode::Code_FunctionDeprecated:
		return Fove::ErrorCode::Code_FunctionDeprecated;
	case EFoveErrorCode::Position_ObjectNotTracked:
		return Fove::ErrorCode::Position_ObjectNotTracked;
	case EFoveErrorCode::Compositor_NotSwapped:
		return Fove::ErrorCode::Compositor_NotSwapped;
	case EFoveErrorCode::Compositor_UnableToCreateDeviceAndContext:
		return Fove::ErrorCode::Compositor_UnableToCreateDeviceAndContext;
	case EFoveErrorCode::Compositor_UnableToUseTexture:
		return Fove::ErrorCode::Compositor_UnableToUseTexture;
	case EFoveErrorCode::Compositor_DeviceMismatch:
		return Fove::ErrorCode::Compositor_DeviceMismatch;
	case EFoveErrorCode::Compositor_DisconnectedFromRuntime:
		return Fove::ErrorCode::Compositor_DisconnectedFromRuntime;
	case EFoveErrorCode::Compositor_ErrorCreatingTexturesOnDevice:
		return Fove::ErrorCode::Compositor_ErrorCreatingTexturesOnDevice;
	case EFoveErrorCode::Compositor_NoEyeSpecifiedForSubmit:
		return Fove::ErrorCode::Compositor_NoEyeSpecifiedForSubmit;
	case EFoveErrorCode::UnknownError:
		return Fove::ErrorCode::UnknownError;
	case EFoveErrorCode::Object_AlreadyRegistered:
		return Fove::ErrorCode::Object_AlreadyRegistered;
	case EFoveErrorCode::Calibration_OtherRendererPrioritized:
		return Fove::ErrorCode::Calibration_OtherRendererPrioritized;
	case EFoveErrorCode::License_FeatureAccessDenied:
		return Fove::ErrorCode::License_FeatureAccessDenied;
	case EFoveErrorCode::Profile_DoesntExist:
		return Fove::ErrorCode::Profile_DoesntExist;
	case EFoveErrorCode::Profile_NotAvailable:
		return Fove::ErrorCode::Profile_NotAvailable;
	case EFoveErrorCode::Profile_InvalidName:
		return Fove::ErrorCode::Profile_InvalidName;
	case EFoveErrorCode::Config_DoesntExist:
		return Fove::ErrorCode::Config_DoesntExist;
	case EFoveErrorCode::Config_TypeMismatch:
		return Fove::ErrorCode::Config_TypeMismatch;
	};

	UE_LOG(LogHMD, Warning, TEXT("Invalid EFoveErrorCode: %d"), static_cast<int>(Err));
	return Fove::ErrorCode::UnknownError;
}

// Helper function to determine if a fove is connected
static bool IsFoveConnected(Fove::Headset& Headset, Fove::Compositor& Compositor)
{
	// Object must be valid
	if (!Headset.getCObject() || !Compositor.getCObject())
		return false;

	// Headset must be plugged in
	const Fove::Result<bool> isHardwareConnected = Headset.isHardwareConnected();
	if (!isHardwareConnected)
	{
		UE_LOG(LogHMD, Warning, TEXT("Headset::isHardwareConnected failed: %d"), static_cast<int>(isHardwareConnected.getError()));
		return false;
	}
	if (!isHardwareConnected.getValue())
		return false;

	// Check if we are connected to the compositor
	// This is an important step because there are potentially other Unreal plugins that support FOVE (such as SteamVR and OSVR)
	// In all cases, the FOVE headset may be connected, but we should only use the FOVE plugin when the FOVE compositor is running
	const Fove::Result<bool> isCompositorReady = Compositor.isReady();
	if (!isCompositorReady)
	{
		UE_LOG(LogHMD, Warning, TEXT("Compositor::isReady failed: %d"), static_cast<int>(isCompositorReady.getError()));
		return false;
	}

	return isCompositorReady.getValue();
}

// Clang (at least for the CentOS cross compile pipeline for UE25) gets the following warning-as-error
// Error: mangled name of 'GenerateTextureHelper' will change in C++17 due to non-throwing exception specification in function signature
// This comes down to the noexcept on the fove_Headset_getEyesImage changing type of the function pointer in C++17
// But this is a static function... we do not care about the name mangling...
#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++17-compat-mangling"
#endif

static bool CreateTransientTextureIfNeeded(UTexture2D*& Out, const TCHAR* const Name, const int Width, const int Height)
{
	UTexture2D* Texture = Out;
	if (!Texture || Texture->GetSizeX() != Width || Texture->GetSizeY() != Height)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
		Out = Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, Name);
#else
		Out = Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
#endif

		if (!Texture)
		{
			UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s, UTexture2D::CreateTransient failed"), Name);
			return false;
		}
		else
			UE_LOG(LogHMD, Log, TEXT("FFoveHMD::%s created transient texture of size %dx%d"), Name, Width, Height);
	}

	return true;
}

// Helper to generate a UE4 texture from a FOVE bitmap
static EFoveErrorCode GenerateTextureHelper(UTexture*& Out, const TCHAR* const Name, const decltype(&fove_Headset_getEyesImage) Func, Fove_Headset* const Headset)
{
	// Fetch the bitmap image from FOVE API
	Fove_BitmapImage Tmp;
	const EFoveErrorCode Ret = ToUnreal((*Func)(Headset, &Tmp));
	if (Ret != EFoveErrorCode::None)
		return Ret;

	// Parse bitmap header
	const uint8* Data = reinterpret_cast<const uint8*>(Tmp.image.data);
	if (Tmp.image.length < 54 || Data[0] != 'B' || Data[1] != 'M')
	{
		UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s bad header"), Name);
		return EFoveErrorCode::Unreal_UnknownError;
	}
	const int32 Width = *reinterpret_cast<const int32*>(Data + 0x12);
	int32 Height = *reinterpret_cast<const int32*>(Data + 0x16);
	if (Width <= 0)
	{
		UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s bad Width: %d"), Name, static_cast<int>(Width));
		return EFoveErrorCode::Unreal_UnknownError;
	}
	if (Height == 0)
	{
		UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s bad Height: %d"), Name, static_cast<int>(Height));
		return EFoveErrorCode::Unreal_UnknownError;
	}
	bool Flip = true;
	if (Height < 0) // Negative height indicates top-down bitmap (like Unreal expects)
	{
		Flip = false;
		Height = -Height;
	}
	const int32 BPP = *reinterpret_cast<const int32*>(Data + 0x1c);
	if (BPP != 24)
	{
		UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s bad BPP: %d"), Name, static_cast<int>(BPP));
		return EFoveErrorCode::Unreal_UnknownError;
	}
	const uint16 NumPlanes = *reinterpret_cast<const uint16*>(Data + 0x1a);
	if (NumPlanes != 1)
	{
		UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s bad number of planes: %d"), Name, static_cast<int>(NumPlanes));
		return EFoveErrorCode::Unreal_UnknownError;
	}
	const int32 ExpectedSize = Width * Height * 3;
	const int32 DataOffset = *reinterpret_cast<const int32*>(Data + 0xa);
	if (Tmp.image.length < DataOffset + ExpectedSize)
	{
		UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s invalid byte size: %d %d %d"), Name, static_cast<int>(Tmp.image.length), static_cast<int>(DataOffset), static_cast<int>(ExpectedSize));
		return EFoveErrorCode::Unreal_UnknownError;
	}

	// Create the UTexture on demand
	// Subsequent calls to this function should be re-passed the previous texture to avoid creation every frame
	UTexture2D* Texture = Cast<UTexture2D>(Out);
	if (!Texture || Texture->GetSizeX() != Width || Texture->GetSizeY() != Height)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
		Out = Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, Name);
#else
		Out = Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
#endif

		if (!Texture)
		{
			UE_LOG(LogHMD, Error, TEXT("FFoveHMD::%s, UTexture2D::CreateTransient failed"), Name);
			return EFoveErrorCode::Unreal_UnknownError;
		}
		else
			UE_LOG(LogHMD, Log, TEXT("FFoveHMD::%s created transient texture"), Name);
	}

	// Update texture
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	uint32* OutData = reinterpret_cast<uint32*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
	for (int32 Row = 0; Row < Height; ++Row) // Simple 24 -> 32 bit conversion
	{
		uint32* const OutRow = OutData + Row * Width;
		const uint8* const InRow = Data + (Flip ? Height - Row - 1 : Row) * Width * 3;
		for (int32 x = 0; x < Width; ++x)
		{
			*(OutRow + x) = static_cast<uint32>(*(InRow + x * 3)) | static_cast<uint32>(*(InRow + x * 3 + 1)) << 8 | static_cast<uint32>(*(InRow + x * 3 + 2)) << 16 | uint32{0xff000000};
		}
	}
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	return EFoveErrorCode::None;
}

#ifdef _MSC_VER
#pragma endregion
#endif

//---------------------------------------------------
// FoveRenderingBridge
//---------------------------------------------------

#ifdef _MSC_VER
#pragma region FoveRenderingBridge
#else
#pragma mark FoveRenderingBridge
#endif

class FoveRenderingBridge : public FOVEHMD_RENDERING_BRIDGE_BASE_CLASS
{
public:
	FoveRenderingBridge(const TSharedRef<Fove::Compositor, ESPMode::ThreadSafe>& compositor)
		: FOVEHMD_RENDERING_BRIDGE_BASE_CLASS(BEFORE_4_20(nullptr))
		, Compositor(compositor)
	{
		CreateCompositorLayer();
	}
	virtual ~FoveRenderingBridge() {}

	void OnBackBufferResize() override {} // Ignored

	const void SetRenderPose(const Fove::Pose& pose, const float WorldToMetersScale)
	{
		FovePose = pose;
		Pose = ToUnreal(FovePose, WorldToMetersScale);
	}

	const FTransform& GetRenderPose() const
	{
		return Pose;
	}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 20 // Declared by FXRRenderBridge base class since 4.20, so for prior versions, we declare it
	virtual void UpdateViewport(const class FViewport& Viewport, class FRHIViewport* InViewportRHI) {}
#endif

	bool Present(int& SyncInterval) override
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 17 // For IsRunningRHIInSeparateThread()
		check(IsRunningRHIInSeparateThread() ? IsInRHIThread() : IsInRenderingThread());
#endif

		// Disable vsync!
		SyncInterval = 0;

		if (!CompositorTex)
		{
			UE_LOG(LogHMD, Warning, TEXT("FOVE present without render texture"));
		}
		else if (!IsCompositorLayerValid())
		{
			UE_LOG(LogHMD, Warning, TEXT("FOVE Plugin waiting for compositor layer to be created (compositor is not running?)"));
		}
		else
		{
			Fove::CompositorLayerSubmitInfo info;
			info.layerId = FoveCompositorLayer.layerId;
			info.pose = FovePose;
			info.left.texInfo = CompositorTex;
			info.right.texInfo = CompositorTex;
			info.left.bounds.left = 0.0f;
			info.left.bounds.right = 0.5f;
			info.left.bounds.bottom = 1.0f;
			info.left.bounds.top = 0.0f;
			info.right.bounds.left = 0.5f;
			info.right.bounds.right = 1.0f;
			info.right.bounds.bottom = 1.0f;
			info.right.bounds.top = 0.0f;

			Compositor->submit(info);
		}

		return true;
	}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	bool NeedsNativePresent() override
	{
		return false;
	}
#endif

	void CreateCompositorLayer()
	{
		if (IsCompositorLayerValid())
			return;

		Fove::CompositorLayerCreateInfo LayerCreateInfo;
		// LayerCreateInfo.disableTimeWarp = FoveMode == FoveUnrealPluginMode::FixedToHMDScreen; FIXME
		const Fove::Result<Fove::CompositorLayer> Layer = Compositor->createLayer(LayerCreateInfo);
		if (Layer)
			FoveCompositorLayer = Layer.getValue();
	}

	Fove::CompositorLayer GetCompositorLayer() const
	{
		return FoveCompositorLayer;
	}

	bool IsCompositorLayerValid() const
	{
		return FoveCompositorLayer.idealResolutionPerEye.x > 0 && FoveCompositorLayer.idealResolutionPerEye.y > 0;
	}

protected:
	const TSharedRef<Fove::Compositor, ESPMode::ThreadSafe> Compositor;  // Pointer back to the Fove plugin object that owns us
	Fove::CompositorLayer FoveCompositorLayer;
	Fove::Pose FovePose; // Pose fetched out via WaitForRenderPose, used internally to submit frames back to fove
	FTransform Pose;     // Same as RenderPose, but converted to Unreal coordinates
	Fove::CompositorTexture* CompositorTex = nullptr;
};

#ifdef _MSC_VER
#pragma endregion
#endif

//---------------------------------------------------
// FoveD3D11Bridge
//---------------------------------------------------

#ifdef _MSC_VER
#pragma region FoveD3D11Bridge
#else
#pragma mark FoveD3D11Bridge
#endif

#if PLATFORM_WINDOWS

class FoveD3D11Bridge : public FoveRenderingBridge
{
	ID3D11Texture2D* RenderTargetTexture = nullptr;

public:
	FoveD3D11Bridge(const TSharedRef<Fove::Compositor, ESPMode::ThreadSafe>& Compositor)
		: FoveRenderingBridge(Compositor)
	{
	}

	~FoveD3D11Bridge()
	{
		if (RenderTargetTexture)
			RenderTargetTexture->Release();
	}

	bool Present(int& SyncInterval) override
	{
		// Clear rasterizer state to avoid Unreal messing with FOVE submit
		ID3D11Device* Dev = nullptr;
		ID3D11DeviceContext* Ctx = nullptr;
		ID3D11RasterizerState* RasterizerState = nullptr;
		RenderTargetTexture->GetDevice(&Dev);
		if (Dev)
		{
			Dev->GetImmediateContext(&Ctx);
			if (Ctx)
			{
				D3D11_RASTERIZER_DESC desc = {};
				Ctx->RSGetState(&RasterizerState);
				Ctx->RSSetState(nullptr);
			}
		}

		// Submit eye images
		Fove::DX11Texture tex{RenderTargetTexture};
		CompositorTex = &tex;
		const bool ret = FoveRenderingBridge::Present(SyncInterval);
		CompositorTex = nullptr;

		// Restore state
		if (Ctx)
		{
			D3D11_RASTERIZER_DESC desc = {};
			Ctx->RSSetState(RasterizerState);
		}

		return ret;
	}

	void UpdateViewport(const class FViewport& Viewport, class FRHIViewport* InViewportRHI) override
	{
		check(IsInGameThread());

		// Update render target
		const FTexture2DRHIRef& textureRef = Viewport.GetRenderTargetTexture();
		ID3D11Texture2D* const newRT = textureRef ? (ID3D11Texture2D*)textureRef->GetNativeResource() : nullptr;
		if (newRT != RenderTargetTexture)
		{
			if (RenderTargetTexture)
				RenderTargetTexture->Release();

			RenderTargetTexture = newRT;

			if (RenderTargetTexture)
				RenderTargetTexture->AddRef();
		}
	}
};

#endif // PLATFORM_WINDOWS

#ifdef _MSC_VER
#pragma endregion
#endif

//---------------------------------------------------
// FoveMetalBridge
//---------------------------------------------------

#if PLATFORM_MAC

class FoveMetalBridge : public FoveRenderingBridge
{
	id<MTLTexture> RenderTargetTexture = nullptr;

public:
	FoveMetalBridge(const TSharedRef<Fove::Compositor, ESPMode::ThreadSafe>& Compositor)
		: FoveRenderingBridge(Compositor)
	{
	}

	bool Present(int& SyncInterval) override
	{
		Fove::MetalTexture tex{RenderTargetTexture};
		CompositorTex = &tex;
		const bool ret = FoveRenderingBridge::Present(SyncInterval);
		CompositorTex = nullptr;

		return ret;
	}

	void UpdateViewport(const class FViewport& Viewport, class FRHIViewport* InViewportRHI) override
	{
		check(IsInGameThread());

		const FTexture2DRHIRef& textureRef = Viewport.GetRenderTargetTexture();
		RenderTargetTexture = (id<MTLTexture>)textureRef->GetNativeResource();
	}
};

#endif // PLATFORM_MAC

//---------------------------------------------------
// FoveVulkanBridge
//---------------------------------------------------

#if !PLATFORM_MAC

class FoveVulkanBridge : public FoveRenderingBridge
{
	FVulkanTexture2D* stextureRef = nullptr;

public:
	FoveVulkanBridge(const TSharedRef<Fove::Compositor, ESPMode::ThreadSafe>& Compositor)
		: FoveRenderingBridge(Compositor)
	{
	}

	~FoveVulkanBridge()
	{
	}

	void UpdateViewport(const class FViewport& Viewport, class FRHIViewport* InViewportRHI) override
	{
		check(IsInGameThread());

		stextureRef = static_cast<FVulkanTexture2D*>(Viewport.GetRenderTargetTexture().GetReference());
	}

	bool Present(int& SyncInterval) override
	{
		if (!stextureRef)
			return true;

		// Get info
		FVulkanTexture2D& textureRef = *stextureRef;
		FVulkanDynamicRHI& rhi = *static_cast<FVulkanDynamicRHI*>(GDynamicRHI);

		// Change image layout to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL so we can sample it
		FVulkanCommandListContext& ImmediateContext = rhi.GetDevice()->GetImmediateContext();
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26
		VkImageLayout& CurrentLayout = ImmediateContext.GetLayoutManager().FindOrAddLayoutRW(textureRef.Surface, VK_IMAGE_LAYOUT_UNDEFINED);
#else
		VkImageLayout& CurrentLayout = ImmediateContext.GetTransitionAndLayoutManager().FindOrAddLayoutRW(textureRef.Surface.Image, VK_IMAGE_LAYOUT_UNDEFINED);
#endif
		bool bHadLayout = (CurrentLayout != VK_IMAGE_LAYOUT_UNDEFINED);
		FVulkanCmdBuffer* CmdBuffer = ImmediateContext.GetCommandBufferManager()->GetUploadCmdBuffer();
		VkImageSubresourceRange SubresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		if (CurrentLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			rhi.VulkanSetImageLayout(CmdBuffer->GetHandle(), textureRef.Surface.Image, CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, SubresourceRange);
		}

		// Generate FOVE texture struct
		Fove::VulkanTexture Tex{};
		Tex.context.instance = (Fove_VkInstance)rhi.GetInstance();
		Tex.context.physicalDevice = (Fove_VkPhysicalDevice)rhi.GetDevice()->GetPhysicalHandle();
		Tex.context.device = (Fove_VkDevice)rhi.GetDevice()->GetInstanceHandle();
		Tex.context.graphicsQueue = (Fove_VkQueue)rhi.GetDevice()->GetGraphicsQueue()->GetHandle();
		Tex.context.presentationQueue = (Fove_VkQueue)rhi.GetDevice()->GetPresentQueue()->GetHandle();
		Tex.context.transferQueue = (Fove_VkQueue)rhi.GetDevice()->GetTransferQueue()->GetHandle();
		Tex.context.graphicsQueueFamilyIndex = rhi.GetDevice()->GetGraphicsQueue()->GetFamilyIndex();
		Tex.context.presentationQueueFamilyIndex = rhi.GetDevice()->GetPresentQueue()->GetFamilyIndex();
		Tex.context.transferQueueFamilyIndex = rhi.GetDevice()->GetTransferQueue()->GetFamilyIndex();
		Tex.resources.deviceMemory = (Fove_VkDeviceMemory)textureRef.Surface.GetAllocationHandle();
		Tex.resources.image = (Fove_VkImage)textureRef.GetNativeResource();
		Tex.resources.imageView = (Fove_VkImageView)textureRef.DefaultView.View;
		Tex.width = textureRef.GetSizeX();
		Tex.height = textureRef.GetSizeY();

		// Validate
		if (!Tex.context.instance)
		{
			UE_LOG(LogHMD, Warning, TEXT("No instance"));
		}
		if (!Tex.context.physicalDevice)
		{
			UE_LOG(LogHMD, Warning, TEXT("No physicalDevice"));
		}
		if (!Tex.context.device)
		{
			UE_LOG(LogHMD, Warning, TEXT("No device"));
		}
		if (!Tex.context.graphicsQueue)
		{
			UE_LOG(LogHMD, Warning, TEXT("No graphicsQueue"));
		}
		if (!Tex.context.presentationQueue)
		{
			UE_LOG(LogHMD, Warning, TEXT("No presentationQueue"));
		}
		if (!Tex.context.transferQueue)
		{
			UE_LOG(LogHMD, Warning, TEXT("No transferQueue"));
		}
		if (!Tex.resources.deviceMemory)
		{
			UE_LOG(LogHMD, Warning, TEXT("No deviceMemory"));
		}
		if (!Tex.resources.image)
		{
			UE_LOG(LogHMD, Warning, TEXT("No image"));
		}
		if (!Tex.resources.imageView)
		{
			UE_LOG(LogHMD, Warning, TEXT("No imageView"));
		}

		CompositorTex = Tex.resources.image != nullptr ? &Tex : nullptr;
		const bool ret = FoveRenderingBridge::Present(SyncInterval);
		CompositorTex = nullptr;

		if (bHadLayout && CurrentLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			rhi.VulkanSetImageLayout(CmdBuffer->GetHandle(), textureRef.Surface.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, CurrentLayout, SubresourceRange);
		}
		else
		{
			CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}

		ImmediateContext.GetCommandBufferManager()->SubmitUploadCmdBuffer();

		return ret;
	}
};

#endif // !PLATFORM_MAC

//---------------------------------------------------
// UFoveVRFunctionLibrary
//---------------------------------------------------

#ifdef _MSC_VER
#pragma region UFoveVRFunctionLibrary
#else
#pragma mark UFoveVRFunctionLibrary
#endif

UFoveVRFunctionLibrary::UFoveVRFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFoveVRFunctionLibrary::IsHardwareConnected(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsHardwareConnected(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsHardwareReady(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsHardwareReady(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsMotionReady(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsMotionReady(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::CheckSoftwareVersions(EFoveErrorCode& OutErrorCode)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->CheckSoftwareVersions() : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetSoftwareVersions(EFoveErrorCode& OutErrorCode, FFoveVersions& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetSoftwareVersions(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetHeadsetHardwareInfo(EFoveErrorCode& OutErrorCode, FFoveHeadsetHardwareInfo& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetHeadsetHardwareInfo(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::RegisterCapabilities(EFoveErrorCode& OutErrorCode, const TArray<EFoveClientCapabilities>& Caps)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->RegisterCapabilities(Caps) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::UnregisterCapabilities(EFoveErrorCode& OutErrorCode, const TArray<EFoveClientCapabilities>& Caps)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->UnregisterCapabilities(Caps) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::WaitAndFetchNextEyeTrackingData(EFoveErrorCode& OutErrorCode, FFoveFrameTimestamp& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->WaitAndFetchNextEyeTrackingData(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::FetchEyeTrackingData(EFoveErrorCode& OutErrorCode, FFoveFrameTimestamp& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->FetchEyeTrackingData(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetGazeVector(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FVector& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetGazeVector(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetGazeScreenPosition(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FVector2D& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetGazeScreenPosition(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetCombinedGazeRay(EFoveErrorCode& OutErrorCode, FVector& OutOrigin, FVector& OutDirection)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetCombinedGazeRay(OutOrigin, OutDirection) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetCombinedGazeDepth(EFoveErrorCode& OutErrorCode, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetCombinedGazeDepth(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsUserShiftingAttention(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsUserShiftingAttention(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetEyeState(EFoveErrorCode& OutErrorCode, EFoveEye Eye, EFoveEyeState& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetEyeState(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsEyeTrackingEnabled(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsEyeTrackingEnabled(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsEyeTrackingCalibrated(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsEyeTrackingCalibrated(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsEyeTrackingCalibrating(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsEyeTrackingCalibrating(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsEyeTrackingCalibratedForGlasses(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsEyeTrackingCalibratedForGlasses(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsHmdAdjustmentGuiVisible(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsHmdAdjustmentGuiVisible(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::HasHmdAdjustmentGuiTimeout(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->HasHmdAdjustmentGuiTimeout(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsEyeTrackingReady(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsEyeTrackingReady(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsUserPresent(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsUserPresent(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetEyesImage(EFoveErrorCode& OutErrorCode, UTexture*& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetEyesImage(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetUserIPD(EFoveErrorCode& OutErrorCode, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetUserIPD(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetUserIOD(EFoveErrorCode& OutErrorCode, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetUserIOD(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetPupilRadius(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetPupilRadius(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetIrisRadius(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetIrisRadius(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetEyeballRadius(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetEyeballRadius(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetEyeTorsion(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetEyeTorsion(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetEyeShape(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FFoveEyeShape& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetEyeShape(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::StartEyeTrackingCalibration(EFoveErrorCode& OutErrorCode, const FFoveCalibrationOptions& Options)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->StartEyeTrackingCalibration(Options) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::StopEyeTrackingCalibration(EFoveErrorCode& OutErrorCode)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->StopEyeTrackingCalibration() : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetEyeTrackingCalibrationState(EFoveErrorCode& OutErrorCode, EFoveCalibrationState& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetEyeTrackingCalibrationState(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::TickEyeTrackingCalibration(EFoveErrorCode& OutErrorCode, float Dt, bool IsVisible, FFoveCalibrationData& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->TickEyeTrackingCalibration(Dt, IsVisible, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetGazedObjectId(EFoveErrorCode& OutErrorCode, int& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetGazedObjectId(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::RegisterGazableObject(EFoveErrorCode& OutErrorCode, const FFoveGazableObject& Object)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->RegisterGazableObject(Object) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::UpdateGazableObject(EFoveErrorCode& OutErrorCode, const int ObjectId, const FFoveObjectPose& ObjectPose)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->UpdateGazableObject(ObjectId, ObjectPose) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::RemoveGazableObject(EFoveErrorCode& OutErrorCode, const int ObjectId)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->RemoveGazableObject(ObjectId) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::RegisterCameraObject(EFoveErrorCode& OutErrorCode, const FFoveCameraObject& Camera)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->RegisterCameraObject(Camera) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::UpdateCameraObject(EFoveErrorCode& OutErrorCode, const int cameraId, const FFoveObjectPose& Pose)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->UpdateCameraObject(cameraId, Pose) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::RemoveCameraObject(EFoveErrorCode& OutErrorCode, const int CameraId)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->RemoveCameraObject(CameraId) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetGazeCastPolicy(EFoveErrorCode& OutErrorCode, EFoveGazeCastPolicy& OutPolicy)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetGazeCastPolicy(OutPolicy) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::SetGazeCastPolicy(EFoveErrorCode& OutErrorCode, const EFoveGazeCastPolicy Policy)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->SetGazeCastPolicy(Policy) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::TareOrientationSensor(EFoveErrorCode& OutErrorCode)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->TareOrientationSensor() : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::IsPositionReady(EFoveErrorCode& OutErrorCode, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->IsPositionReady(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::TarePositionSensors(EFoveErrorCode& OutErrorCode)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->TarePositionSensors() : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::FetchPoseData(EFoveErrorCode& OutErrorCode,FFoveFrameTimestamp& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->FetchPoseData(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetPose(EFoveErrorCode& OutErrorCode, FTransform& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetPose(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetPositionImage(EFoveErrorCode& OutErrorCode, UTexture*& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetPositionImage(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetProjectionMatrix(EFoveErrorCode& OutErrorCode, const EFoveEye Eye, const float ZNear, const float ZFar, FMatrix& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetProjectionMatrix(Eye, ZNear, ZFar, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetRawProjectionValues(EFoveErrorCode& OutErrorCode, const EFoveEye Eye, FFoveProjectionParams& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetRawProjectionValues(Eye, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetRenderIOD(EFoveErrorCode& OutErrorCode, float& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetRenderIOD(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::CreateProfile(EFoveErrorCode& OutErrorCode, const FString& Name)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->CreateProfile(Name) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::RenameProfile(EFoveErrorCode& OutErrorCode, const FString& OldName, const FString& NewName)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->RenameProfile(OldName, NewName) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::DeleteProfile(EFoveErrorCode& OutErrorCode, const FString& ProfileName)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->DeleteProfile(ProfileName) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::ListProfiles(EFoveErrorCode& OutErrorCode, TArray<FString>& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->ListProfiles(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::SetCurrentProfile(EFoveErrorCode& OutErrorCode, const FString& ProfileName)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->SetCurrentProfile(ProfileName) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetCurrentProfile(EFoveErrorCode& OutErrorCode, FString& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetCurrentProfile(Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetProfileDataPath(EFoveErrorCode& OutErrorCode, const FString& ProfileName, FString& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->GetProfileDataPath(ProfileName, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::HasAccessToFeature(EFoveErrorCode& OutErrorCode, const FString& InFeatureName, bool& Out)
{
	FFoveHMD* const Hmd = FFoveHMD::Get();
	OutErrorCode = Hmd ? Hmd->HasAccessToFeature(InFeatureName, Out) : EFoveErrorCode::Unreal_NoFoveHMD;
}

void UFoveVRFunctionLibrary::GetConfigBool(EFoveErrorCode& OutErrorCode, const FString& Key, bool& Out)
{
	OutErrorCode = FFoveHMD::GetConfigBool(Key, Out);
}

void UFoveVRFunctionLibrary::GetConfigInt(EFoveErrorCode& OutErrorCode, const FString& Key, int& Out)
{
	OutErrorCode = FFoveHMD::GetConfigInt(Key, Out);
}

void UFoveVRFunctionLibrary::GetConfigFloat(EFoveErrorCode& OutErrorCode, const FString& Key, float& Out)
{
	OutErrorCode = FFoveHMD::GetConfigFloat(Key, Out);
}

void UFoveVRFunctionLibrary::GetConfigString(EFoveErrorCode& OutErrorCode, const FString& Key, FString& Out)
{
	OutErrorCode = FFoveHMD::GetConfigString(Key, Out);
}

void UFoveVRFunctionLibrary::SetConfigValueBool(EFoveErrorCode& OutErrorCode, const FString& Key, bool Value)
{
	OutErrorCode = FFoveHMD::SetConfigValueBool(Key, Value);
}

void UFoveVRFunctionLibrary::SetConfigValueInt(EFoveErrorCode& OutErrorCode, const FString& Key, int Value)
{
	OutErrorCode = FFoveHMD::SetConfigValueInt(Key, Value);
}

void UFoveVRFunctionLibrary::SetConfigValueFloat(EFoveErrorCode& OutErrorCode, const FString& Key, float Value)
{
	OutErrorCode = FFoveHMD::SetConfigValueFloat(Key, Value);
}

void UFoveVRFunctionLibrary::SetConfigValueString(EFoveErrorCode& OutErrorCode, const FString& Key, const FString& Value)
{
	OutErrorCode = FFoveHMD::SetConfigValueString(Key, Value);
}

void UFoveVRFunctionLibrary::ClearConfigValue(EFoveErrorCode& OutErrorCode, const FString& Key)
{
	OutErrorCode = FFoveHMD::ClearConfigValue(Key);
}

void UFoveVRFunctionLibrary::LogText(EFoveErrorCode& OutErrorCode, const EFoveLogLevel Level, const FString& Text)
{
	OutErrorCode = FFoveHMD::LogText(Level, Text);
}

void UFoveVRFunctionLibrary::IsValid(const EFoveErrorCode ErrorCode, bool& OutIsValid)
{
	OutIsValid = FFoveHMD::IsValid(ErrorCode);
}

void UFoveVRFunctionLibrary::BranchOnIsValid(const EFoveErrorCode ErrorCode, EFoveIsValid& OutBranch, EFoveErrorCode& OutErrorCode,  bool& OutIsValid, bool& OutIsReliable)
{
	OutErrorCode = ErrorCode;
	OutIsValid = FFoveHMD::IsValid(ErrorCode);
	OutIsReliable = FFoveHMD::IsReliable(ErrorCode);
	OutBranch = OutIsValid ? EFoveIsValid::IsValid : EFoveIsValid::IsNotValid;
}

void UFoveVRFunctionLibrary::IsReliable(const EFoveErrorCode ErrorCode, bool& OutIsReliable)
{
	OutIsReliable = FFoveHMD::IsReliable(ErrorCode);
}

void UFoveVRFunctionLibrary::BranchOnIsReliable(const EFoveErrorCode ErrorCode, EFoveIsReliable& OutBranch, EFoveErrorCode& OutErrorCode,  bool& OutIsValid, bool& OutIsReliable)
{
	OutErrorCode = ErrorCode;
	OutIsValid = FFoveHMD::IsValid(ErrorCode);
	OutIsReliable = FFoveHMD::IsReliable(ErrorCode);
	OutBranch = OutIsReliable ? EFoveIsReliable::IsReliable : EFoveIsReliable::IsNotReliable;
}

#ifdef _MSC_VER
#pragma endregion
#endif

//---------------------------------------------------
// FFoveHMDPlugin
//---------------------------------------------------

#ifdef _MSC_VER
#pragma region FFoveHMDPlugin
#else
#pragma mark FFoveHMDPlugin
#endif

class FFoveHMDPlugin : public IFoveHMDPlugin
{
public: // IHeadMountedDisplayModule implementation

	void StartupModule() override
	{
		IFoveHMDPlugin::StartupModule();

		// On windows, we delay loading of the DLL, so the game can function if it's missing
		// This is not implemented on other platforms currently
#if PLATFORM_WINDOWS
		if (!dllHandle)
		{
			// Get the library path based on the base dir of this plugin
			const FString baseDir = IPluginManager::Get().FindPlugin("FoveHMD")->GetBaseDir();
			const FString foveLibDir = FString::Printf(TEXT("Binaries/ThirdParty/FoveVR/FoveVR_SDK_%s/x64/FoveClient.dll"), FOVEVR_SDK_VER);
			const FString libraryPath = FPaths::Combine(*baseDir, *foveLibDir);

			// Load the fove client dll and show an error if it fails
			dllHandle = !libraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*libraryPath) : nullptr;
			if (!dllHandle)
			{
				UE_LOG(LogHMD, Warning, TEXT("Failed to load FoveVR DLL handle"));
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Failed to load FoveClient: " + libraryPath));
				return;
			}
		}
#endif

		// We do not create the Headset and Compositor objects here, hence the CreateObjectsIfNeeded() function
		// If we do so, it causes a "SECURE CRT: Invalid parameter detected" error when packing projects with the FOVE plugin
		// The reason for this is unknown
	}

	void ShutdownModule() override
	{
		// Clear headset & compositor
		// It is assumed that all other references are cleared by now as well
		Headset.Reset();
		Compositor = Fove::Compositor{};

		// Unload the fove client dll
		if (dllHandle)
		{
			FPlatformProcess::FreeDllHandle(dllHandle);
			dllHandle = nullptr;
		}

		// Call base class last per standard ordering
		IFoveHMDPlugin::ShutdownModule();
	}


#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	TSharedPtr<class IXRTrackingSystem  , ESPMode::ThreadSafe> CreateTrackingSystem() override
#else
	TSharedPtr<class IHeadMountedDisplay, ESPMode::ThreadSafe> CreateHeadMountedDisplay() override
#endif
	{
		CreateObjectsIfNeeded();
		if (!Headset.IsValid() || !Compositor.getCObject())
			return nullptr;

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
		TSharedPtr<FFoveHMD, ESPMode::ThreadSafe> FoveHMD = FSceneViewExtensions::NewExtension<FFoveHMD>(Headset.ToSharedRef(), MoveTemp(Compositor));
#else
		TSharedPtr<FFoveHMD, ESPMode::ThreadSafe> FoveHMD(new FFoveHMD(Headset.ToSharedRef(), MoveTemp(Compositor)));
#endif

		return FoveHMD;
	}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 14
	FString GetModuleKeyName() const override
#else
	FString GetModulePriorityKeyName() const override
#endif
	{
		return FString(TEXT("FoveHMD"));
	}

	bool IsHMDConnected() override
	{
		check(IsInGameThread());

		CreateObjectsIfNeeded();
		return IsFoveConnected(*Headset, Compositor);
	}

#if !PLATFORM_MAC
	struct FVulkanExtensions : public IHeadMountedDisplayVulkanExtensions
	{
		virtual ~FVulkanExtensions() {}

		bool GetVulkanInstanceExtensionsRequired(TArray<const ANSICHAR*>& Out) override
		{
			return true;
		}

		bool GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T *pPhysicalDevice, TArray<const ANSICHAR*>& Out) override
		{
			Out.Add("VK_KHR_external_memory");
			Out.Add("VK_KHR_external_memory_fd");
			return true;
		}
	};
	TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe > VulkanExtensions;

	TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe > GetVulkanExtensions() override
	{
		if (!VulkanExtensions.IsValid())
		{
			VulkanExtensions = MakeShareable(new FVulkanExtensions);
		}
		return VulkanExtensions;
	}
#endif

private:

	void CreateObjectsIfNeeded()
	{
		if (!Headset.IsValid())
		{
			// Create the headset object
			// Initially, we only enable orientation tracking
			// If you want position tracking, eye tracking, etc, enable it via RegisterCapabilities
			auto TempHeadset = Fove::Headset::create(Fove::ClientCapabilities::OrientationTracking);
			if (!TempHeadset)
			{
				UE_LOG(LogHMD, Warning, TEXT("Failed to create Headset %d"), static_cast<int>(TempHeadset.getError()));
				return;
			}

			Headset = TSharedPtr<Fove::Headset, ESPMode::ThreadSafe>(new Fove::Headset(MoveTemp(*TempHeadset)));
		}

		if (!Compositor.getCObject())
		{
			// Create or destroy the compositor object as needed
			// To lower overhead and not open IPC to the compositor, we do this only once the headset is plugged in
			Fove::Result<Fove::Compositor> TempCompositor = Headset->createCompositor();
			if (!TempCompositor)
			{
				UE_LOG(LogHMD, Warning, TEXT("Failed to create Compositor %d"), static_cast<int>(TempCompositor.getError()));
				return;
			}

			Compositor = MoveTemp(TempCompositor).getValue();
		}
	}

	TSharedPtr<Fove::Headset, ESPMode::ThreadSafe> Headset; // Headset is persisted and shared indefinitely
	Fove::Compositor Compositor; // Compositor is passed into FFoveHMD and recreated later again if needed

	void* dllHandle = nullptr;
};

IMPLEMENT_MODULE(FFoveHMDPlugin, FoveHMD)

#ifdef _MSC_VER
#pragma endregion
#endif

//---------------------------------------------------
// FFoveHMD
//---------------------------------------------------

#ifdef _MSC_VER
#pragma region FFoveHMD
#else
#pragma mark FFoveHMD
#endif

FFoveHMD::FFoveHMD(AFTER_4_18(const FAutoRegister& AutoRegister FOVEHMD_COMMA) TSharedRef<Fove::Headset, ESPMode::ThreadSafe> Headset, Fove::Compositor Compositor)
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 22
	: FOVEHMD_BASE_CLASS(nullptr) // 4.22 FHeadMountedDisplayBase requires IARSystemSupport* passed here
#else
	: FOVEHMD_BASE_CLASS()
#endif
	, FOVEHMD_SCENE_EXTENSION_BASE_CLASS(AFTER_4_18(AutoRegister))
	, ZNear(GNearClippingPlane)
	, ZFar(GNearClippingPlane)
	, FoveHeadset(MoveTemp(Headset))
	, FoveCompositor(new Fove::Compositor(MoveTemp(Compositor)))
	, Bridge(*(new TRefCountPtr<FoveRenderingBridge>))
{
	IHeadMountedDisplay::StartupModule();

	// Grab a pointer to the renderer module
	static const FName RendererModuleName("Renderer");
	RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

#if PLATFORM_WINDOWS
	if (IsPCPlatform(GMaxRHIShaderPlatform) && !IsOpenGLPlatform(GMaxRHIShaderPlatform))
	{
		Bridge = TRefCountPtr<FoveRenderingBridge>(new FoveD3D11Bridge(FoveCompositor));
	}
#endif

#if PLATFORM_MAC
	if (!Bridge && !IsOpenGLPlatform(GMaxRHIShaderPlatform))
	{
		Bridge = TRefCountPtr<FoveRenderingBridge>(new FoveMetalBridge(FoveCompositor));
	}
#else
	if (!Bridge && IsVulkanPlatform(GMaxRHIShaderPlatform))
	{
		Bridge = TRefCountPtr<FoveRenderingBridge>(new FoveVulkanBridge(FoveCompositor));
	}
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 17
	SpectatorScreenController = MakeUnique<FDefaultSpectatorScreenController>(this);
#endif

	UE_LOG(LogHMD, Log, TEXT("FFoveHMD initialized"));
}

FFoveHMD::~FFoveHMD()
{
	UE_LOG(LogHMD, Log, TEXT("FFoveHMD destructing"));

	delete &Bridge;
}

FFoveHMD* FFoveHMD::Get()
{
	// Get the global HMD object
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	IHeadMountedDisplay* const Hmd = GEngine->XRSystem ? GEngine->XRSystem->GetHMDDevice() : nullptr;
#else
	IHeadMountedDisplay* const Hmd = GEngine->HMDDevice.Get();
#endif
	if (!Hmd)
		return nullptr;

	// Check if the HMD object is a FoveHMD device
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	if (GEngine->XRSystem->GetSystemName() != TEXT("FoveHMD"))
#elif ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 13
	if (Hmd->GetDeviceName() != TEXT("FoveHMD"))
#else
	if (Hmd->GetHMDDeviceType() != EHMDDeviceType::DT_ES2GenericStereoMesh)
#endif
		return nullptr;

	return static_cast<FFoveHMD*>(Hmd);
}

EFoveErrorCode FFoveHMD::IsHardwareConnected(bool &Out)
{
	return ToUnreal(fove_Headset_isHardwareConnected(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsHardwareReady(bool &Out)
{
	return ToUnreal(fove_Headset_isHardwareReady(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsMotionReady(bool &Out)
{
	return ToUnreal(fove_Headset_isMotionReady(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::CheckSoftwareVersions()
{
	return ToUnreal(fove_Headset_checkSoftwareVersions(FoveHeadset->getCObject()));
}

EFoveErrorCode FFoveHMD::GetSoftwareVersions(FFoveVersions& Out)
{
	Fove::Versions Tmp;
	const auto Ret = ToUnreal(fove_Headset_getSoftwareVersions(FoveHeadset->getCObject(), &Tmp));
	Out.ClientMajor = Tmp.clientMajor;
	Out.ClientMinor = Tmp.clientMinor;
	Out.ClientBuild = Tmp.clientBuild;
	Out.ClientProtocol = Tmp.clientProtocol;
	Out.RuntimeMajor = Tmp.runtimeMajor;
	Out.RuntimeMinor = Tmp.runtimeMinor;
	Out.RuntimeBuild = Tmp.runtimeBuild;
	Out.Firmware = Tmp.firmware;
	Out.MaxFirmware = Tmp.maxFirmware;
	Out.MinFirmware = Tmp.minFirmware;
	Out.TooOldHeadsetConnected = Tmp.tooOldHeadsetConnected;
	return Ret;
}

EFoveErrorCode FFoveHMD::GetHeadsetHardwareInfo(FFoveHeadsetHardwareInfo& Out)
{
	Fove_HeadsetHardwareInfo Tmp;
	const auto Ret = ToUnreal(fove_Headset_getHardwareInfo(FoveHeadset->getCObject(), &Tmp));
	Out.Manufacturer = UTF8_TO_TCHAR(Tmp.manufacturer);
	Out.ModelName = UTF8_TO_TCHAR(Tmp.modelName);
	Out.SerialNumber = UTF8_TO_TCHAR(Tmp.serialNumber);
	return Ret;
}

Fove::ClientCapabilities ToFove(const TArray<EFoveClientCapabilities>& Caps)
{
	Fove::ClientCapabilities Ret = Fove::ClientCapabilities::None;

	for (EFoveClientCapabilities Cap : Caps)
	{
		Ret = Ret | [&]
		{
			switch (Cap)
			{
			case EFoveClientCapabilities::OrientationTracking: return Fove::ClientCapabilities::OrientationTracking;
			case EFoveClientCapabilities::PositionTracking: return Fove::ClientCapabilities::PositionTracking;
			case EFoveClientCapabilities::PositionImage: return Fove::ClientCapabilities::PositionImage;
			case EFoveClientCapabilities::EyeTracking: return Fove::ClientCapabilities::EyeTracking;
			case EFoveClientCapabilities::GazeDepth: return Fove::ClientCapabilities::GazeDepth;
			case EFoveClientCapabilities::UserPresence: return Fove::ClientCapabilities::UserPresence;
			case EFoveClientCapabilities::UserAttentionShift: return Fove::ClientCapabilities::UserAttentionShift;
			case EFoveClientCapabilities::UserIOD: return Fove::ClientCapabilities::UserIOD;
			case EFoveClientCapabilities::UserIPD: return Fove::ClientCapabilities::UserIPD;
			case EFoveClientCapabilities::EyeTorsion: return Fove::ClientCapabilities::EyeTorsion;
			case EFoveClientCapabilities::EyeShape: return Fove::ClientCapabilities::EyeShape;
			case EFoveClientCapabilities::EyesImage: return Fove::ClientCapabilities::EyesImage;
			case EFoveClientCapabilities::EyeballRadius: return Fove::ClientCapabilities::EyeballRadius;
			case EFoveClientCapabilities::IrisRadius: return Fove::ClientCapabilities::IrisRadius;
			case EFoveClientCapabilities::PupilRadius: return Fove::ClientCapabilities::PupilRadius;
			case EFoveClientCapabilities::GazedObjectDetection: return Fove::ClientCapabilities::GazedObjectDetection;
			case EFoveClientCapabilities::DirectScreenAccess: return Fove::ClientCapabilities::DirectScreenAccess;
			};
			UE_LOG(LogHMD, Error, TEXT("Unknown Capability %d"), static_cast<int>(Cap));
			return Fove::ClientCapabilities::None;
		}();
	}

	return Ret;
}

EFoveErrorCode FFoveHMD::RegisterCapabilities(const TArray<EFoveClientCapabilities>& Caps)
{
	return ToUnreal(fove_Headset_registerCapabilities(FoveHeadset->getCObject(), ToFove(Caps)));
}

EFoveErrorCode FFoveHMD::UnregisterCapabilities(const TArray<EFoveClientCapabilities>& Caps)
{
	return ToUnreal(fove_Headset_unregisterCapabilities(FoveHeadset->getCObject(), ToFove(Caps)));
}

EFoveErrorCode FFoveHMD::WaitAndFetchNextEyeTrackingData(FFoveFrameTimestamp& Out)
{
	Fove::FrameTimestamp Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_waitAndFetchNextEyeTrackingData(FoveHeadset->getCObject(), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::FetchEyeTrackingData(FFoveFrameTimestamp& Out)
{
	Fove::FrameTimestamp Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_fetchEyeTrackingData(FoveHeadset->getCObject(), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetGazeVector(EFoveEye Eye, FVector& Out)
{
	Fove::Vec3 Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getGazeVector(FoveHeadset->getCObject(), ToFove(Eye), &Tmp));
	Out = ToUnreal(Tmp, 1.0f);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetGazeScreenPosition(EFoveEye Eye, FVector2D& Out)
{
	Fove::Vec2 Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getGazeScreenPosition(FoveHeadset->getCObject(), ToFove(Eye), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetCombinedGazeRay(FVector& OutOrigin, FVector& OutDirection)
{
	Fove::Ray Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getCombinedGazeRay(FoveHeadset->getCObject(), &Tmp));
	OutOrigin = ToUnreal(Tmp.origin, WorldToMetersScale);
	OutDirection = ToUnreal(Tmp.direction, 1.0f);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetCombinedGazeDepth(float& Out)
{
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getCombinedGazeDepth(FoveHeadset->getCObject(), &Out));
	Out *= WorldToMetersScale;
	return Ret;
}

EFoveErrorCode FFoveHMD::IsUserShiftingAttention(bool& Out)
{
	return ToUnreal(fove_Headset_isUserShiftingAttention(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::GetEyeState(EFoveEye Eye, EFoveEyeState& Out)
{
	Fove::EyeState Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getEyeState(FoveHeadset->getCObject(), ToFove(Eye), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::IsEyeTrackingEnabled(bool& Out)
{
	return ToUnreal(fove_Headset_isEyeTrackingEnabled(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsEyeTrackingCalibrated(bool& Out)
{
	return ToUnreal(fove_Headset_isEyeTrackingCalibrated(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsEyeTrackingCalibrating(bool& Out)
{
	return ToUnreal(fove_Headset_isEyeTrackingCalibrating(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsEyeTrackingCalibratedForGlasses(bool& Out)
{
	return ToUnreal(fove_Headset_isEyeTrackingCalibratedForGlasses(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsHmdAdjustmentGuiVisible(bool& Out)
{
	return ToUnreal(fove_Headset_isHmdAdjustmentGuiVisible(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::HasHmdAdjustmentGuiTimeout(bool& Out)
{
	return ToUnreal(fove_Headset_hasHmdAdjustmentGuiTimeout(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsEyeTrackingReady(bool& Out)
{
	return ToUnreal(fove_Headset_isEyeTrackingReady(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::IsUserPresent(bool& Out)
{
	return ToUnreal(fove_Headset_isUserPresent(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::GetEyesImage(UTexture*& Out)
{
	return GenerateTextureHelper(Out, TEXT("GetEyesImage"), &fove_Headset_getEyesImage, FoveHeadset->getCObject());
}

EFoveErrorCode FFoveHMD::GetUserIPD(float& Out)
{
	return ToUnreal(fove_Headset_getUserIPD(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::GetUserIOD(float& Out)
{
	return ToUnreal(fove_Headset_getUserIOD(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::GetPupilRadius(EFoveEye Eye, float& Out)
{
	return ToUnreal(fove_Headset_getPupilRadius(FoveHeadset->getCObject(), ToFove(Eye), &Out));
}

EFoveErrorCode FFoveHMD::GetIrisRadius(EFoveEye Eye, float& Out)
{
	return ToUnreal(fove_Headset_getIrisRadius(FoveHeadset->getCObject(), ToFove(Eye), &Out));
}

EFoveErrorCode FFoveHMD::GetEyeballRadius(EFoveEye Eye, float& Out)
{
	return ToUnreal(fove_Headset_getEyeballRadius(FoveHeadset->getCObject(), ToFove(Eye), &Out));
}

EFoveErrorCode FFoveHMD::GetEyeTorsion(EFoveEye Eye, float& Out)
{
	return ToUnreal(fove_Headset_getEyeTorsion(FoveHeadset->getCObject(), ToFove(Eye), &Out));
}

EFoveErrorCode FFoveHMD::GetEyeShape(EFoveEye Eye, FFoveEyeShape& Out)
{
	Fove::EyeShape Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getEyeShape(FoveHeadset->getCObject(), ToFove(Eye), &Tmp));
	Out.Outline00 = ToUnreal(Tmp.outline[0]);
	Out.Outline01 = ToUnreal(Tmp.outline[1]);
	Out.Outline02 = ToUnreal(Tmp.outline[2]);
	Out.Outline03 = ToUnreal(Tmp.outline[3]);
	Out.Outline04 = ToUnreal(Tmp.outline[4]);
	Out.Outline05 = ToUnreal(Tmp.outline[5]);
	Out.Outline06 = ToUnreal(Tmp.outline[6]);
	Out.Outline07 = ToUnreal(Tmp.outline[7]);
	Out.Outline08 = ToUnreal(Tmp.outline[8]);
	Out.Outline09 = ToUnreal(Tmp.outline[9]);
	Out.Outline10 = ToUnreal(Tmp.outline[10]);
	Out.Outline11 = ToUnreal(Tmp.outline[11]);
	return Ret;
}

EFoveErrorCode FFoveHMD::StartEyeTrackingCalibration(const FFoveCalibrationOptions& Options)
{
	Fove::CalibrationOptions Tmp;
	Tmp.lazy = Options.Lazy;
	Tmp.restart = Options.Restart;
	Tmp.method = ToFove(Options.Method);
	Tmp.eyeByEye = ToFove(Options.EyeByEye);
	return ToUnreal(fove_Headset_startEyeTrackingCalibration(FoveHeadset->getCObject(), &Tmp));
}

EFoveErrorCode FFoveHMD::StopEyeTrackingCalibration()
{
	return ToUnreal(fove_Headset_stopEyeTrackingCalibration(FoveHeadset->getCObject()));
}

EFoveErrorCode FFoveHMD::GetEyeTrackingCalibrationState(EFoveCalibrationState& Out)
{
	Fove::CalibrationState Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getEyeTrackingCalibrationState(FoveHeadset->getCObject(), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::TickEyeTrackingCalibration(float Dt, bool IsVisible, FFoveCalibrationData& Out)
{
	Fove::CalibrationData Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_tickEyeTrackingCalibration(FoveHeadset->getCObject(), Dt, IsVisible, &Tmp));
	Out = ToUnreal(Tmp, WorldToMetersScale);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetGazedObjectId(int& Out)
{
	return ToUnreal(fove_Headset_getGazedObjectId(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::RegisterGazableObject(const FFoveGazableObject& Object)
{
	Fove::GazableObject Tmp;
	Tmp.id = Object.Id;
	Tmp.group = ToFove(Object.Group);
	Tmp.colliderCount = 0;/*Object.Collider.Num();
	for (int i = 0; i < Object.Collider.Num())
	{FIXME
	}*/
	return ToUnreal(fove_Headset_registerGazableObject(FoveHeadset->getCObject(), &Tmp));
}

EFoveErrorCode FFoveHMD::UpdateGazableObject(const int ObjectId, const FFoveObjectPose& ObjectPose)
{
	const Fove::ObjectPose Tmp = ToFove(ObjectPose, WorldToMetersScale);
	return ToUnreal(fove_Headset_updateGazableObject(FoveHeadset->getCObject(), ObjectId, &Tmp));
}

EFoveErrorCode FFoveHMD::RemoveGazableObject(const int ObjectId)
{
	return ToUnreal(fove_Headset_removeGazableObject(FoveHeadset->getCObject(), ObjectId));
}

EFoveErrorCode FFoveHMD::RegisterCameraObject(const FFoveCameraObject& Camera)
{
	const Fove::CameraObject Tmp = ToFove(Camera, WorldToMetersScale);
	return ToUnreal(fove_Headset_registerCameraObject(FoveHeadset->getCObject(), &Tmp));
}

EFoveErrorCode FFoveHMD::UpdateCameraObject(const int cameraId, const FFoveObjectPose& Pose)
{
	const Fove::ObjectPose Tmp = ToFove(Pose, WorldToMetersScale);
	return ToUnreal(fove_Headset_updateCameraObject(FoveHeadset->getCObject(), cameraId, &Tmp));
}

EFoveErrorCode FFoveHMD::RemoveCameraObject(const int CameraId)
{
	return ToUnreal(fove_Headset_removeCameraObject(FoveHeadset->getCObject(), CameraId));
}

EFoveErrorCode FFoveHMD::GetGazeCastPolicy(EFoveGazeCastPolicy& Out)
{
	Fove::GazeCastPolicy Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getGazeCastPolicy(FoveHeadset->getCObject(), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::SetGazeCastPolicy(const EFoveGazeCastPolicy Policy)
{
	return ToUnreal(fove_Headset_setGazeCastPolicy(FoveHeadset->getCObject(), ToFove(Policy)));
}

EFoveErrorCode FFoveHMD::TareOrientationSensor()
{
	return ToUnreal(fove_Headset_tareOrientationSensor(FoveHeadset->getCObject()));
}

EFoveErrorCode FFoveHMD::IsPositionReady(bool& Out)
{
	return ToUnreal(fove_Headset_isPositionReady(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::TarePositionSensors()
{
	return ToUnreal(fove_Headset_tarePositionSensors(FoveHeadset->getCObject()));
}

EFoveErrorCode FFoveHMD::FetchPoseData(FFoveFrameTimestamp& Out)
{
	Fove::FrameTimestamp Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_fetchPoseData(FoveHeadset->getCObject(), &Tmp));
	Out = ToUnreal(Tmp);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetPose(FTransform& Out)
{
	Fove::Pose Tmp;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getPose(FoveHeadset->getCObject(), &Tmp));
	Out = ToUnreal(Tmp, WorldToMetersScale);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetPositionImage(UTexture*& Out)
{
	return GenerateTextureHelper(Out, TEXT("GetPositionImage"), &fove_Headset_getPositionImage, FoveHeadset->getCObject());
}

EFoveErrorCode FFoveHMD::GetProjectionMatrix(const EFoveEye Eye, const float InZNear, const float InZFar, FMatrix& Out)
{
	Fove::Matrix44 TmpL, TmpR;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getProjectionMatricesLH(FoveHeadset->getCObject(), InZNear, InZFar, &TmpL, &TmpR));
	Out = ToUnreal(Eye == EFoveEye::Left ? TmpL : TmpR);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetRawProjectionValues(const EFoveEye Eye, FFoveProjectionParams& Out)
{
	Fove_ProjectionParams TmpL, TmpR;
	const EFoveErrorCode Ret = ToUnreal(fove_Headset_getRawProjectionValues(FoveHeadset->getCObject(), &TmpL, &TmpR));
	Out = ToUnreal(Eye == EFoveEye::Left ? TmpL : TmpR);
	return Ret;
}

EFoveErrorCode FFoveHMD::GetRenderIOD(float& Out)
{
	return ToUnreal(fove_Headset_getRenderIOD(FoveHeadset->getCObject(), &Out));
}

EFoveErrorCode FFoveHMD::CreateProfile(const FString& Name)
{
	return ToUnreal(fove_Headset_createProfile(FoveHeadset->getCObject(), TCHAR_TO_UTF8(*Name)));
}

EFoveErrorCode FFoveHMD::RenameProfile(const FString& OldName, const FString& NewName)
{
	return ToUnreal(fove_Headset_renameProfile(FoveHeadset->getCObject(), TCHAR_TO_UTF8(*OldName), TCHAR_TO_UTF8(*NewName)));
}

EFoveErrorCode FFoveHMD::DeleteProfile(const FString& ProfileName)
{
	return ToUnreal(fove_Headset_deleteProfile(FoveHeadset->getCObject(), TCHAR_TO_UTF8(*ProfileName)));
}

EFoveErrorCode FFoveHMD::ListProfiles(TArray<FString>& Out)
{
	auto Callback = [] (const char* const Value, void* const Out)
	{
		reinterpret_cast<TArray<FString>*>(Out)->Add(UTF8_TO_TCHAR(Value));
	};
	return ToUnreal(fove_Headset_listProfiles(FoveHeadset->getCObject(), Callback, &Out));
}

EFoveErrorCode FFoveHMD::SetCurrentProfile(const FString& ProfileName)
{
	return ToUnreal(fove_Headset_setCurrentProfile(FoveHeadset->getCObject(), TCHAR_TO_UTF8(*ProfileName)));
}

EFoveErrorCode FFoveHMD::GetCurrentProfile(FString& Out)
{
	auto Callback = [] (const char* const Value, void* const Out)
	{
		*reinterpret_cast<FString*>(Out) = UTF8_TO_TCHAR(Value);
	};
	return ToUnreal(fove_Headset_getCurrentProfile(FoveHeadset->getCObject(), Callback, &Out));
}

EFoveErrorCode FFoveHMD::GetProfileDataPath(const FString& ProfileName, FString& Out)
{
	auto Callback = [] (const char* const Value, void* const Out)
	{
		*reinterpret_cast<FString*>(Out) = UTF8_TO_TCHAR(Value);
	};
	return ToUnreal(fove_Headset_getProfileDataPath(FoveHeadset->getCObject(), TCHAR_TO_UTF8(*ProfileName), Callback, &Out));
}

EFoveErrorCode FFoveHMD::HasAccessToFeature(const FString& InFeatureName, bool& Out)
{
	return ToUnreal(fove_Headset_hasAccessToFeature(FoveHeadset->getCObject(), TCHAR_TO_UTF8(*InFeatureName), &Out));
}

EFoveErrorCode FFoveHMD::GetConfigBool(const FString& Key, bool& Out)
{
	return ToUnreal(fove_Config_getValue_bool(TCHAR_TO_UTF8(*Key), &Out));
}

EFoveErrorCode FFoveHMD::GetConfigInt(const FString& Key, int& Out)
{
	return ToUnreal(fove_Config_getValue_int(TCHAR_TO_UTF8(*Key), &Out));
}

EFoveErrorCode FFoveHMD::GetConfigFloat(const FString& Key, float& Out)
{
	return ToUnreal(fove_Config_getValue_float(TCHAR_TO_UTF8(*Key), &Out));
}

EFoveErrorCode FFoveHMD::GetConfigString(const FString& Key, FString& Out)
{
	auto Callback = [] (const char* const Value, void* const Out)
	{
		*reinterpret_cast<FString*>(Out) = UTF8_TO_TCHAR(Value);
	};
	return ToUnreal(fove_Config_getValue_string(TCHAR_TO_UTF8(*Key), Callback, &Out));
}

EFoveErrorCode FFoveHMD::SetConfigValueBool(const FString& Key, bool Value)
{
	return ToUnreal(fove_Config_setValue_bool(TCHAR_TO_UTF8(*Key), Value));
}

EFoveErrorCode FFoveHMD::SetConfigValueInt(const FString& Key, int Value)
{
	return ToUnreal(fove_Config_setValue_int(TCHAR_TO_UTF8(*Key), Value));
}

EFoveErrorCode FFoveHMD::SetConfigValueFloat(const FString& Key, float Value)
{
	return ToUnreal(fove_Config_setValue_float(TCHAR_TO_UTF8(*Key), Value));
}

EFoveErrorCode FFoveHMD::SetConfigValueString(const FString& Key, const FString& Value)
{
	return ToUnreal(fove_Config_setValue_string(TCHAR_TO_UTF8(*Key), TCHAR_TO_UTF8(*Value)));
}

EFoveErrorCode FFoveHMD::ClearConfigValue(const FString& Key)
{
	return ToUnreal(fove_Config_clearValue(TCHAR_TO_UTF8(*Key)));
}

EFoveErrorCode FFoveHMD::LogText(const EFoveLogLevel Level, const FString& Text)
{
	return ToUnreal(fove_logText(ToFove(Level), TCHAR_TO_UTF8(*Text)));
}

bool FFoveHMD::IsValid(const EFoveErrorCode err)
{
	return Fove::Result<>(ToFove(err)).isValid();
}

bool FFoveHMD::IsReliable(const EFoveErrorCode err)
{
	return Fove::Result<>(ToFove(err)).isReliable();
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26

int32 FFoveHMD::GetXRSystemFlags() const
{
	return EXRSystemFlags::IsHeadMounted;
}

#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20

FXRRenderBridge* FFoveHMD::GetActiveRenderBridge_GameThread(bool bUseSeparateRenderTarget)
{
	// Supersedes UpdateViewportRHIBridge
	return Bridge;
}

#endif // ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
FName FFoveHMD::GetSystemName() const
{
	static FName name(TEXT("FoveHMD"));
	return name;
}

bool FFoveHMD::EnumerateTrackedDevices(TArray<int, FDefaultAllocator>& OutDevices, const EXRTrackedDeviceType Type)
{
	if (Type == EXRTrackedDeviceType::Any || Type == EXRTrackedDeviceType::HeadMountedDisplay)
	{
		static const int32 DeviceId = IXRTrackingSystem::HMDDeviceId;
		OutDevices.Add(DeviceId);
		return true;
	}
	return false;
}

void FFoveHMD::RefreshPoses()
{
	Fove::Result<Fove::Pose> Pose;
	const bool isRendering = IsInRenderingThread();
	if (isRendering)
		Pose = GetCompositor().waitForRenderPose();
	else
	{
		FoveHeadset->fetchPoseData();
		Pose = FoveHeadset->getPose();
	}

	if (Pose && Bridge)
		Bridge->SetRenderPose(Pose.getValue(), WorldToMetersScale);
	else
		UE_LOG(LogHMD, Warning, TEXT("Failed to get FOVE pose: %d %d"), static_cast<int>(Pose.getError()), isRendering ? 0 : 1);
}

bool FFoveHMD::GetCurrentPose(const int32 DeviceId, FQuat& OutQuat, FVector& OutVec)
{
	check(DeviceId == IXRTrackingSystem::HMDDeviceId);

	if (Bridge)
	{
		const FTransform Transform = Bridge->GetRenderPose();
		OutQuat = Transform.GetRotation();
		OutVec = Transform.GetLocation();
		return true;
	}

	return false;
}

bool FFoveHMD::OnStartGameFrame(FWorldContext& WorldContext)
{
	check(IsInGameThread());

#if 1
	// Attempt to enable stereo every frame
	EnableStereo(true);
#endif

	// Try to connect to the compositor if it was not running at the start of the game
	if (!Bridge->IsCompositorLayerValid() && IsHMDReallyConnected())
		Bridge->CreateCompositorLayer();

	return true;
}

void FFoveHMD::CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* SrcTexture, FIntRect SrcRect, FRHITexture2D* DstTexture, FIntRect DstRect, bool bClearBlack, bool bNoAlpha) const
{
	// FIXME MERGE THIS WITH MANUAL RENDER CODE

	check(IsInRenderingThread());

	const uint32 ViewportWidth = DstRect.Width();
	const uint32 ViewportHeight = DstRect.Height();
	const FIntPoint TargetSize(ViewportWidth, ViewportHeight);

	const float SrcTextureWidth = SrcTexture->GetSizeX();
	const float SrcTextureHeight = SrcTexture->GetSizeY();
	float U = 0.f, V = 0.f, USize = 1.f, VSize = 1.f;
	if (!SrcRect.IsEmpty())
	{
		U = SrcRect.Min.X / SrcTextureWidth;
		V = SrcRect.Min.Y / SrcTextureHeight;
		USize = SrcRect.Width() / SrcTextureWidth;
		VSize = SrcRect.Height() / SrcTextureHeight;
	}

	FRHITexture * ColorRT = DstTexture->GetTexture2D();
	FRHIRenderPassInfo RenderPassInfo(ColorRT, ERenderTargetActions::DontLoad_Store);
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("FOVEHMD_CopyTexture"));
	{
		if (bClearBlack)
		{
			const FIntRect ClearRect(0, 0, DstTexture->GetSizeX(), DstTexture->GetSizeY());
			RHICmdList.SetViewport(ClearRect.Min.X, ClearRect.Min.Y, 0, ClearRect.Max.X, ClearRect.Max.Y, 1.0f);
			DrawClearQuad(RHICmdList, FLinearColor::Black);
		}

		RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState = bNoAlpha ? TStaticBlendState<>::GetRHI() : TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		const auto FeatureLevel = GMaxRHIFeatureLevel;
		auto ShaderMap = GetGlobalShaderMap(FeatureLevel);

		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
		TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = BEFORE_4_25(GETSAFERHISHADER_VERTEX(*VertexShader)) AFTER_4_25(VertexShader.GetVertexShader());
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = BEFORE_4_25(GETSAFERHISHADER_PIXEL(*PixelShader)) AFTER_4_25(PixelShader.GetPixelShader());

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		const bool bSameSize = DstRect.Size() == SrcRect.Size();
		if (bSameSize)
		{
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Point>::GetRHI(), SrcTexture);
		}
		else
		{
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);
		}

		RendererModule->DrawRectangle(
			RHICmdList,
			0, 0,
			ViewportWidth, ViewportHeight,
			U, V,
			USize, VSize,
			TargetSize,
			FIntPoint(1, 1),
			BEFORE_4_25(*)VertexShader,
			EDRF_Default);
	}
	RHICmdList.EndRenderPass();
}

FIntRect FFoveHMD::GetFullFlatEyeRect_RenderThread(FTexture2DRHIRef EyeTexture) const
{
	// This is used by the spectator screen to determine the size of the render texture when cropping out the left/right eye textures
	static FVector2D SrcNormRectMin(0.05f, 0.2f);
	static FVector2D SrcNormRectMax(0.45f, 0.8f);
	return FIntRect(EyeTexture->GetSizeX() * SrcNormRectMin.X, EyeTexture->GetSizeY() * SrcNormRectMin.Y, EyeTexture->GetSizeX() * SrcNormRectMax.X, EyeTexture->GetSizeY() * SrcNormRectMax.Y);
}

#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 13 && ENGINE_MINOR_VERSION < 18
FName FFoveHMD::GetDeviceName() const
{
	static FName name(TEXT("FoveHMD"));
	return name;
}
#endif

bool FFoveHMD::IsHMDConnected()
{
	if (GetDefault<UFoveVRSettings>()->bForceStartInVR)
		return true;
	return IsHMDReallyConnected();
}

bool FFoveHMD::IsHMDReallyConnected()
{
	return IsFoveConnected(*FoveHeadset, *FoveCompositor);
}

bool FFoveHMD::IsHMDEnabled() const
{
	return bHmdEnabled;
}

void FFoveHMD::EnableHMD(const bool enable)
{
	// Early out
	if (bHmdEnabled == enable)
		return;

	// The documentation for this function in unreal simply states: "Enables or disables switching to stereo."
	// The meaning of the statement is unclear and could be either:
	//  a) Enables/disables stereo directly
	//  b) Enables/disables the ability to enable stereo (but enabling stereo would be a separate call)
	// We've taken it to mean the latter, so we don't enable stereo when the hmd is enabled
	// However, if you disable the hmd, we no longer have the ability to be in stereo, so we disable that
	if (!enable)
		EnableStereo(false);

	// Update cached state
	// This happens after the call to EnableStereo(false) as that function becomes a no-op when bHmdEnabled is true
	bHmdEnabled = enable;
}

bool FFoveHMD::GetHMDMonitorInfo(MonitorInfo& outInfo)
{
	// Write default values
	outInfo.MonitorName = "";
	outInfo.MonitorId = 0;
	outInfo.DesktopX = outInfo.DesktopY = outInfo.ResolutionX = outInfo.ResolutionY = outInfo.WindowSizeX = outInfo.WindowSizeX = 0;

	// Write resolution
	if (Bridge->IsCompositorLayerValid())
	{
		const Fove::CompositorLayer layer = Bridge->GetCompositorLayer();
		outInfo.ResolutionX = outInfo.WindowSizeX = layer.idealResolutionPerEye.x * 2; // Stereo rendering places the two eyes side by side horizontally
		outInfo.ResolutionY = outInfo.WindowSizeY = layer.idealResolutionPerEye.y;
	}

	return true;
}

void FFoveHMD::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
	OutHFOVInDegrees = 0.0f; // TODO
	OutVFOVInDegrees = 0.0f;
}

bool FFoveHMD::IsChromaAbCorrectionEnabled() const
{
	// Note from Unreal after being asked why the engine needs to know this:
	// Generally, we don't!  However, on certain platforms, there are options to turn on and off
	// chromatic aberration correction to trade off performance and quality, which is why we
	// provide the option in the interface.  It's fine to always return true if you're doing it.
	return true;
}

void FFoveHMD::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
	UE_LOG(LogHMD, Warning, TEXT("FOVE does not support SetInterpupillaryDistance"));
}

float FFoveHMD::GetInterpupillaryDistance() const
{
	// Fetch inter-ocular distance from Fove service
	const Fove::Result<float> Ret = FoveHeadset->getRenderIOD();
	if (!Ret)
	{
		UE_LOG(LogHMD, Warning, TEXT("Headset::getIOD failed: %d"), static_cast<int>(Ret.getError()));
		return 0.064f; // Sane default in the event of error
	}

	return Ret.getValue();
}

bool FFoveHMD::DoesSupportPositionalTracking() const
{
	// Todo: Fove supports position tracking in general,
	// but should we query whether the position camera is connected for this?
	return true;
}

bool FFoveHMD::HasValidTrackingPosition()
{
	// Todo: FOVE API has no way to return whether we currently have a valid position, simply that position tracking is running
	const Fove::Result<bool> Ret = FoveHeadset->isPositionReady();
	if (!Ret)
		UE_LOG(LogHMD, Warning, TEXT("Headset::isPositionReady: %d"), static_cast<int>(Ret.getError()));

	return Ret.getValue();
}

void FFoveHMD::RebaseObjectOrientationAndPosition(FVector& Position, FQuat& Orientation) const
{
	UE_LOG(LogHMD, Warning, TEXT("FOVE does not support RebaseObjectOrientationAndPosition"));
}

bool FFoveHMD::IsHeadTrackingAllowed() const
{
	return true;// GEngine && GEngine->IsStereoscopic3D();
}

void FFoveHMD::ResetOrientationAndPosition(float yaw)
{
	// Note from Unreal about this function:
	// The intention of these functions is to allow the user to reset the calibrated
	// position at any point in the experience.  Generally, this takes the form of
	// saving a base orientation and position, and then using those to modify the
	// pose returned from the SDK as a "poor man's calibration."

	ResetOrientation(yaw);
	ResetPosition();
}

void FFoveHMD::ResetOrientation(float yaw)
{
	// Fixme: what to do with yaw?
	const Fove::Result<> Ret = FoveHeadset->tareOrientationSensor();
	if (!Ret)
		UE_LOG(LogHMD, Warning, TEXT("Headset::tareOrientationSensor failed: %d"), static_cast<int>(Ret.getError()));
}

void FFoveHMD::ResetPosition()
{
	const Fove::Result<> Ret = FoveHeadset->tarePositionSensors();
	if (!Ret)
		UE_LOG(LogHMD, Warning, TEXT("Headset::tarePositionSensors failed: %d"), static_cast<int>(Ret.getError()));
}

void FFoveHMD::SetBaseRotation(const FRotator& BaseRot)
{
	BaseOrientation = BaseRot.Quaternion();
}

FRotator FFoveHMD::GetBaseRotation() const
{
	return BaseOrientation.Rotator();
}

void FFoveHMD::SetBaseOrientation(const FQuat& BaseOrient)
{
	BaseOrientation = BaseOrient;
}

FQuat FFoveHMD::GetBaseOrientation() const
{
	return BaseOrientation;
}

void FFoveHMD::OnBeginPlay(FWorldContext& InWorldContext)
{
#if 0
	EnableStereo(true);
#endif
}

void FFoveHMD::OnEndPlay(FWorldContext& InWorldContext)
{
	EnableStereo(false);
}

void FFoveHMD::SetTrackingOrigin(EHMDTrackingOrigin::Type NewOrigin)
{
	// Note from Unreal:
	// This basically allows you to consider the calibrated origin in two locations, depending on the style of game and hardware.
	// EHMDTrackingOrigin::Eye means that the "zero" position is where the player's eyes are.  Floor means that it's on the floor.
	// The difference matters to how people set up their content. Generally, games that require the player to stand use the Floor
	// origin, and the player's pawn is set up so that the Camera Component's parent is located at the bottom of their collision (at the feet).
	// This is nice, because you know that the player's height in game will be the same as their real world height.
	// Alternatively, for games where the player is disembodied, or sitting in a chair, it's more useful to consider the origin of the
	// camera at their eyes, so you can place the parent there.  The player is no longer their true height, but for cockpit games, etc. this doesn't matter.

	switch (NewOrigin)
	{
	case EHMDTrackingOrigin::Eye:
		break;
	default:
		// Fove currently only supports sitting experiences, if a game tries to set this, log a warning
		UE_LOG(LogHMD, Warning, TEXT("FOVE only supports EHMDTrackingOrigin::Eye"));
		break;
	}
}

EHMDTrackingOrigin::Type FFoveHMD::GetTrackingOrigin() AFTER_4_23(const)
{
	// Currently, FOVE only supports sitting experiences. See comment in SetTrackingOrigin
	return EHMDTrackingOrigin::Eye;
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 18 // Removed in 4.18

void FFoveHMD::GetPositionalTrackingCameraProperties(FVector&, FQuat&, float&, float&, float&, float&, float&) const
{
	UE_LOG(LogHMD, Warning, TEXT("FOVE does not support GetPositionalTrackingCameraProperties"));
}

void FFoveHMD::GetCurrentOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition)
{
	PrivOrientationAndPosition(CurrentOrientation, CurrentPosition);
}

TSharedPtr< class ISceneViewExtension, ESPMode::ThreadSafe > FFoveHMD::GetViewExtension()
{
	TSharedPtr< FFoveHMD, ESPMode::ThreadSafe > ptr(AsShared());
	return StaticCastSharedPtr< ISceneViewExtension >(ptr);
}

void FFoveHMD::ApplyHmdRotation(APlayerController* PC, FRotator& ViewRotation)
{
	ViewRotation.Normalize();

	FQuat hmdOrientation;
	FVector hmdPosition;
	GetCurrentOrientationAndPosition(hmdOrientation, hmdPosition);

	const FRotator DeltaRot = ViewRotation - PC->GetControlRotation();
	ControlRotation = (ControlRotation + DeltaRot).GetNormalized();

	// Pitch from other sources is never good, because there is an absolute up and down that must be respected to avoid motion sickness.
	// Same with roll. Retain yaw by default - mouse/controller based yaw movement still isn't pleasant, but
	// it's necessary for sitting VR experiences.
	ControlRotation.Pitch = 0;
	ControlRotation.Roll = 0;

	ViewRotation = FRotator(ControlRotation.Quaternion() * hmdOrientation);

	AppliedHmdOrientation = FRotator(hmdOrientation);
	AppliedHmdOrientation.Pitch = 0;
	AppliedHmdOrientation.Roll = 0;
}

bool FFoveHMD::UpdatePlayerCamera(FQuat& CurrentOrientation, FVector& CurrentPosition)
{
	FQuat hmdOrientation;
	FVector hmdPosition;
	GetCurrentOrientationAndPosition(hmdOrientation, hmdPosition);

	CurrentOrientation = hmdOrientation;
	CurrentPosition = AppliedHmdOrientation.Quaternion().Inverse().RotateVector(hmdPosition);

	return true;
}

bool FFoveHMD::IsPositionalTrackingEnabled() const
{
	return true;
}
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 16
bool FFoveHMD::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FParse::Command(&Cmd, TEXT("STEREO")))
	{
		if (FParse::Command(&Cmd, TEXT("ON")))
		{
			if (!IsHMDEnabled())
			{
				Ar.Logf(TEXT("HMD is disabled. Use 'hmd enable' to re-enable it"));
			}
			EnableStereo(true);
			return true;
		}
		else if (FParse::Command(&Cmd, TEXT("OFF")))
		{
			EnableStereo(false);
			return true;
		}
	}
	else if (FParse::Command(&Cmd, TEXT("HMD")))
	{
		if (FParse::Command(&Cmd, TEXT("ENABLE")))
		{
			EnableHMD(true);
			return true;
		}
		else if (FParse::Command(&Cmd, TEXT("DISABLE")))
		{
			EnableHMD(false);
			return true;
		}
	}
	else if (FParse::Command(&Cmd, TEXT("UNCAPFPS")))
	{
		GEngine->bSmoothFrameRate = false;
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("HEADTRACKING")))
	{
		FString val;
		if (FParse::Value(Cmd, TEXT("SOURCE="), val))
		{
			EnablePositionalTracking(false);
			//OSVRInterfaceName = val;
			EnablePositionalTracking(true);
		}
		if (FParse::Command(&Cmd, TEXT("ENABLE")))
		{
			EnablePositionalTracking(true);
			return true;
		}
		else if (FParse::Command(&Cmd, TEXT("DISABLE")))
		{
			EnablePositionalTracking(false);
			return true;
		}
	}

	return false;
}

bool FFoveHMD::EnablePositionalTracking(bool bEnable)
{
	UE_LOG(LogHMD, Warning, TEXT("FOVE does not support EnablePositionalTracking"));
	return true;
}

bool FFoveHMD::IsInLowPersistenceMode() const
{
	return true; // Not supported, game can think of us as always in low persistence mode
}

void FFoveHMD::EnableLowPersistenceMode(bool bEnable)
{
	UE_LOG(LogHMD, Warning, TEXT("FOVE does not support EnableLowPersistenceMode"));
}

#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18 && ENGINE_MINOR_VERSION < 20

void FFoveHMD::UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport, FRHIViewport* const ViewportRHI)
{
	// Supersedes UpdateViewport since 4.18
	// Replaced by GetActiveRenderBridge_GameThread in 4.20

	check(IsInGameThread());

	if (ViewportRHI)
	{
		if (Bridge && IsStereoEnabled())
		{
			ViewportRHI->SetCustomPresent(Bridge);
			Bridge->UpdateViewport(Viewport, ViewportRHI);
		}
		else
		{
			ViewportRHI->SetCustomPresent(nullptr);
		}
	}
}

#endif // ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18

void FFoveHMD::SetClippingPlanes(float NCP, float FCP)
{
	ZNear = NCP;
	ZFar = FCP;
}

void FFoveHMD::GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
	if (Context.View.StereoPass == eSSP_LEFT_EYE)
	{
		EyeToSrcUVOffsetValue.X = 0.0f;
		EyeToSrcUVOffsetValue.Y = 0.0f;
	}
	else
	{
		EyeToSrcUVOffsetValue.X = 0.5f;
		EyeToSrcUVOffsetValue.Y = 0.0f;
	}

	EyeToSrcUVScaleValue = FVector2D(0.5f, 1.0f);
}

bool FFoveHMD::IsStereoEnabled() const
{
	check(!bStereoEnabled || bHmdEnabled); // bHmdEnabled must be true for bStereoEnabled to be true
	return bStereoEnabled;
}

bool FFoveHMD::EnableStereo(const bool enable)
{
	check(IsInGameThread());

	// Early out
	if (enable == bStereoEnabled)
		return enable;

	// Don't allow enablement of stereo on while the headset is disabled (see comment in EnableHMD)
	if (!bHmdEnabled)
	{
		check(!bStereoEnabled);
		return false;
	}

	// Uncap fps to ensure we render at the framerate that FOVE needs
	GEngine->bForceDisableFrameRateSmoothing = enable;

	// Cache state of stereo enablement
	bStereoEnabled = enable;

	return bStereoEnabled;
}

void FFoveHMD::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{
	// This function determines the viewport when rendering
	// The incoming values here are the window size, so we ignore them and use the RT size which should match CalculateRenderTargetSize()
	if (Bridge->IsCompositorLayerValid())
	{
		const Fove::CompositorLayer layer = Bridge->GetCompositorLayer();
		SizeX = layer.idealResolutionPerEye.x;
		SizeY = layer.idealResolutionPerEye.y;
	}

	// Handle stereo
	if (StereoPass == eSSP_RIGHT_EYE)
	{
		X += SizeX;
	}
}

void FFoveHMD::InitCanvasFromView(FSceneView* InView, UCanvas* Canvas)
{
	// Couldn't find any other HMD plugins that do anything here
	// Leaving blank for now
}

void FFoveHMD::CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY)
{
	check(IsInGameThread());

	// This is used for the virtual render target, not the window backbuffer
	// We simply use the HMD size here and ignore the incoming size entirely
	// While this handles the render target size, AdjustViewRect will adjust the viewport accordingly as well
	if (Bridge->IsCompositorLayerValid())
	{
		const Fove::CompositorLayer layer = Bridge->GetCompositorLayer();
		InOutSizeX = layer.idealResolutionPerEye.x * 2;
		InOutSizeY = layer.idealResolutionPerEye.y;
	}
}

bool FFoveHMD::ShouldUseSeparateRenderTarget() const
{
	check(IsInGameThread());
	return IsStereoEnabled();
}

void FFoveHMD::CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, BEFORE_4_18(const) FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation)
{
	if (StereoPassType == eSSP_LEFT_EYE || StereoPassType == eSSP_RIGHT_EYE)
	{
		const float EyeOffset = GetInterpupillaryDistance() * WorldToMeters / (StereoPassType == eSSP_LEFT_EYE ? -2.0f : 2.0f);
		ViewLocation += ViewRotation.Quaternion().RotateVector(FVector(0, EyeOffset, 0));
	}
}

FMatrix FFoveHMD::GetStereoProjectionMatrix(enum EStereoscopicPass StereoPass BEFORE_4_18(FOVEHMD_COMMA const float FOV_ignored)) const
{
	check(IsStereoEnabled());

	// Query Fove SDK for projection matrix for this eye
	Fove::Result<Fove::Stereo<Fove::Matrix44>> Matrices = FoveHeadset->getProjectionMatricesLH(ZNear, ZFar);
	if (!Matrices)
	{
		UE_LOG(LogHMD, Warning, TEXT("Headset::getProjectionMatricesLH: %d"), static_cast<int>(Matrices.getError()));
		Matrices = Fove::Stereo<Fove::Matrix44>{};
	}

	// Convert to Unreal matrix and correct near/far clip (which use reversed-Z in Unreal)
	FMatrix Ret = ToUnreal(eSSP_LEFT_EYE ? Matrices->l : Matrices->r);
	Ret.M[3][3] = 0.0f;
	Ret.M[2][3] = 1.0f;
	Ret.M[2][2] = ZNear == ZFar ? 0.0f : ZNear / (ZNear - ZFar);
	Ret.M[3][2] = ZNear == ZFar ? ZNear : -ZFar * ZNear / (ZNear - ZFar);

	return Ret;
}

void FFoveHMD::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture AFTER_4_18(FOVEHMD_COMMA FVector2D WindowSize)) const
{
	check(IsInRenderingThread());

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 17
	if (SpectatorScreenController)
	{
		SpectatorScreenController->RenderSpectatorScreen_RenderThread(RHICmdList, BackBuffer, SrcTexture, WindowSize);
	}
#else
	const uint32 ViewportWidth = BackBuffer->GetSizeX();
	const uint32 ViewportHeight = BackBuffer->GetSizeY();

	// Set & clear the render target
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 14
		const ERenderTargetLoadAction loadAction = ERenderTargetLoadAction::ENoAction;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 16
		FRHIRenderTargetView target(BackBuffer, loadAction);
#else
		FRHIRenderTargetView target(BackBuffer);
		target.LoadAction = loadAction;
#endif
		RHICmdList.SetRenderTargetsAndClear(FRHISetRenderTargetsInfo(1, &target, FRHIDepthRenderTargetView()));
#else
		SetRenderTarget(RHICmdList, BackBuffer, FTextureRHIRef());
#endif

		RHICmdList.SetViewport(0, 0, 0, ViewportWidth, ViewportHeight, 1.0f);
	}

	// Get shaders
	const auto FeatureLevel = GMaxRHIFeatureLevel;
	auto ShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
	TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

	// Set render state
#ifdef FOVE_USE_PIPLINE_STATE_CACHE
	FGraphicsPipelineStateInitializer piplineState;
	RHICmdList.ApplyCachedRenderTargets(piplineState);
	piplineState.BlendState = TStaticBlendState<>::GetRHI();
	piplineState.RasterizerState = TStaticRasterizerState<>::GetRHI();
	piplineState.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	piplineState.BoundShaderState.VertexDeclarationRHI = RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI;
	piplineState.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	piplineState.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	piplineState.PrimitiveType = PT_TriangleList;

	SetGraphicsPipelineState(RHICmdList, piplineState);
#else
	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
	RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());

	static FGlobalBoundShaderState BoundShaderState;
	SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState, RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);
#endif

	// Set shader properties
	PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);

	// Draw a rectangle with the content of one or both of the eye images, depending on the mirror mode
	RendererModule->DrawRectangle(
		RHICmdList,
		0,                                                         // X
		0,                                                         // Y
		ViewportWidth,                                             // SizeX
		ViewportHeight,                                            // SizeY
		0.0f,                                                      // U
		0.0f,                                                      // V
		1.0f,                                                      // SizeU
		1.0f,                                                      // SizeV
		FIntPoint(ViewportWidth, ViewportHeight),
		FIntPoint(1, 1),
		*VertexShader,
		EDRF_Default);
#endif
}

bool FFoveHMD::NeedReAllocateViewportRenderTarget(const FViewport& Viewport)
{
	check(IsInGameThread());

	if (IsStereoEnabled())
	{
		const uint32 InSizeX = Viewport.GetSizeXY().X;
		const uint32 InSizeY = Viewport.GetSizeXY().Y;
		FIntPoint RenderTargetSize;
		RenderTargetSize.X = Viewport.GetRenderTargetTexture()->GetSizeX();
		RenderTargetSize.Y = Viewport.GetRenderTargetTexture()->GetSizeY();

		uint32 NewSizeX = InSizeX, NewSizeY = InSizeY;
		CalculateRenderTargetSize(Viewport, NewSizeX, NewSizeY);
		if (NewSizeX != RenderTargetSize.X || NewSizeY != RenderTargetSize.Y)
		{
			return true;
		}
	}
	return false;
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 21 // Removed in 4.21

void FFoveHMD::GetOrthoProjection(int32 RTWidth, int32 RTHeight, float OrthoDistance, FMatrix OrthoProjection[2]) const
{
	const float HudOffset = 50.0f;
	OrthoProjection[0] = FTranslationMatrix(FVector(HudOffset, 0.0f, 0.0f));
	OrthoProjection[1] = FTranslationMatrix(FVector(-HudOffset + RTWidth * 0.5f, 0.0f, 0.0f));
}

#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 18 // Removed in 4.18

void FFoveHMD::UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& InViewport, SViewport* ViewportWidget)
{
	// Replaced by UpdateViewportRHIBridge in 4.18

	check(IsInGameThread());

	const FViewportRHIRef& viewportRef = InViewport.GetViewportRHI();
	if (viewportRef)
	{
		if (Bridge && IsStereoEnabled())
		{
			viewportRef->SetCustomPresent(Bridge);
			Bridge->UpdateViewport(InViewport, viewportRef);
		}
		else
		{
			viewportRef->SetCustomPresent(nullptr);
		}
	}
}

#endif

void FFoveHMD::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	check(IsInGameThread());

	InViewFamily.EngineShowFlags.MotionBlur = 0;
	InViewFamily.EngineShowFlags.HMDDistortion = false;
	InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();
}

void FFoveHMD::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 17
	if (SpectatorScreenController)
	{
		SpectatorScreenController->BeginRenderViewFamily();
	}
#endif
}

void FFoveHMD::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	check(IsInGameThread());

	PrivOrientationAndPosition(InView.BaseHmdOrientation, InView.BaseHmdLocation);
	WorldToMetersScale = InView.WorldToMetersScale;
}

void FFoveHMD::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	check(IsInRenderingThread());

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 18
	// Update the view rotation with the latest value, sampled just beforehand in PreRenderViewFamily_RenderThread
	if (Bridge)
	{
		const FQuat DeltaOrient = InView.BaseHmdOrientation.Inverse() * Bridge->GetRenderPose().GetRotation();
		InView.ViewRotation = FRotator(InView.ViewRotation.Quaternion() * DeltaOrient);
		InView.UpdateViewMatrix();
	}
#endif
}

void FFoveHMD::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
	check(IsInRenderingThread());

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 17
	if (SpectatorScreenController)
	{
		SpectatorScreenController->UpdateSpectatorScreenMode_RenderThread();
	}
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 18 // Replaced by RefreshPoses() in 4.16

	if (Bridge)
	{
		// Blocks until the next time we need to render, as determined by the compositor, and fetches a new pose to use during rendering
		// This allows the compositor to cap rendering at exactly the frame rate needed, so we don't draw more frames than the compositor can use
		// Vsync and any other frame rate limiting options within Unreal should be disabled when using with FOVE to ensure this works well
		// This also lets us update the pose just before rendering, so time warp only needs to correct by a small amount
		const Fove::Result<Fove::Pose> FovePose = GetCompositor().waitForRenderPose();
		if (!FovePose)
		{
			UE_LOG(LogHMD, Warning, TEXT("Compositor::waitForRenderPose failed: %d"), static_cast<int>(FovePose.getError()));
		}
		else
		{
			// We will be moving the view location just before rendering, so camera-attached objects need a late update to stay locked to the view
			const FTransform LastPose = Bridge->GetRenderPose();
			Bridge->SetRenderPose(FovePose.getValue(), WorldToMetersScale);
			const FTransform NewPose = Bridge->GetRenderPose();

			ApplyLateUpdate(ViewFamily.Scene, LastPose, NewPose);
		}
	}
#endif
}

void FFoveHMD::PrivOrientationAndPosition(FQuat& OutOrientation, FVector& OutPosition)
{
	checkf(IsInGameThread(), TEXT("PrivOrientationAndPosition called from not game thread"));

	FTransform transform;
	if (Bridge)
	{
		transform = Bridge->GetRenderPose();
	}
	else
	{
		Fove::Result<Fove::Pose> Pose = FoveHeadset->getPose();
		if (!Pose)
		{
			UE_LOG(LogHMD, Warning, TEXT("Headset::getLatestPose failed: %d"), static_cast<int>(Pose.getError()));
			Pose = Fove::Pose{};
		}

		transform = ToUnreal(Pose.getValue(), WorldToMetersScale);
	}

	OutOrientation = transform.GetRotation();
	OutPosition = transform.GetLocation();
}

#ifdef _MSC_VER
#pragma endregion
#endif
