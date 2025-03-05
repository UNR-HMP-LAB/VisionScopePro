// ATestStimuli.cpp

#include "ATestStimuli.h"
#include "UThresholdEstimator.h"
#include "AStimuli.h"
#include "AFixationPoint.h"
#include "ABackgroundSphere.h"
#include "FTestResults.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PXR_MotionTracking.h"
#include "PXR_MotionTrackingTypes.h" 
#include "PXR_HMDFunctionLibrary.h"
#include "EyeTrackerFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "FLogManager.h"

// Constructor sets default values for properties and initializes eye tracking and test settings
ATestStimuli::ATestStimuli()
{
    // Initialize default test parameters. These values are set based on standard visual field tests.
    TestType = ETestType::TEST_24_2;  // Default test type: 24-2, commonly used in visual field tests
    TestState = ETestState::Idle;     // The test starts in the idle state, no stimuli presented initially
    StimuliDuration = 0.2f;           // Default duration of each stimulus, set to 200ms for visual threshold assessment
    TimeBetweenStimuli = 1.0f;        // Time between stimuli presentation to prevent overlap
    RetestCount = 3;                  // Number of retests for stimuli near threshold to ensure accuracy
    RetestProbability = 0.1f;         // Probability that a stimulus is retested
    bIsDemoMode = false;              // By default, the demo mode is disabled; real eye-tracking data is used
    bIsLeftEye = true;                // Start with the left eye, as is standard in most vision tests
    ConsecutiveMisses = 0;            // Track missed stimuli to adjust the test dynamically
    ConsistentResponsesCount = 0;     // Track consistent responses to adjust test speed
    FalsePositiveCount = 0;           // Count of false positives to detect unreliable responses
    DetectedLatency = 0.0f;           // Initialize latency tracking to zero
    bEnableConsoleMessages = true;    // Enable debug messages on the console
    bEnableOnScreenMessages = true;   // Enable debug messages to the screen
    bEnableSaveToLog = true;          // Enable debug messages to be saved to the logfile on the headset

    // Initialize eye tracking state variables, set to false until validated
    bIsEyeTrackingSupported = false;
    bIsEyeTrackingActive = false;

    // Predefine settings for different test types (e.g., 24-2 and 10-2), defining degrees and spacing between stimuli
    TestSettingsMap.Add(ETestType::TEST_10_2, FTestSettings(55.0f, 2.0f, 0.5f, 6.0f, 68, 15.0f, 15.0f));
    TestSettingsMap.Add(ETestType::TEST_24_2, FTestSettings(133.5f, 2.0f, 0.5f, 6.0f, 54, 24.0f, 24.0f));

    // Initialize ThresholdEstimator for tracking and managing threshold estimation
    ThresholdEstimator = CreateDefaultSubobject<UThresholdEstimator>(TEXT("ThresholdEstimator"));
}

// Called when the game starts. Initializes the eye tracking and sets up the test environment.
void ATestStimuli::BeginPlay()
{
    Super::BeginPlay();

    // Bind the input action for stimulus detection
    if (InputComponent)
    {
        InputComponent->BindAction("DetectStimulus", IE_Pressed, this, &ATestStimuli::OnStimulusDetected);
    }

    // Ensure that the eye-tracking system is initialized before starting the test
    if (!bIsDemoMode)
    {
        InitializeEyeTracking();
    }

    // Set up the test parameters and environment (e.g., fixation point, stimuli locations) for the first eye
    SetupTest(TestType);

    // Initialize LastFixationPosition to the starting position of the fixation actor
    if (FixationActor)
    {
        LastFixationPosition = FixationActor->GetActorLocation();
    }

    // Initialize the threshold estimator
    FTestSettings TestSettings = *TestSettingsMap.Find(TestType);
    ThresholdEstimator->Initialize(TestSettings, TestType, bIsLeftEye);

    // Start the test and monitor eye gaze to check if the participant is focused on the fixation point
    StartTest();
    GetWorld()->GetTimerManager().SetTimer(GazeCheckTimerHandle, [this]() { CheckGazeFocus(); }, 0.1f, true);
}

// Called every frame to update the position of dynamic elements such as the fixation point.
void ATestStimuli::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Call MonitorLatency() to track any hardware or performance delays
    MonitorLatency();

    // Ensure the test is running and the fixation actor is valid
    if (TestState == ETestState::Running && FixationActor)
    {
        // Get the user's current camera location and rotation
        AActor* PICOXRPawnActor = UGameplayStatics::GetActorOfClass(GetWorld(), PICOXRPawnClass);
        if (PICOXRPawnActor)
        {
            UCameraComponent* CameraComponent = PICOXRPawnActor->FindComponentByClass<UCameraComponent>();
            if (CameraComponent)
            {
                FVector CameraLocation = CameraComponent->GetComponentLocation();
                FRotator CameraRotation = CameraComponent->GetComponentRotation();

                // Update the fixation point location to stay a fixed distance from the user's camera
                FVector NewFixationLocation = CameraLocation + CameraRotation.Vector() * 30.0f;  // 30cm ahead of the user

                // Set the updated location for the fixation actor
                FixationActor->SetActorLocation(NewFixationLocation);

                // If the fixation point has moved update the stimuli positions accordingly
                if (FixationActorMoved())
                {
                    // Update stimuli positions only when the fixation point has moved
                    UpdateStimuliPositions();

                    // Update the last known position to the current one
                    LastFixationPosition = FixationActor->GetActorLocation();
                }
            }
        }
    }
}

