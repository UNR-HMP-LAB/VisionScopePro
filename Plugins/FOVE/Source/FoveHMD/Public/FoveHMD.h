#pragma once

// Unreal headeers
#include "Engine.h"
#include "Templates/RefCounting.h"
#include "Runtime/Launch/Resources/Version.h"

// Fixme: Remove this once the API is fully fully wrapped
#include "FoveAPI.h"

// Local headers
#include "FoveVRFunctionLibrary.h"

// Make sure that we have the macros needed to specialize the build for different versions of Unreal Engine
#if !defined(ENGINE_MAJOR_VERSION) || !defined(ENGINE_MINOR_VERSION)
static_assert(false, "Unable to find Unreal version macros");
#endif

// Check the allowed version
// If this check fails, this version of unreal is not supported, but patches are welcome!
#if ENGINE_MAJOR_VERSION != 4 || ENGINE_MINOR_VERSION > 26 || ENGINE_MINOR_VERSION < 12
static_assert(false, "This version of the Fove Unreal plugin only supports Unreal 4.12 through 4.26");
#endif

// Determine the base classes for FFoveHMD
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 16
#include "HeadMountedDisplayBase.h"
#define FOVEHMD_BASE_CLASS FHeadMountedDisplayBase
#else
#include "HeadMountedDisplay.h"
#define FOVEHMD_BASE_CLASS IHeadMountedDisplay
#endif
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
#include "SceneViewExtension.h"
#include "XRRenderTargetManager.h"
#define FOVEHMD_SCENE_EXTENSION_BASE_CLASS FSceneViewExtensionBase
#define FOVEHMD_EXTRA_BASE_CLASS FXRRenderTargetManager
#else
#include "SceneViewExtension.h"
#define FOVEHMD_SCENE_EXTENSION_BASE_CLASS ISceneViewExtension
#define FOVEHMD_EXTRA_BASE_CLASS TSharedFromThis<FFoveHMD, ESPMode::ThreadSafe>
#endif

#define FOVEHMD_COMMA ,

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 25
#define BEFORE_4_25(x)
#define AFTER_4_25(x) x
#else
#define BEFORE_4_25(x) x
#define AFTER_4_25(x)
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
#define AFTER_4_23(x) x
#else
#define AFTER_4_23(x)
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20
#define BEFORE_4_20(x)
#else
#define BEFORE_4_20(x) x
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
#define AFTER_4_18(x) x
#define BEFORE_4_18(x)
#else
#define AFTER_4_18(x)
#define BEFORE_4_18(x) x
#endif

// Forward declarations
struct ID3D11Texture2D;
class FoveRenderingBridge;
class IRendererModule;
struct Fove_CompositorLayer;
namespace Fove
{
	// This header intentionally doesn't include FoveAPI.h because
	// 1. Possibly slightly faster compile times (it pulls in the standard library which is normally not used in Unreal)
	// 2. Want to encourage using the wrappers here
	// So we forward declare the bits that we need from the API
	class Headset;
	class Compositor;
}

class FOVEHMD_API FFoveHMD : public FOVEHMD_BASE_CLASS, public FOVEHMD_SCENE_EXTENSION_BASE_CLASS, public FOVEHMD_EXTRA_BASE_CLASS
{
public: // Generic

	// Construction / destruction
	FFoveHMD(AFTER_4_18(const FAutoRegister& AutoRegister FOVEHMD_COMMA) TSharedRef<Fove::Headset, ESPMode::ThreadSafe> headset, Fove::Compositor compositor);
	~FFoveHMD() override;

	// Helper to return the global FFoveHMD object
	// Returns null if there is no HMD device, or if the current HMD device is not an FFoveHMD
	static FFoveHMD* Get();

public: // FOVE API Wrapper

	// Getters for the FOVE C++ API objects
	// The full FOVE C++ API can be directly accessed via these helpers
	// Generally, you should go through the wrappers below though
	Fove::Headset&       GetHeadset()       { return *FoveHeadset; }
	Fove::Headset const& GetHeadset() const { return *FoveHeadset; }
	Fove::Compositor&       GetCompositor()       { return *FoveCompositor; }
	Fove::Compositor const& GetCompositor() const { return *FoveCompositor; }

