#pragma once

/*
* Function library for accessing data from the Fove SDK via blueprints
* All functions in here simply forward to FFoveHMD, and in turn to the underlying FOVE C++ API
* For documentation, please see the corresponding functions in FoveAPI.h
*/

#include "IFoveHMDPlugin.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Quat.h"
#include "FoveVRFunctionLibrary.generated.h"

// Unfortunately UEnum doesn't allow values over 255 so we can't map this exactly to the FOVE API values
UENUM(BlueprintType)
enum class EFoveErrorCode : uint8
{
	None,
	Connect_NotConnected,
	Connect_RuntimeVersionTooOld,
	Connect_ClientVersionTooOld,
	API_InvalidArgument,
	API_NotRegistered,
	API_NullInPointer,
	API_InvalidEnumValue,
	API_NullOutPointersOnly,
	API_OverlappingOutPointers,
	API_MissingArgument,
	API_Timeout,
	Data_Unreadable,
	Data_NoUpdate,
	Data_Uncalibrated,
	Data_Unreliable,
	Data_LowAccuracy,
	Hardware_Disconnected,
	Hardware_WrongFirmwareVersion,
	Code_NotImplementedYet,
	Code_FunctionDeprecated,
	Position_ObjectNotTracked,
	Compositor_NotSwapped,
	Compositor_UnableToCreateDeviceAndContext,
	Compositor_UnableToUseTexture,
	Compositor_DeviceMismatch,
	Compositor_DisconnectedFromRuntime,
	Compositor_ErrorCreatingTexturesOnDevice,
	Compositor_NoEyeSpecifiedForSubmit,
	UnknownError,
	Object_AlreadyRegistered,
	Calibration_OtherRendererPrioritized,
	License_FeatureAccessDenied,
	Profile_DoesntExist,
	Profile_NotAvailable,
	Profile_InvalidName,
	Config_DoesntExist,
	Config_TypeMismatch,

	// Unreal specific things not normally in the API
	Unreal_NoFoveHMD,
	Unreal_UnknownError,
};

UENUM(BlueprintType)
enum class EFoveEye : uint8
{
	Left,
	Right,
};

UENUM(BlueprintType)
enum class EFoveEyeState : uint8
{
	NotDetected,
	Opened,
	Closed,
};

UENUM(BlueprintType)
enum class EFoveGazeCastPolicy : uint8
{
	DismissBothEyeClosed,
	DismissOneEyeClosed,
	NeverDismiss,
};

UENUM(BlueprintType)
enum class EFoveLogLevel : uint8
{
	Debug,
	Warning,
	Error,
};

UENUM(BlueprintType)
enum class EFoveClientCapabilities : uint8
{
	OrientationTracking,
	PositionTracking,
	PositionImage,
	EyeTracking,
	GazeDepth,
	UserPresence,
	UserAttentionShift,
	UserIOD,
	UserIPD,
	EyeTorsion,
	EyeShape,
	EyesImage,
	EyeballRadius,
	IrisRadius,
	PupilRadius,
	GazedObjectDetection,
	DirectScreenAccess,
};

UENUM(BlueprintType)
enum class EFoveCalibrationState : uint8
{
	NotStarted,
	HeadsetAdjustment,
	WaitingForUser,
	CollectingData,
	ProcessingData,
	Successful_HighQuality,
	Successful_MediumQuality,
	Successful_LowQuality,
	Failed_Unknown,
	Failed_InaccurateData,
	Failed_NoRenderer,
	Failed_NoUser,
	Failed_Aborted,
};

UENUM(BlueprintType)
enum class EFoveCalibrationMethod : uint8
{
	Default,
	OnePoint,
	Spiral,
	OnePointWithNoGlassesSpiralWithGlasses,
};

UENUM(BlueprintType)
enum class EFoveEyeByEyeCalibration : uint8
{
	Default,
	Disabled,
	Enabled,
};