// Checks and configures the eye-tracking system for compatibility and activation
void ATestStimuli::InitializeEyeTracking()
{
    // Check if the device supports eye tracking. This ensures the test can proceed with valid data.
    bool bSupportSuccess = PICOXRMotionTracking::GetEyeTrackingSupported(bIsEyeTrackingSupported, SupportedModes);
    if (bSupportSuccess && bIsEyeTrackingSupported)
    {
        bool bSupportsBothEyes = false;
        for (const EPXREyeTrackingMode& Mode : SupportedModes)
        {
            if (Mode == EPXREyeTrackingMode::PXR_ETM_BOTH)
            {
                bSupportsBothEyes = true;
                break;
            }
        }

        if (bSupportsBothEyes)
        {
            // Now start the eye tracking service
            FPXREyeTrackingStartInfo StartInfo;
            StartInfo.StartMode = EPXREyeTrackingMode::PXR_ETM_BOTH; // Start tracking for both eyes
            StartInfo.NeedCalibration = true;

            bool bStartSuccess = PICOXRMotionTracking::StartEyeTracking(StartInfo);

            if (bStartSuccess)
            {
                LogMessage = "Eye tracking service started successfully.";
                LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            }
            else
            {
                LogMessage = "Failed to start eye tracking service.";
                LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                return;
            }

            // Check if eye tracking is active.
            bool bTrackingSuccess = PICOXRMotionTracking::GetEyeTrackingState(bIsEyeTrackingActive, TrackingState);

            if (bTrackingSuccess && bIsEyeTrackingActive && TrackingState.CurrentTrackingMode == EPXREyeTrackingMode::PXR_ETM_BOTH)
            {
                LogMessage = "Eye tracking is active and operational.";
                LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            }
            else
            {
                LogMessage = "Eye tracking is not currently active or failed to retrieve state.";
                LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            }

            // Setup the eye tracking data request to gather real-time position and orientation data.
            GetInfo.DisplayTime = 0;  // Use predicted frame time for smoother gaze tracking
            GetInfo.QueryPosition = true;
            GetInfo.QueryOrientation = true;
        }
        else
        {
            LogMessage = "Eye tracking is supported, but not for both eyes.";
            LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            return;
        }
    }
    else
    {
        // If unsupported, log a warning and proceed with the test in demo mode.
        LogMessage = "Eye tracking is not supported or API call failed.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        return;
    }
}