	// The functions in this section wrap the FOVE API with the following exceptions
	//   1. Types are converted to Unreal types (std::vector -> FArray, etc)
	//   2. Coordinate systems conversion using World->WorldToMeters and axis change from FOVE's XYZ -> Unreal's YZX
	//   3. Naming conventions are updated to Unreal conventions
	// Please see FoveAPI.h for documentation on each function

	EFoveErrorCode IsHardwareConnected(bool &OutIsHardwareConnected);
	EFoveErrorCode IsHardwareReady(bool &OutIsHardwareReady);
	EFoveErrorCode IsMotionReady(bool &OutIsMotionReady);
	EFoveErrorCode CheckSoftwareVersions();
	EFoveErrorCode GetSoftwareVersions(FFoveVersions& OutVersions);
	EFoveErrorCode GetHeadsetHardwareInfo(FFoveHeadsetHardwareInfo& OutHardwareInfo);
	EFoveErrorCode RegisterCapabilities(const TArray<EFoveClientCapabilities>& Caps);
	EFoveErrorCode UnregisterCapabilities(const TArray<EFoveClientCapabilities>& Caps);
	EFoveErrorCode WaitAndFetchNextEyeTrackingData(FFoveFrameTimestamp& OutTimestamp);
	EFoveErrorCode FetchEyeTrackingData(FFoveFrameTimestamp& OutTimestamp);
	EFoveErrorCode GetGazeVector(EFoveEye Eye, FVector& OutVector);
	EFoveErrorCode GetGazeScreenPosition(EFoveEye Eye, FVector2D& OutVector);
	EFoveErrorCode GetCombinedGazeRay(FVector& OutCombinedGazeRayOrigin, FVector& OutCombinedGayRayDirection);
	EFoveErrorCode GetCombinedGazeDepth(float& OutCombinedGazeDepth);
	EFoveErrorCode IsUserShiftingAttention(bool& OutIsUserShiftingAttention);
	EFoveErrorCode GetEyeState(EFoveEye Eye, EFoveEyeState& OutEyeState);
	EFoveErrorCode IsEyeTrackingEnabled(bool& OutIsEyeTrackingEnabled);
	EFoveErrorCode IsEyeTrackingCalibrated(bool& OutIsEyeTrackingCalibrated);
	EFoveErrorCode IsEyeTrackingCalibrating(bool& OutIsEyeTrackingCalibrating);
	EFoveErrorCode IsEyeTrackingCalibratedForGlasses(bool& OutIsEyeTrackingCalibratedForGlasses);
	EFoveErrorCode IsHmdAdjustmentGuiVisible(bool& OutIsHmdAdjustmentGuiVisible);
	EFoveErrorCode HasHmdAdjustmentGuiTimeout(bool& OutHasHmdAdjustmentGuiTimeout);
	EFoveErrorCode IsEyeTrackingReady(bool& OutIsEyeTrackingReady);
	EFoveErrorCode IsUserPresent(bool& OutIsUserPresent);
	EFoveErrorCode GetEyesImage(UTexture*& OutEyesImage);
	EFoveErrorCode GetUserIPD(float& OutGetUserIPD);
	EFoveErrorCode GetUserIOD(float& OutGetUserIOD);
	EFoveErrorCode GetPupilRadius(EFoveEye Eye, float& OutPupilRadius);
	EFoveErrorCode GetIrisRadius(EFoveEye Eye, float& OutIrisRadius);
	EFoveErrorCode GetEyeballRadius(EFoveEye Eye, float& OutEyeballRadius);
	EFoveErrorCode GetEyeTorsion(EFoveEye Eye, float& OutEyeTorsion);
	EFoveErrorCode GetEyeShape(EFoveEye Eye, FFoveEyeShape& OutEyeShape);
	EFoveErrorCode StartEyeTrackingCalibration(const FFoveCalibrationOptions& Options);
	EFoveErrorCode StopEyeTrackingCalibration();
	EFoveErrorCode GetEyeTrackingCalibrationState(EFoveCalibrationState& OutCalibrationState);
	EFoveErrorCode TickEyeTrackingCalibration(float Dt, bool IsVisible, FFoveCalibrationData& OutData);
	EFoveErrorCode GetGazedObjectId(int& OutGazesObjectId);
	EFoveErrorCode RegisterGazableObject(const FFoveGazableObject& Object);
	EFoveErrorCode UpdateGazableObject(int ObjectId, const FFoveObjectPose& ObjectPose);
	EFoveErrorCode RemoveGazableObject(int ObjectId);
	EFoveErrorCode RegisterCameraObject(const FFoveCameraObject& Camera);
	EFoveErrorCode UpdateCameraObject(int cameraId, const FFoveObjectPose& Pose);
	EFoveErrorCode RemoveCameraObject(int CameraId);
	EFoveErrorCode GetGazeCastPolicy(EFoveGazeCastPolicy& OutPolicy);
	EFoveErrorCode SetGazeCastPolicy(EFoveGazeCastPolicy Policy);
	EFoveErrorCode TareOrientationSensor();
	EFoveErrorCode IsPositionReady(bool& OutIsPositionReady);
	EFoveErrorCode TarePositionSensors();
	EFoveErrorCode FetchPoseData(FFoveFrameTimestamp& OutTimestamp);
	EFoveErrorCode GetPose(FTransform& OutPose);
	EFoveErrorCode GetPositionImage(UTexture*& OutPositionImage);
	EFoveErrorCode GetProjectionMatrix(EFoveEye Eye, float ZNear, const float ZFar, FMatrix& OutMatrix);
	EFoveErrorCode GetRawProjectionValues(EFoveEye Eye, FFoveProjectionParams& OutProjectionParams);
	EFoveErrorCode GetRenderIOD(float& outRenderIOD);
	EFoveErrorCode CreateProfile(const FString& Name);
	EFoveErrorCode RenameProfile(const FString& OldName, const FString& NewName);
	EFoveErrorCode DeleteProfile(const FString& ProfileName);
	EFoveErrorCode ListProfiles(TArray<FString>& OutProfiles);
	EFoveErrorCode SetCurrentProfile(const FString& ProfileName);
	EFoveErrorCode GetCurrentProfile(FString& OutCurrentProfile);
	EFoveErrorCode GetProfileDataPath(const FString& ProfileName, FString& OutPath);
	EFoveErrorCode HasAccessToFeature(const FString& InFeatureName, bool& OutHasAccessToFeature);

