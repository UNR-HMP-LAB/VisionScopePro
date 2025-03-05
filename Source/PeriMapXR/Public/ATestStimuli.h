// TestStimuli.h

#pragma once

#include "CoreMinimal.h"
#include "PXR_MotionTracking.h"
#include "GameFramework/Actor.h"
#include "ABackgroundSphere.h"
#include "AFixationPoint.h"
#include "AStimuli.h"
#include "ETestState.h"
#include "ETestType.h"
#include "FTestSettings.h"
#include "FTestResults.h"
#include "FLogManager.h"
#include "UThresholdEstimator.h"
#include "ATestStimuli.generated.h"

UCLASS()
class PERIMAPXR_API ATestStimuli : public AActor
{
    GENERATED_BODY()

public:
    // Constructor that sets default values for properties, especially around eye-tracking and test setup
    ATestStimuli();

protected:
    // Called once when the actor is first initialized, used to start the test and configure settings
    virtual void BeginPlay() override;

    // Called every frame to update actors like the fixation point
    virtual void Tick(float DeltaTime) override;

    // Setup and Initialization
    /** Initializes eye tracking settings and checks whether it's supported and active. */
    void InitializeEyeTracking();

    /** Configures the test environment for a specific test type (e.g., 10-2 or 24-2). */
    void SetupTest(ETestType NewTestType);

    /** Starts the test by generating stimuli and beginning the presentation cycle. */
    void StartTest();

    /** Stops the test once all required stimuli are presented. */
    void StopTest();

    /** Pauses the test, useful for demo mode or when eye tracking is interrupted. */
    void PauseTest();

    /** Resumes the test from a paused state. */
    void ResumeTest();

    // Stimuli Management
    /** Generates the spatial pattern of stimuli based on the test type and configuration. */
    void RunTest();

    /** Flashes a stimulus at a given index and records user interaction with the stimulus. */
    void FlashStimuli(int32 StimulusIndex, float StimulusIntensityInDb);

    /** Update stimuli positions, used when the user moves their head position. */
    void UpdateStimuliPositions();

    /** Compares the current position of the fixation actor to the last stored position. */
    bool FixationActorMoved() const;

    /** Destroys any leftover stimuli actors and cleans up. */
    void CleanupStimuli();

    // Response and Data Management
	/** Saves the test results to a file for later analysis and review. */
    void SaveResultsToFile();

    /** Detects false positives during stimulus detection trials and adjusts the test accordingly. */
    bool CheckForFalsePositives();

    /** Tracks missed responses to identify attention lapses and adjust test timing if necessary. */
    void TrackAttentionLapses(bool bSeen);

    /** Calculates the overall reliability of the user's responses by comparing seen stimuli to total stimuli. */
    float CalculateResponseReliability();

    /** Determines whether a stimulus was detected by the user, combining gaze and input tracking. */
    bool WasStimulusDetected();

    /** Function to set a flag indicating the stimulus was detected. */
    void OnStimulusDetected();

    // Eye Tracking and Gaze Focus
    /** Checks if the user's gaze is focused on the fixation point and manages test state accordingly. */
    bool CheckGazeFocus();

    /** Adjusts the timing between stimuli based on the user's consistency in responding to stimuli. */
    void AdjustTimingBasedOnResponses();

    /** Function to add data to the buffer and keep it within the BufferSize limit. */
    void AddEyeTrackingDataToBuffer(const FPXREyeTrackingData& NewData);

    /** Function to compute a smoothed gaze direction by averaging the buffered data. */
    FVector GetSmoothedGazeDirection();

    /** Function to predict the user's gaze based on the angular velocity. */
    FVector PredictGazeDirection(float PredictionTime);

    /** Function to interpolate gaze direction when data is delayed or unavailable. */
    FVector InterpolateGazeDirection(float DeltaTime, float InterpolationSpeed);

    // Utility Methods
    /** Switches the eye being tested (left/right), resetting relevant data for the new test. */
    void SwitchEye();

    /** Scales the converging lines actor to match the background sphere size, ensuring visual consistency. */
    void SetConvergingLinesScale();

    /** Converts polar coordinates (used for stimuli locations) into cartesian coordinates for actor placement. */
    FVector PolarToCartesian(float Radius, float VerticalAngle, float HorizontalAngle);

	/** Monitors the latency between frames to detect performance spikes. */
    void MonitorLatency();

    // Properties

    // Actor Class References
    /** Reference to the class of the player pawn, used to track the camera and player view. */
    UPROPERTY(EditAnywhere, Category = "References")
    TSubclassOf<AActor> PICOXRPawnClass;

    /** The class of the stimuli actors to spawn at various positions around the player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stimuli")
    TSubclassOf<AStimuli> StimuliActorClass;

    /** The class of the fixation point actor that appears at a fixed distance from the player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fixation")
    TSubclassOf<AFixationPoint> FixationActorClass;

    /** The class for the converging lines actor, which visually guides the user to the fixation point. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Converging Lines")
    TSubclassOf<AActor> ConvergingLinesActorClass;

    /** The class of the background sphere, providing context and environment around the test. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background Sphere")
    TSubclassOf<ABackgroundSphere> BackgroundSphereActorClass;

    // Eye Tracking
    /** Indicates whether the device supports eye tracking, determined during initialization. */
    bool bIsEyeTrackingSupported = false;