// Configures the test environment for the specified test type (e.g., 24-2, 10-2)
void ATestStimuli::SetupTest(ETestType NewTestType)
{
    TestType = NewTestType;  // Set the test type to the provided value

    // Retrieve and apply test settings based on the test type selected
    if (TestSettingsMap.Contains(TestType))
    {
        FTestSettings Settings = *TestSettingsMap.Find(TestType);

        // Find the camera in the PICOXRPawn actor (the user), so we can orient elements (e.g., fixation point) around it
        AActor* PICOXRPawnActor = UGameplayStatics::GetActorOfClass(GetWorld(), PICOXRPawnClass);
        if (PICOXRPawnActor)
        {
            UCameraComponent* CameraComponent = PICOXRPawnActor->FindComponentByClass<UCameraComponent>();
            if (CameraComponent)
            {
                FVector CameraLocation = CameraComponent->GetComponentLocation();
                FRotator CameraRotation = CameraComponent->GetComponentRotation();

                // Calculate fixation point's position 30cm in front of the user’s camera
                FVector FixationLocation = CameraLocation + CameraRotation.Vector() * 30.0f;

                // Spawn and scale the fixation point based on the test settings
                if (FixationActorClass)
                {
                    FixationActor = GetWorld()->SpawnActor<AFixationPoint>(FixationActorClass, FixationLocation, FRotator::ZeroRotator);
                    FixationActor->SetScale(Settings.FixationPointDiameter);
                }
                else
                {
                    LogMessage = "FixationActor is null.";
                    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                }

                // Spawn and scale the background sphere that encapsulates stimuli
                if (BackgroundSphereActorClass)
                {
                    BackgroundSphereActor = GetWorld()->SpawnActor<ABackgroundSphere>(BackgroundSphereActorClass, CameraLocation, FRotator::ZeroRotator);
                    float BackgroundSphereScale(Settings.StimuliRadius * 10);  // Ensure the sphere encompasses all stimuli
                    BackgroundSphereActor->SetScale(BackgroundSphereScale);
                    LogMessage = FString::Printf(TEXT("BackgroundSphere Scale: %s"), *BackgroundSphereActor->GetActorScale3D().ToString());
                    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                }
                else
                {
                    LogMessage = "BackgroundSphereActor is null.";
                    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                }

                // Spawn and orient the converging lines actor, ensuring it points toward the fixation point
                if (ConvergingLinesActorClass)
                {
                    ConvergingLinesActor = GetWorld()->SpawnActor<AActor>(ConvergingLinesActorClass, FixationLocation, FRotator::ZeroRotator);
                    FVector DirectionToFixation = FixationActor->GetActorLocation() - ConvergingLinesActor->GetActorLocation();
                    ConvergingLinesActor->SetActorRotation(DirectionToFixation.Rotation());
                    SetConvergingLinesScale();  // Scale the lines relative to the background sphere
                }
                else
                {
                    LogMessage = "ConvergingLinesActor is null.";
                    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                }

                // Prepare the locations for stimuli presentation, ensuring appropriate spacing and coverage
                StimuliLocations.Empty();
                StimuliActors.Empty();

                for (int32 i = 0; i < Settings.NumStimuli; ++i)
                {
                    // Distribute stimuli evenly across the angular range
                    float VerticalAngle = FMath::RandRange(-Settings.MaxVerticalAngle, Settings.MaxVerticalAngle);
                    float HorizontalAngle = FMath::RandRange(-Settings.MaxHorizontalAngle, Settings.MaxHorizontalAngle);

                    FVector RelativeLocation = PolarToCartesian(Settings.StimuliRadius, FMath::DegreesToRadians(VerticalAngle), FMath::DegreesToRadians(HorizontalAngle));
                    StimuliLocations.Add(RelativeLocation);

                    if (StimuliActorClass)
                    {
                        FActorSpawnParameters SpawnParams;
                        AStimuli* NewStimulus = GetWorld()->SpawnActor<AStimuli>(StimuliActorClass, FixationLocation + RelativeLocation, FRotator::ZeroRotator, SpawnParams);
                        if (NewStimulus)
                        {
                            NewStimulus->bEnableConsoleMessages = bEnableConsoleMessages;
                            NewStimulus->bEnableOnScreenMessages = bEnableOnScreenMessages;
                            NewStimulus->bEnableSaveToLog = bEnableSaveToLog;
                            NewStimulus->SetVisibility(false);
                            NewStimulus->SetScale(Settings.StimuliDiameter);
                            StimuliActors.Add(NewStimulus);

                            // Debug message for confirmation
                            LogMessage = FString::Printf(TEXT("Stimulus %d spawned at location: %s"), i, *(FixationLocation + RelativeLocation).ToString());
                            LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                        }
                    }
                    else
                    {
                        LogMessage = "StimuliActor is null.";
                        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
                    }
                }
            }
        }
    }
}

// Initiates the visual stimuli test by generating the stimuli pattern and starting the timer.
void ATestStimuli::StartTest()
{
    if (TestState != ETestState::Idle)
    {
        LogMessage = "Test is already running or completed.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        return;
    }

    // Log that the test is starting
    LogMessage = FString::Printf(TEXT("Starting the test for the %s eye."), bIsLeftEye ? TEXT("left") : TEXT("right"));
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

    // Set the test state to running, which triggers stimuli generation
    TestState = ETestState::Running;

    // Initialize threshold estimator for the current eye
    if (ThresholdEstimator)
    {
        FTestSettings TestSettings = *TestSettingsMap.Find(TestType);
        ThresholdEstimator->Initialize(TestSettings, TestType, bIsLeftEye);
    }

    // Generate the stimuli pattern and start presenting them to the user
    RunTest();
}

// Stops the test and cleans up stimuli after both eyes have been tested
void ATestStimuli::StopTest()
{
    if (TestState != ETestState::Running)
    {
        LogMessage = "Test is not currently running.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        return;
    }

    // Stop stimuli presentation and clear timers
    GetWorld()->GetTimerManager().ClearTimer(StimuliPresentationTimerHandle);

    // Clean up all spawned stimuli
    CleanupStimuli();

    // Check if the test needs to switch to the other eye
    if (bIsLeftEye)
    {
        // Switch to the right eye and continue testing
        LogMessage = "Left eye test completed. Switching to the right eye.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        SwitchEye();
    }
    else
    {
        // If both eyes have been tested, complete the test
        TestState = ETestState::Completed;
        LogMessage = "Test completed for both eyes.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        // Calculate final thresholds and sensitivities
        if (ThresholdEstimator)
        {
            ThresholdEstimator->CalculateFinalThresholds();
            ThresholdEstimator->CalculateFinalSensitivities();
        }
    }

    // Save the test results to a file for later review
    SaveResultsToFile();
}