	// Do not need an instance for these
	static EFoveErrorCode GetConfigBool(const FString& Key, bool& OutValue);
	static EFoveErrorCode GetConfigInt(const FString& Key, int& OutValue);
	static EFoveErrorCode GetConfigFloat(const FString& Key, float& OutValue);
	static EFoveErrorCode GetConfigString(const FString& Key, FString& OutValue);
	static EFoveErrorCode SetConfigValueBool(const FString& Key, bool Value);
	static EFoveErrorCode SetConfigValueInt(const FString& Key, int Value);
	static EFoveErrorCode SetConfigValueFloat(const FString& Key, float Value);
	static EFoveErrorCode SetConfigValueString(const FString& Key, const FString& Value);
	static EFoveErrorCode ClearConfigValue(const FString& Key);
	static EFoveErrorCode LogText(EFoveLogLevel Level, const FString& Text);
	static bool IsValid(EFoveErrorCode);
	static bool IsReliable(EFoveErrorCode);

public: // Utility functions

	// When "Force start in VR" setting is enabled (UFoveVRSettings::bForceStartInVR), IsHMDConnected function will always return true
	// IsHMDReallyConnected can then be used to query the actual connection status
	bool IsHMDReallyConnected();

public: // IXRTrackingSystem / FXRTrackingSystemBase interface

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26
	int32 GetXRSystemFlags() const override;
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	IHeadMountedDisplay* GetHMDDevice() override { return this; }
	TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> GetStereoRenderingDevice() override { return SharedThis(this); }
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20
	FXRRenderBridge* GetActiveRenderBridge_GameThread(bool bUseSeparateRenderTarget) override;
#endif

public: // IHeadMountedDisplay / FHeadMountedDisplayBase interface