    /** Indicates whether eye tracking is currently active, checked at runtime. */
    bool bIsEyeTrackingActive = false;

    /** Holds the value of the HMD refresh rate to sync with eye tracking. */
    float RefreshRate;

    /** Buffer for storing the last few frames of eye-tracking data. */
    TArray<FPXREyeTrackingData> EyeTrackingDataBuffer;

	/** Size of the buffer for storing eye-tracking data frames. */
    const int32 BufferSize = 5;

    /** Array of supported eye tracking modes on the device, populated during initialization. */
    TArray<EPXREyeTrackingMode> SupportedModes;

    /** Tracks the state of the eye tracker, used to detect if eye tracking is paused or disabled. */
    FPXREyeTrackingState TrackingState;

    /** Holds the eye tracking data retrieved from the device, including gaze position and orientation. */
    FPXREyeTrackingData EyeTrackingData;

    /** Configuration for retrieving eye tracking data, including whether to query position and orientation. */
    FPXREyeTrackingDataGetInfo GetInfo;

    // Test Configuration
    /** The current test type (e.g., 10-2 or 24-2), which determines the stimuli arrangement and behavior. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    ETestType TestType;

    /** A map of test settings based on test type, used to configure stimuli size, spacing, and duration. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    TMap<ETestType, FTestSettings> TestSettingsMap;

    // Timing and Randomization
    /** The duration (in seconds) that each stimulus is visible to the user. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float StimuliDuration;

    /** The time (in seconds) between stimuli presentations, adjusted dynamically based on user performance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float TimeBetweenStimuli;

    /** The number of times a stimulus may be retested if uncertainty is detected. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Randomization")
    int32 RetestCount;

    /** The probability that a stimulus will be randomly retested, used to verify user consistency. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Randomization")
    float RetestProbability;

    // Settings for message toggling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Settings")
    bool bEnableConsoleMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Settings")
    bool bEnableOnScreenMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Settings")
    bool bEnableSaveToLog;

    FString LogMessage;

    // Gaze Focus and Demo Mode
    /** Toggles whether the test is in demo mode, which disables certain features like eye tracking. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
    bool bIsDemoMode;

    // Threshold Levels for Eyes
    /** Threshold class pointer object for storing and managing threshold data. */
    UPROPERTY(BlueprintReadOnly)
    UThresholdEstimator* ThresholdEstimator = nullptr;

    // Test State and Results
    /** Current state of the test (e.g., Idle, Running, or Paused). */
    ETestState TestState;

    /** Array of locations for each stimulus, stored in cartesian coordinates. */
    TArray<FVector> StimuliLocations;

    /** Array of recorded results for each stimulus presentation. */
    TArray<FTestResults> TestResultsArray;

    /** Array of active stimuli actors, stored so they can be destroyed when necessary. */
    TArray<AStimuli*> StimuliActors;

    /** Index of the currently active stimulus in the test sequence. */
    int32 CurrentStimulusIndex;

    /** Count of how many times the user has provided consistent responses in a row. */
    int32 ConsistentResponsesCount;

    /** Count of consecutive successful responses, used to adjust difficulty. */
    int32 ConsecutiveSuccessCount;

    /** Tracks the number of consistent responses for each stimulus location. */
    TMap<FVector, int32> ConsistencyMap;

    /** Count of false positives detected during the test. */
    int32 FalsePositiveCount;

    /** Count of consecutive missed responses, used to track attention lapses. */
    int32 ConsecutiveMisses;

    /** Boolean flag to track if the user has responded to a stimulus. */
    bool bUserResponded;

    // Actors for Fixation, Background Sphere, and Converging Lines
    /** Pointer to the fixation point actor, placed in front of the user to guide their gaze. */
    AFixationPoint* FixationActor;

    /** Pointer to the converging lines actor, visually guiding the user towards the fixation point. */
    AActor* ConvergingLinesActor;

    /** Pointer to the background sphere actor, which creates a visual environment for the test. */
    ABackgroundSphere* BackgroundSphereActor;

    // Eye Tracking and Test Control Variables
    /** Boolean flag to indicate if the test is paused. */
    bool bIsTestPaused;

    /** Boolean flag to track whether the left eye is being tested. */
    bool bIsLeftEye;

    // Timer Handles
    /** Handle for managing the timer that checks the user's gaze focus every 0.1 seconds. */
    FTimerHandle GazeCheckTimerHandle;

    /** Handle for managing the timer that schedules the presentation of stimuli. */
    FTimerHandle StimuliPresentationTimerHandle;

    /** Handle for managing the timer that handles the user's response to stimuli. */
    FTimerHandle StimulusResponseTimerHandle;

    /** Handle for managing the timer that schedules the presentation of stimuli for false positives. */
    FTimerHandle CatchTrialTimerHandle;

	// Utility Variables
    /** Stores the latency value, initialized to 0. */
    float DetectedLatency;

    /** Stores the last known position of the FixationActor. */
    FVector LastFixationPosition;

    /** Instance of FLogManager to call in this class, ensures ini */
    FLogManager LogManager;

    // Timing Constants
    const float MinStimuliDuration = 0.2f;  // Fixed stimulus duration
    const float MaxStimuliDuration = 0.2f;  // Fixed stimulus duration

    const float MinTimeBetweenStimuli = 1.0f;  // Minimum time between stimuli
    const float MaxTimeBetweenStimuli = 2.0f;  // Maximum time between stimuli
};