// Temporarily halts the stimuli presentation and test progression
void ATestStimuli::PauseTest()
{
    TestState = ETestState::Paused;
    GetWorld()->GetTimerManager().PauseTimer(StimuliPresentationTimerHandle);
    GetWorld()->GetTimerManager().PauseTimer(StimulusResponseTimerHandle);
    GetWorld()->GetTimerManager().PauseTimer(CatchTrialTimerHandle);

    // Hide any visible stimuli using SetVisibility
    for (AStimuli* Stimulus : StimuliActors)
    {
        if (Stimulus)
        {
            Stimulus->SetVisibility(false);
        }
    }
    LogMessage = "Test paused.";
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
}

// Resumes the test from where it was paused
void ATestStimuli::ResumeTest()
{
    TestState = ETestState::Running;
    GetWorld()->GetTimerManager().UnPauseTimer(StimuliPresentationTimerHandle);
    GetWorld()->GetTimerManager().UnPauseTimer(StimulusResponseTimerHandle);
    GetWorld()->GetTimerManager().UnPauseTimer(CatchTrialTimerHandle);
    LogMessage = "Test resumed.";
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
}

// Handles the logic of presenting each stimulus and checking for user response
void ATestStimuli::RunTest()
{
    LogMessage = FString::Printf(TEXT("RunTest called with CurrentStimulusIndex: %d"), CurrentStimulusIndex);
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

    // Ensure that the test is running and hasn't reached the end of stimuli
    if (TestState != ETestState::Running || bIsTestPaused)
    {
        LogMessage = FString::Printf(TEXT("RunTest exiting: TestState=%d, bIsTestPaused=%d"), (int32)TestState, bIsTestPaused);
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        return;
    }

    // Check if all stimuli have been processed for the current eye
    if (CurrentStimulusIndex >= StimuliLocations.Num())
    {
        LogMessage = "All stimuli processed for current eye.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        // If testing for the left eye is complete, switch to the right eye or end the test
        if (bIsLeftEye)
        {
            SwitchEye();
            RunTest();  // Start testing for the right eye
        }
        else
        {
            StopTest();  // Both eyes have been tested, stop the test
        }
        return;
    }

    // Check if the CurrentStimulusIndex is within bounds
    if (!StimuliLocations.IsValidIndex(CurrentStimulusIndex))
    {
        LogMessage = "CurrentStimulusIndex out of bounds. Test will stop.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        StopTest();
        return;
    }

    FVector Location = StimuliLocations[CurrentStimulusIndex];

    // Get the next stimulus intensity from the threshold estimator
    float StimulusIntensityInDb = ThresholdEstimator ? ThresholdEstimator->GetNextStimulusIntensityInDb(Location) : 20.0f;

    // Debugging: Log the intensity returned by ThresholdEstimator
    LogMessage = FString::Printf(TEXT("ThresholdEstimator returned intensity %f dB for location %s"), StimulusIntensityInDb, *Location.ToString());
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);


    // Adjust StimuliDuration and TimeBetweenStimuli for latency
    float AdjustedStimuliDuration = FMath::Clamp(StimuliDuration + DetectedLatency, MinStimuliDuration, MaxStimuliDuration);

    // Flash the stimulus at the current index with the calculated intensity
    FlashStimuli(CurrentStimulusIndex, StimulusIntensityInDb);

    // Set the state to waiting for input
    TestState = ETestState::WaitingForInput;

    // Set a timer to handle the user's response after the stimulus presentation
    GetWorld()->GetTimerManager().SetTimer(StimulusResponseTimerHandle, [this, Location]()
    {
        LogMessage = FString::Printf(TEXT("Response handling lambda called for stimulus at location: %s"), *Location.ToString());
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        // Ensure that the test is still waiting for input
        if (TestState != ETestState::WaitingForInput || bIsTestPaused)
        {
            LogMessage = FString::Printf(TEXT("Exiting response handler: TestState=%d, bIsTestPaused=%d"), (int32)TestState, bIsTestPaused);
            LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            return;
        }

        // Check if the stimulus was detected
        bool bStimulusDetected = WasStimulusDetected();
        LogMessage = FString::Printf(TEXT("Stimulus detected: %s"), bStimulusDetected ? TEXT("Yes") : TEXT("No"));
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        // Record the result with the threshold estimator
        if (ThresholdEstimator)
        {
            ThresholdEstimator->UpdateWithResponse(Location, ThresholdEstimator->GetThresholdEstimateInDb(Location), bStimulusDetected);
        }

        // Move to the next stimulus
        CurrentStimulusIndex++;

        // Reset test state
        TestState = ETestState::Running;

        // Schedule the next RunTest() call after TimeBetweenStimuli
        GetWorld()->GetTimerManager().SetTimer(StimuliPresentationTimerHandle, this, &ATestStimuli::RunTest, TimeBetweenStimuli, false);

    }, AdjustedStimuliDuration, false);
}