	// 4.18 and later
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	FName GetSystemName() const override;
	bool EnumerateTrackedDevices(TArray<int, FDefaultAllocator>&, EXRTrackedDeviceType) override;
	void RefreshPoses() override;
	bool GetCurrentPose(int32, FQuat&, FVector&) override;
	float GetWorldToMetersScale() const override { return WorldToMetersScale; }
	bool OnStartGameFrame(FWorldContext& WorldContext) override;
	void CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* SrcTexture, FIntRect SrcRect, FRHITexture2D* DstTexture, FIntRect DstRect, bool bClearBlack, bool bNoAlpha) const override;
	FIntRect GetFullFlatEyeRect_RenderThread(FTexture2DRHIRef EyeTexture) const override;
#endif

	// 4.13 through 4.17
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 13 && ENGINE_MINOR_VERSION < 18
	FName GetDeviceName() const override;
#endif

	// 4.12 and later
	bool IsHMDConnected() override;
	bool IsHMDEnabled() const override;
	void EnableHMD(bool allow = true) override;
	bool GetHMDMonitorInfo(MonitorInfo&) override;
	void GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const override;
	bool IsChromaAbCorrectionEnabled() const override;
	void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
	float GetInterpupillaryDistance() const override;
	bool DoesSupportPositionalTracking() const override;
	bool HasValidTrackingPosition() override;
	void RebaseObjectOrientationAndPosition(FVector& Position, FQuat& Orientation) const override;
	bool IsHeadTrackingAllowed() const override;
	void ResetOrientationAndPosition(float yaw = 0.f) override;
	void ResetOrientation(float Yaw = 0.f) override;
	void ResetPosition() override;
	void SetBaseRotation(const FRotator& BaseRot) override;
	FRotator GetBaseRotation() const override;
	void SetBaseOrientation(const FQuat& BaseOrient) override;
	FQuat GetBaseOrientation() const override;
	void OnBeginPlay(FWorldContext& InWorldContext) override;
	void OnEndPlay(FWorldContext& InWorldContext) override;
	void SetTrackingOrigin(EHMDTrackingOrigin::Type NewOrigin) override;
	EHMDTrackingOrigin::Type GetTrackingOrigin() AFTER_4_23(const) override;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 19 // Removed in 4.19
	EHMDDeviceType::Type GetHMDDeviceType() const override { return EHMDDeviceType::DT_ES2GenericStereoMesh; }
#endif
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 18 // Removed in 4.18
	void GetPositionalTrackingCameraProperties(FVector& OutOrigin, FQuat& OutOrientation, float& OutHFOV, float& OutVFOV, float& OutCameraDistance, float& OutNearPlane, float& OutFarPlane) const override;
	void GetCurrentOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition) override;
	TSharedPtr<ISceneViewExtension, ESPMode::ThreadSafe> GetViewExtension() override;
	void ApplyHmdRotation(APlayerController* PC, FRotator& ViewRotation) override;
	bool UpdatePlayerCamera(FQuat& CurrentOrientation, FVector& CurrentPosition) override;
	bool IsPositionalTrackingEnabled() const override;
#endif
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 16 // Removed in 4.16
	bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	bool EnablePositionalTracking(bool enable) override;
	bool IsInLowPersistenceMode() const override;
	void EnableLowPersistenceMode(bool Enable = true) override;
#endif

public: // IStereoRendering interface

	// 4.18 and later
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 18
	IStereoRenderTargetManager* GetRenderTargetManager() override { return this; }

#if ENGINE_MINOR_VERSION < 20
	void UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport, FRHIViewport* const ViewportRHI) override;