UENUM(BlueprintType)
enum class EFoveObjectGroup : uint8
{
	Group0,
	Group1,
	Group2,
	Group3,
	Group4,
	Group5,
	Group6,
	Group7,
	Group8,
	Group9,
	Group10,
	Group11,
	Group12,
	Group13,
	Group14,
	Group15,
	Group16,
	Group17,
	Group18,
	Group19,
	Group20,
	Group21,
	Group22,
	Group23,
	Group24,
	Group25,
	Group26,
	Group27,
	Group28,
	Group29,
	Group30,
	Group31,
};

USTRUCT(BlueprintType)
struct FFoveFrameTimestamp
{
	GENERATED_USTRUCT_BODY()

	// Unreal doesn't support 64bit integers in blueprints

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int32 IdLower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int32 IdUpper = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int32 TimestampLower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int32 TimestampUpper = 0;
};

USTRUCT(BlueprintType)
struct FFoveProjectionParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		float Left = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		float Right = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		float Top = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		float Bottom = -1;
};

USTRUCT(BlueprintType)
struct FFoveVersions
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int ClientMajor = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int ClientMinor = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int ClientBuild = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int ClientProtocol = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int RuntimeMajor = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int RuntimeMinor = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int RuntimeBuild = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int Firmware = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int MaxFirmware = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int MinFirmware = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		bool TooOldHeadsetConnected = false;
};

USTRUCT(BlueprintType)
struct FFoveHeadsetHardwareInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FString SerialNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FString Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FString ModelName;
};

USTRUCT(BlueprintType)
struct FFoveEyeShape
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline00;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline04;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline05;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline06;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline07;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline08;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline09;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector2D Outline11;
};

USTRUCT(BlueprintType)
struct FFoveCalibrationOptions
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		bool Lazy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		bool Restart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		EFoveEyeByEyeCalibration EyeByEye = EFoveEyeByEyeCalibration::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		EFoveCalibrationMethod Method = EFoveCalibrationMethod::Default;
};

USTRUCT(BlueprintType)
struct FFoveCalibrationTarget
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		float RecommendedSize;
};

USTRUCT(BlueprintType)
struct FFoveCalibrationData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		EFoveCalibrationMethod Method = EFoveCalibrationMethod::Spiral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		EFoveCalibrationState State = EFoveCalibrationState::NotStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FFoveCalibrationTarget targetL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FFoveCalibrationTarget targetR;
};

USTRUCT(BlueprintType)
struct FFoveObjectPose
{
	GENERATED_USTRUCT_BODY()

	FVector Scale = {1, 1, 1};
	FQuat Rotation;
	FVector Position;
	FVector Velocity;
};

USTRUCT(BlueprintType)
struct FFoveObjectCollider
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FVector Center;
	//Fove_ColliderType FOVE_STRUCT_VAL(shapeType, {}); FIXME
	//Fove_ColliderCube cube;
	//Fove_ColliderSphere sphere;
	//Fove_ColliderMesh mesh;
};

USTRUCT(BlueprintType)
struct FFoveGazableObject
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FFoveObjectPose Pose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		EFoveObjectGroup Group = EFoveObjectGroup::Group0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		TArray<FFoveObjectCollider> Collider;
};

USTRUCT(BlueprintType)
struct FFoveCameraObject
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		FFoveObjectPose Pose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
		int GroupMask = 0xffffffff;
};

UENUM(BlueprintType)
enum class EFoveIsValid : uint8
{
	IsValid,
	IsNotValid,
};

UENUM(BlueprintType)
enum class EFoveIsReliable : uint8
{
	IsReliable,
	IsNotReliable,
};