// Update stimuli positions, used when the user moves their head position.
void ATestStimuli::UpdateStimuliPositions()
{
    if (FixationActor)
    {
        FVector FixationLocation = FixationActor->GetActorLocation();
        for (int32 i = 0; i < StimuliActors.Num(); ++i)
        {
            if (StimuliActors[i])
            {
                // Update each stimulus' position relative to the fixation point
                StimuliActors[i]->SetActorLocation(FixationLocation + StimuliLocations[i]);
            }
        }
    }
}

// Compares the current position of the fixation actor to the last stored position.
bool ATestStimuli::FixationActorMoved() const
{
    if (FixationActor)
    {
        // Compare the current position of the fixation actor with the last known position
        FVector CurrentPosition = FixationActor->GetActorLocation();
        return !CurrentPosition.Equals(LastFixationPosition, 0.01f);  // Allow small tolerance to prevent floating point errors
    }
    return false;
}

// Handles the visibility logic for a specific stimulus
void ATestStimuli::FlashStimuli(int32 StimulusIndex, float StimulusIntensityInDb)
{
    // Exit if demo mode is active, or the test is paused, or the index is invalid
    /*
    if (bIsDemoMode || bIsTestPaused || !StimuliActors.IsValidIndex(StimulusIndex))
    {
        LogMessage = "Stimuli flashing is paused due to demo mode or test pause.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        return;
    }
    */

    // Get the stimulus actor for the current index
    AStimuli* StimulusActor = StimuliActors[StimulusIndex];
    if (StimulusActor)
    {
        // Set the brightness of the stimulus based on the intensity in decibels
        StimulusActor->SetBrightnessFromDb(StimulusIntensityInDb);
        LogMessage = FString::Printf(TEXT("SetBrightnessFromDb called on StimulusActor at index %d with intensity %f dB."), StimulusIndex, StimulusIntensityInDb);
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        // Make the stimulus visible using the SetVisibility function
        StimulusActor->SetVisibility(true);
        LogMessage = FString::Printf(TEXT("SetVisibility(true) called on StimulusActor at index %d."), StimulusIndex);
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        // Hide the stimulus after the specified duration using SetVisibility(false)
        FTimerHandle HideStimuliTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(HideStimuliTimerHandle, [this, StimulusIndex, StimulusActor]()
        {
            StimulusActor->SetVisibility(false);  // Hide the stimulus after the flash
            LogMessage = FString::Printf(TEXT("Stimulus %d hidden after duration."), StimulusIndex);
            LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        }, StimuliDuration, false);
    }
    else
    {
        LogMessage = FString::Printf(TEXT("StimulusActor is null at index %d."), StimulusIndex);
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
}

// Removes and destroys all existing stimuli actors from the world.
void ATestStimuli::CleanupStimuli()
{
    // Iterate through each spawned stimulus actor and destroy it
    for (AActor* StimulusActor : StimuliActors)
    {
        if (StimulusActor)
        {
            StimulusActor->Destroy();
        }
    }

    // Clear the array of actors after they are destroyed
    StimuliActors.Empty();
}

// Saves the test results to a file for later analysis and review.
void ATestStimuli::SaveResultsToFile()
{
    FString SavePath = FPaths::ProjectDir() + "/TestResults.csv";
    FString ResultsString = "LocationX,LocationY,Threshold,Sensitivity\n";

    const TMap<FVector, float>& FinalThresholds = ThresholdEstimator->GetFinalThresholdsInDb();
    for (const auto& Pair : FinalThresholds)
    {
        FVector Location = Pair.Key;
        float Threshold = Pair.Value;
        float Sensitivity = 1.0f / Threshold;
        ResultsString += FString::Printf(TEXT("%f,%f,%f,%f\n"), Location.X, Location.Y, Threshold, Sensitivity);
    }

    FFileHelper::SaveStringToFile(ResultsString, *SavePath);
    LogMessage = FString::Printf(TEXT("Test results saved to %s"), *SavePath);
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
}

bool ATestStimuli::CheckForFalsePositives()
{
    // Determine if a catch trial should occur
    bool bIsCatchTrial = FMath::RandRange(0, 9) == 0;  // 10% chance

    if (bIsCatchTrial)
    {
        LogMessage = "Catch trial initiated";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

        // Set the test state to waiting for input
        TestState = ETestState::WaitingForInput;

        // Adjusted timings
        float AdjustedStimuliDuration = StimuliDuration + DetectedLatency;
        float AdjustedTimeBetweenStimuli = TimeBetweenStimuli + DetectedLatency;

        // Clamp adjusted times
        AdjustedStimuliDuration = FMath::Clamp(AdjustedStimuliDuration, MinStimuliDuration, MaxStimuliDuration);
        AdjustedTimeBetweenStimuli = FMath::Clamp(AdjustedTimeBetweenStimuli, MinTimeBetweenStimuli, MaxTimeBetweenStimuli);

        // Schedule the timer to handle the user's response
        GetWorld()->GetTimerManager().SetTimer(CatchTrialTimerHandle, [this, AdjustedTimeBetweenStimuli]()
            {
                // Ensure that the test is still running and waiting for input
                if (TestState != ETestState::WaitingForInput || bIsTestPaused)
                {
                    return;
                }

                // Check if the user responded during the catch trial
                bool bStimulusDetected = WasStimulusDetected();

                if (bStimulusDetected)
                {
                    // User responded when no stimulus was presented (false positive)
                    FalsePositiveCount++;
                    LogMessage = "False positive detected during catch trial.";
                    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

                    // Optionally adjust test parameters due to unreliable responses
                    TimeBetweenStimuli = FMath::Clamp(TimeBetweenStimuli + 0.2f, MinTimeBetweenStimuli, MaxTimeBetweenStimuli);  // Slow down the test
                }

                // Reset test state and proceed to the next stimulus
                TestState = ETestState::Running;

                // Schedule the next RunTest() call using adjusted time
                GetWorld()->GetTimerManager().SetTimer(StimuliPresentationTimerHandle, this, &ATestStimuli::RunTest, AdjustedTimeBetweenStimuli, false);

            }, AdjustedStimuliDuration + AdjustedTimeBetweenStimuli, false);
    }

    return bIsCatchTrial;
}

// Tracks whether the user is failing to focus and adjusts the test speed accordingly.
void ATestStimuli::TrackAttentionLapses(bool bSeen)
{
    if (!bSeen)
    {
        // If the user misses multiple stimuli in a row, slow down the test to regain focus
        ConsecutiveMisses++;
        if (ConsecutiveMisses >= 3)
        {
            LogMessage = "Attention lapse detected. Slowing down stimuli.";
            LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            TimeBetweenStimuli = FMath::Clamp(TimeBetweenStimuli + 0.5f, 1.0f, 2.0f);
        }
    }
    else
    {
        // Reset the miss counter after a successful detection and optionally speed up the test
        ConsecutiveMisses = 0;
        TimeBetweenStimuli = FMath::Clamp(TimeBetweenStimuli - 0.1f, 0.5f, 1.0f);
    }
}

// Computes the ratio of correct responses to total stimuli presented to gauge test reliability.
float ATestStimuli::CalculateResponseReliability()
{
    int32 TotalStimuliPresented = TestResultsArray.Num();
    int32 TotalSeen = TestResultsArray.FilterByPredicate([](const FTestResults& Result) { return Result.bSeen; }).Num();
    return TotalSeen / static_cast<float>(TotalStimuliPresented);  // Calculate response reliability ratio
}

// Determines if the user detected the stimulus based on eye gaze and user input.
bool ATestStimuli::WasStimulusDetected()
{
    // Check if the user's gaze was focused on the fixation point during stimulus presentation and key pressed
    if (CheckGazeFocus() && bUserResponded)
    {
        bUserResponded = false;  // Reset the flag for the next stimulus
        return true;
    }
    else
    {
        LogMessage = "Gaze left the fixation point during stimulus presentation.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        return false;
    }
}

// Function to set a flag indicating the stimulus was detected
void ATestStimuli::OnStimulusDetected()
{
    if (TestState == ETestState::WaitingForInput)
    {
        bUserResponded = true;
    }
}

// Monitors the user's eye gaze to verify that they are focusing on the fixation point.
bool ATestStimuli::CheckGazeFocus()
{
    if (bIsDemoMode)
    {
        // If demo mode is active, pause the test and return early without checking eye-tracking data.
        // PauseTest();
        LogMessage = "Running in demo mode, eye tracking data not available.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        // return false;
        return true;
    }

    // Retrieve eye-tracking data to check if the user's gaze is focused on the fixation point
    if (PICOXRMotionTracking::GetEyeTrackingData(0.0f, GetInfo, EyeTrackingData))
    {
        // Add the new eye-tracking data to the buffer
        AddEyeTrackingDataToBuffer(EyeTrackingData);

        FVector GazeDirection = GetSmoothedGazeDirection();

        // If gaze data is invalid or delayed, use interpolation
        if (GazeDirection.IsZero())
        {
            float DeltaTime = GetWorld()->GetDeltaSeconds();
            float InterpolationSpeed = 5.0f;  // Define as needed
            GazeDirection = InterpolateGazeDirection(DeltaTime, InterpolationSpeed);
        }

        if (GazeDirection.IsZero())
        {
            LogMessage = "Unable to determine gaze direction.";
            LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
            return false;
        }

        // Retrieve the current position and orientation of the HMD (head-mounted display)
        FQuat HMDOrientation = UPICOXRHMDFunctionLibrary::PXR_GetCurrentOrientation();
        FVector HMDPosition = UPICOXRHMDFunctionLibrary::PXR_GetCurrentPosition();

        // Rotate the gaze direction according to the HMD's current orientation
        FVector WorldGazeDirection = HMDOrientation.RotateVector(GazeDirection);

        // Calculate the vector pointing to the fixation point and normalize it
        FVector FixationLocation = FixationActor->GetActorLocation();
        FVector ToFixation = (FixationLocation - HMDPosition).GetSafeNormal();

        // Check if the gaze is within a certain angular threshold of the fixation point
        float GazeToleranceDegrees = 10.0f;  // Define as needed
        bool bIsGazingAtFixation = FVector::DotProduct(WorldGazeDirection, ToFixation) > FMath::Cos(FMath::DegreesToRadians(GazeToleranceDegrees));

        if (bIsGazingAtFixation)
        {
            // If the user is gazing at the fixation point, hide the converging lines and resume the test
            if (ConvergingLinesActor)
            {
                ConvergingLinesActor->SetActorHiddenInGame(true);
            }
            if (TestState == ETestState::Paused)
            {
                ResumeTest();
            }
            return true;
        }
        else
        {
            // If the user is not focused on the fixation point, show the converging lines and pause the test
            if (ConvergingLinesActor)
            {
                ConvergingLinesActor->SetActorHiddenInGame(false);
            }
            if (TestState != ETestState::Paused)
            {
                PauseTest();
            }
            return false;
        }
    }
    else
    {
        // Log an error if the eye-tracking data retrieval fails
        LogMessage = "Failed to retrieve eye tracking data.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        if (TestState != ETestState::Paused)
        {
            PauseTest();
        }
        return false;
    }
}

// Dynamically adjusts the timing of stimulus presentation based on user consistency.
void ATestStimuli::AdjustTimingBasedOnResponses()
{
    if (ConsistentResponsesCount > 5)
    {
        // Speed up the test if the participant is responding consistently
        TimeBetweenStimuli = FMath::Clamp(TimeBetweenStimuli - 0.1f, 0.5f, 1.0f);
    }
    else
    {
        // Slow down the test if responses are inconsistent
        TimeBetweenStimuli = FMath::Clamp(TimeBetweenStimuli + 0.1f, 1.0f, 2.0f);
    }
}

// Function to add data to the buffer and keep it within the BufferSize limit
void ATestStimuli::AddEyeTrackingDataToBuffer(const FPXREyeTrackingData& NewData)
{
    // Insert the new data at the front of the buffer
    EyeTrackingDataBuffer.Insert(NewData, 0);

    // Ensure the buffer doesn't exceed the specified size
    if (EyeTrackingDataBuffer.Num() > BufferSize)
    {
        EyeTrackingDataBuffer.RemoveAt(BufferSize);  // Remove the oldest data
    }
}

// Function to compute a smoothed gaze direction by averaging the buffered data
FVector ATestStimuli::GetSmoothedGazeDirection()
{
    if (EyeTrackingDataBuffer.Num() == 0) return FVector::ZeroVector;

    FVector SmoothedGazeDirection = FVector::ZeroVector;

    // Average the gaze directions from the buffered data
    for (const FPXREyeTrackingData& Data : EyeTrackingDataBuffer)
    {
        const FPXRPerEyeData& EyeData = bIsLeftEye ? Data.PerEyeDatas[0] : Data.PerEyeDatas[1];
        if (EyeData.bIsPoseValid)
        {
            SmoothedGazeDirection += EyeData.Orientation.Vector();
        }
    }

    // Handle the case where no valid gaze data was found
    if (SmoothedGazeDirection.IsZero())
    {
        return FVector::ZeroVector;
    }

    // Normalize the resulting gaze direction vector
    SmoothedGazeDirection /= EyeTrackingDataBuffer.Num();
    return SmoothedGazeDirection;
}

// Function to predict the user's gaze based on the angular velocity
FVector ATestStimuli::PredictGazeDirection(float PredictionTime)
{
    FVector PredictedGazeDirection = FVector::ZeroVector;

    if (EyeTrackingDataBuffer.Num() > 0)
    {
        // Get the most recent eye-tracking data
        const FPXREyeTrackingData& CurrentEyeData = EyeTrackingDataBuffer[0];

        // Get the current gaze direction and angular velocity from the data
        FVector CurrentGazeDirection = CurrentEyeData.PerEyeDatas[0].Orientation.Vector();  // Example for left eye
        FVector AngularVelocity = UPICOXRHMDFunctionLibrary::PXR_GetAngularVelocity();

        // Predict the next gaze direction based on current angular velocity
        PredictedGazeDirection = CurrentGazeDirection + AngularVelocity * PredictionTime;
    }

    // Return the predicted direction, normalized
    return PredictedGazeDirection.GetSafeNormal();
}

// Function to interpolate gaze direction when data is delayed or unavailable
FVector ATestStimuli::InterpolateGazeDirection(float DeltaTime, float InterpolationSpeed)
{
    // Default to using the smoothed gaze direction if available
    FVector SmoothedGazeDirection = GetSmoothedGazeDirection();

    // Use interpolation if no fresh data is available
    FVector LastGazePosition = SmoothedGazeDirection;
    FVector CurrentGazePosition = FVector::ZeroVector;  // Placeholder for actual gaze direction

    if (EyeTrackingDataBuffer.Num() > 0)
    {
        // Retrieve the most recent gaze direction
        CurrentGazePosition = EyeTrackingDataBuffer[0].PerEyeDatas[0].Orientation.Vector();
    }

    // If eye-tracking data is unavailable, predict the gaze direction
    if (EyeTrackingDataBuffer.Num() == 0 || PICOXRMotionTracking::GetEyeTrackingData(100.0f, GetInfo, EyeTrackingData) == false)
    {
        FVector PredictedGaze = PredictGazeDirection(0.05f);  // Predict 50ms ahead
        CurrentGazePosition = FMath::VInterpTo(LastGazePosition, PredictedGaze, DeltaTime, InterpolationSpeed);
    }

    return CurrentGazePosition.GetSafeNormal();
}

// Switches the test to the other eye (e.g., from left eye to right eye).
void ATestStimuli::SwitchEye()
{
    bIsLeftEye = !bIsLeftEye;  // Toggle the eye being tested
    LogMessage = FString::Printf(TEXT("Switched to %s eye."), bIsLeftEye ? TEXT("left") : TEXT("right"));
    LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);

    // Reset the stimulus index and clean up existing stimuli before starting the test for the other eye
    CurrentStimulusIndex = 0;
    CleanupStimuli();

    // Reconfigure the test for the newly selected eye and start the test
    SetupTest(TestType);
    StartTest();
}