#endif
#endif

	// 4.12 and later
	void SetClippingPlanes(float NCP, float FCP) override;
	void GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
	bool IsStereoEnabled() const override;
	bool EnableStereo(bool stereo = true) override;
	void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
	void InitCanvasFromView(FSceneView* InView, UCanvas* Canvas) override;
	void CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) override;
	bool ShouldUseSeparateRenderTarget() const override;
	void CalculateStereoViewOffset(EStereoscopicPass StereoPassType, BEFORE_4_18(const) FRotator& ViewRotation, const float MetersToWorld, FVector& ViewLocation) override;
	FMatrix GetStereoProjectionMatrix(EStereoscopicPass StereoPassType BEFORE_4_18(FOVEHMD_COMMA const float FOV)) const override;
	void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture AFTER_4_18(FOVEHMD_COMMA FVector2D WindowSize)) const override;
	bool NeedReAllocateViewportRenderTarget(const FViewport& Viewport) override;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 21 // Removed in 4.21
	void GetOrthoProjection(int32 RTWidth, int32 RTHeight, float OrthoDistance, FMatrix OrthoProjection[2]) const override;
#endif
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 18 // Removed in 4.18
	void UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, SViewport*) override;
#endif

public: // ISceneViewExtension interface

	void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
	void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;

private: // Implementation details

	void PrivOrientationAndPosition(FQuat& OutOrientation, FVector& OutPosition);

	// Number of "world" units in one meter
	float WorldToMetersScale = 1;

	// Distance to the clip planes. This can be set by the game
	float ZNear = 1.0f;
	float ZFar = 1.0f; // If this is equal to ZNear then there is no far clip
	
	FQuat    BaseOrientation = FQuat::Identity;
	FRotator AppliedHmdOrientation = FRotator(0, 0, 0);
	FRotator ControlRotation = FRotator(0, 0, 0);

	const TSharedRef<Fove::Headset, ESPMode::ThreadSafe> FoveHeadset;
	const TSharedRef<Fove::Compositor, ESPMode::ThreadSafe> FoveCompositor;
	IRendererModule* RendererModule = nullptr;

	bool bHmdEnabled = true;
	bool bStereoEnabled = false;

	// The rendering bridge used to submit to the FOVE compositor
	// This is a reference as a hack around sporatic build fails on 4.17+MSVC due to ~FoveRenderingBridge not being defined yet.
	// Even though a forward declaration should be perfectly fine since ~TRefCountPtr<FoveRenderingBridge> is not instanciated until after FoveRenderingBridge is declared...
	TRefCountPtr<FoveRenderingBridge>& Bridge;
};

/*
TODO:
Note from Unreal:
The last comment on this file has to do with eye tracking.  While there's nothing technically wrong with your implementation, there are a few Unreal-isms that you might be able to take advantage of in order to get a more naturally integrated plugin.

Right now, you expose the eye tracking parameters to the player through some functions, which return locations and rotations.  That's useful, but for many users, especially ones that use Blueprint more extensively, it's often more natural and useful for them to think in terms of component based design.  In Unreal, you can have something called SceneComponent, which basically means something that you can add to your Actor, which has a transformation associated with it.  Most things that you can see in the game are components...meshes, sprites, etc.  You can composite them together in Actors in order to create more complex Actors, and they follow a parenting hierarchy.  Because of that, it makes it very easy to deal with them in whichever space you want...component, actor, or world space.

For your eye tracking interface, I think it's fine to leave accessors to get the transforms directly, but you might also consider making a new class based off of SceneComponent, whose job it is to simply modify it's orientation and position to match that of the user's eyes.  That will give users something physical in the world to represent the eye position, and all their standard functions (GetComponentPosition and Orientation, GetComponentWorldOrientation, etc) will also still work.  It also lets them attach things directly to the eye, if they wanted to do a gaze cursor!  All they'd have to do is attach to the eye tracking component, and the rest would be automatically updated.

In the typical set up, I'd imagine that the player would have their character actor, which would in turn have its normal camera, which gets updated by the location and orientation of the HMD.  That part works now!  To do eye tracking, all they would have to do was attach two of your new FoveEyeTrackingComponents, one for each eye, to the camera, and then everything is done!

In order to do this, I suggest looking at MotionControllerComponent, which updates its relative position and orientation constantly based on the position and orientation of the motion controller.  The same could be used for the eye tracking component.  The relative position of the component would be updated to be half of the IPD offset to the left, and the orientation would be the face-space orientation of the eye.  That's it, and it would be a very natural extension of the Unreal component system.
*/