UCLASS()
class UFoveVRFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsHardwareConnected(EFoveErrorCode& OutErrorCode, bool& OutIsHardwareConnected);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsHardwareReady(EFoveErrorCode& OutErrorCode, bool& OutIsHardwareReady);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsMotionReady(EFoveErrorCode& OutErrorCode, bool& OutIsMotionReady);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void CheckSoftwareVersions(EFoveErrorCode& OutErrorCode);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetSoftwareVersions(EFoveErrorCode& OutErrorCode, FFoveVersions& OutVersions);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetHeadsetHardwareInfo(EFoveErrorCode& OutErrorCode, FFoveHeadsetHardwareInfo& OutHardwareInfo);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void RegisterCapabilities(EFoveErrorCode& OutErrorCode, const TArray<EFoveClientCapabilities>& Caps);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void UnregisterCapabilities(EFoveErrorCode& OutErrorCode, const TArray<EFoveClientCapabilities>& Caps);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void WaitAndFetchNextEyeTrackingData(EFoveErrorCode& OutErrorCode, FFoveFrameTimestamp& OutTimestamp);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void FetchEyeTrackingData(EFoveErrorCode& OutErrorCode, FFoveFrameTimestamp& OutTimestamp);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetGazeVector(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FVector& OutVector);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetGazeScreenPosition(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FVector2D& OutVector);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetCombinedGazeRay(EFoveErrorCode& OutErrorCode, FVector& OutCombinedGazeRayOrigin, FVector& OutCombinedGazeRayDirection);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetCombinedGazeDepth(EFoveErrorCode& OutErrorCode, float& OutCombinedGazeDepth);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsUserShiftingAttention(EFoveErrorCode& OutErrorCode, bool& OutIsUserShiftingAttention);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetEyeState(EFoveErrorCode& OutErrorCode, EFoveEye Eye, EFoveEyeState& OutEyeState);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsEyeTrackingEnabled(EFoveErrorCode& OutErrorCode, bool& OutIsEyeTrackingEnabled);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsEyeTrackingCalibrated(EFoveErrorCode& OutErrorCode, bool& OutIsEyeTrackingCalibrated);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsEyeTrackingCalibrating(EFoveErrorCode& OutErrorCode, bool& OutIsEyeTrackingCalibrating);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsEyeTrackingCalibratedForGlasses(EFoveErrorCode& OutErrorCode, bool& OutIsEyeTrackingCalibratedForGlasses);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsHmdAdjustmentGuiVisible(EFoveErrorCode& OutErrorCode, bool& OutIsHmdAdjustmentGuiVisible);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void HasHmdAdjustmentGuiTimeout(EFoveErrorCode& OutErrorCode, bool& OutHasHmdAdjustmentGuiTimeout);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsEyeTrackingReady(EFoveErrorCode& OutErrorCode, bool& OutIsEyeTrackingReady);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsUserPresent(EFoveErrorCode& OutErrorCode, bool& OutIsUserPresent);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetEyesImage(EFoveErrorCode& OutErrorCode, UTexture*& OutEyesImage);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetUserIPD(EFoveErrorCode& OutErrorCode, float& OutGetUserIPD);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetUserIOD(EFoveErrorCode& OutErrorCode, float& OutGetUserIOD);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetPupilRadius(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& OutPupilRadius);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetIrisRadius(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& OutIrisRadius);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetEyeballRadius(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& OutEyeballRadius);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetEyeTorsion(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float& OutEyeTorsion);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetEyeShape(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FFoveEyeShape& OutEyeShape);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void StartEyeTrackingCalibration(EFoveErrorCode& OutErrorCode, const FFoveCalibrationOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void StopEyeTrackingCalibration(EFoveErrorCode& OutErrorCode);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetEyeTrackingCalibrationState(EFoveErrorCode& OutErrorCode, EFoveCalibrationState& OutCalibrationState);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void TickEyeTrackingCalibration(EFoveErrorCode& OutErrorCode, float Dt, bool IsVisible, FFoveCalibrationData& OutData);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetGazedObjectId(EFoveErrorCode& OutErrorCode, int& OutGazesObjectId);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void RegisterGazableObject(EFoveErrorCode& OutErrorCode, const FFoveGazableObject& Object);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void UpdateGazableObject(EFoveErrorCode& OutErrorCode, int ObjectId, const FFoveObjectPose& ObjectPose);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void RemoveGazableObject(EFoveErrorCode& OutErrorCode, int ObjectId);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void RegisterCameraObject(EFoveErrorCode& OutErrorCode, const FFoveCameraObject& Camera);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void UpdateCameraObject(EFoveErrorCode& OutErrorCode, int cameraId, const FFoveObjectPose& Pose);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void RemoveCameraObject(EFoveErrorCode& OutErrorCode, int CameraId);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetGazeCastPolicy(EFoveErrorCode& OutErrorCode, EFoveGazeCastPolicy& OutPolicy);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void SetGazeCastPolicy(EFoveErrorCode& OutErrorCode,  EFoveGazeCastPolicy Policy);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void TareOrientationSensor(EFoveErrorCode& OutErrorCode);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void IsPositionReady(EFoveErrorCode& OutErrorCode, bool& OutIsPositionReady);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void TarePositionSensors(EFoveErrorCode& OutErrorCode);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void FetchPoseData(EFoveErrorCode& OutErrorCode, FFoveFrameTimestamp& OutTimestamp);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetPose(EFoveErrorCode& OutErrorCode, FTransform& OutPose);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetPositionImage(EFoveErrorCode& OutErrorCode, UTexture*& OutPositionImage);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetProjectionMatrix(EFoveErrorCode& OutErrorCode, EFoveEye Eye, float ZNear, float ZFar, FMatrix& OutMatrix);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetRawProjectionValues(EFoveErrorCode& OutErrorCode, EFoveEye Eye, FFoveProjectionParams& OutProjectionParams);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetRenderIOD(EFoveErrorCode& OutErrorCode, float& outRenderIOD);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void CreateProfile(EFoveErrorCode& OutErrorCode, const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void RenameProfile(EFoveErrorCode& OutErrorCode, const FString& OldName, const FString& NewName);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void DeleteProfile(EFoveErrorCode& OutErrorCode, const FString& ProfileName);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void ListProfiles(EFoveErrorCode& OutErrorCode, TArray<FString>& OutProfiles);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void SetCurrentProfile(EFoveErrorCode& OutErrorCode, const FString& ProfileName);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetCurrentProfile(EFoveErrorCode& OutErrorCode, FString& OutCurrentProfile);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetProfileDataPath(EFoveErrorCode& OutErrorCode, const FString& ProfileName, FString& OutPath);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void HasAccessToFeature(EFoveErrorCode& OutErrorCode, const FString& InFeatureName, bool& OutHasAccessToFeature);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetConfigBool(EFoveErrorCode& OutErrorCode, const FString& Key, bool& OutValue);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetConfigInt(EFoveErrorCode& OutErrorCode, const FString& Key, int& OutValue);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetConfigFloat(EFoveErrorCode& OutErrorCode, const FString& Key, float& OutValue);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void GetConfigString(EFoveErrorCode& OutErrorCode, const FString& Key, FString& OutValue);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void SetConfigValueBool(EFoveErrorCode& OutErrorCode, const FString& Key, bool Value);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void SetConfigValueInt(EFoveErrorCode& OutErrorCode, const FString& Key, int Value);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void SetConfigValueFloat(EFoveErrorCode& OutErrorCode, const FString& Key, float Value);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void SetConfigValueString(EFoveErrorCode& OutErrorCode, const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void ClearConfigValue(EFoveErrorCode& OutErrorCode, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "FoveVR")
		static void LogText(EFoveErrorCode& OutErrorCode, EFoveLogLevel Level, const FString& Text);

	UFUNCTION(BlueprintPure, Category = "FoveVR")
		static void IsValid(EFoveErrorCode ErrorCode, bool& OutIsValid);

	UFUNCTION(BlueprintCallable, Category = "FoveVR", Meta = (ExpandEnumAsExecs = "OutBranch"))
		static void BranchOnIsValid(EFoveErrorCode ErrorCode, EFoveIsValid& OutBranch, EFoveErrorCode& OutErrorCode, bool& OutIsValid, bool& OutIsReliable);

	UFUNCTION(BlueprintPure, Category = "FoveVR")
		static void IsReliable(EFoveErrorCode ErrorCode, bool& OutIsReliable);

	UFUNCTION(BlueprintCallable, Category = "FoveVR", Meta = (ExpandEnumAsExecs = "OutBranch"))
		static void BranchOnIsReliable(EFoveErrorCode ErrorCode, EFoveIsReliable& OutBranch, EFoveErrorCode& OutErrorCode, bool& OutIsValid, bool& OutIsReliable);
};