// Scales the converging lines actor to fit relative to the background sphere.
void ATestStimuli::SetConvergingLinesScale()
{
    if (BackgroundSphereActor)
    {
        FVector BackgroundSphereScale = BackgroundSphereActor->GetActorScale3D();
        FVector ConvergingLinesScale = BackgroundSphereScale * 0.98f;  // Scale the converging lines slightly smaller than the sphere
        ConvergingLinesActor->SetActorScale3D(ConvergingLinesScale);
    }
}

// Converts polar coordinates to cartesian coordinates for stimulus placement.
FVector ATestStimuli::PolarToCartesian(float Radius, float VerticalAngle, float HorizontalAngle)
{
    // Convert spherical coordinates (radius, angles) into Cartesian coordinates for precise stimulus placement
    FVector Cartesian;
    Cartesian.X = Radius * FMath::Cos(VerticalAngle) * FMath::Sin(HorizontalAngle);
    Cartesian.Y = Radius * FMath::Sin(VerticalAngle);
    Cartesian.Z = Radius * FMath::Cos(VerticalAngle) * FMath::Cos(HorizontalAngle);
    return Cartesian;
}

// Monitors the latency between frames to detect performance spikes.
void ATestStimuli::MonitorLatency()
{
    static float LastFrameTime = FPlatformTime::Seconds();  // Keep track of the previous frame time
    float CurrentTime = FPlatformTime::Seconds();           // Get the current time

    // Calculate the time difference between the current frame and the last frame
    float FrameLatency = CurrentTime - LastFrameTime;

    // Update the last frame time to the current time
    LastFrameTime = CurrentTime;

    // Initialize DetectedLatency if it's the first frame
    if (DetectedLatency == 0.0f)
    {
        DetectedLatency = FrameLatency;
    }
    else
    {
        // Apply exponential moving average
        float Alpha = 0.1f;  // Smoothing factor between 0 and 1
        DetectedLatency = Alpha * FrameLatency + (1.0f - Alpha) * DetectedLatency;
    }

    // Optional: Clamp the DetectedLatency to a reasonable maximum value
    DetectedLatency = FMath::Clamp(DetectedLatency, 0.0f, 0.1f);

    // Log a warning if the smoothed latency exceeds a certain threshold
    if (DetectedLatency > 0.05f)  // Adjust this threshold as needed
    {
        LogMessage = FString::Printf(TEXT("Latency spike detected: %f seconds."), DetectedLatency);
        LogManager.LogMessage(LogMessage, ELogVerbosity::Warning, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
}