// UThresholdEstimator.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FTestResults.h"
#include "FTestSettings.h"
#include "ETestType.h"
#include "UThresholdEstimator.generated.h"

/**
 * UThresholdEstimator class encapsulates all threshold estimation logic,
 * including managing per-location estimations, recording results, and
 * calculating final thresholds and sensitivities.
 */
UCLASS(Blueprintable, BlueprintType)
class PERIMAPXR_API UThresholdEstimator : public UObject
{
    GENERATED_BODY()

public:
    // Constructor and Destructor
    UThresholdEstimator();
    virtual ~UThresholdEstimator();

    // Initialize the estimator for the given test settings
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    void Initialize(const FTestSettings& TestSettings, ETestType TestType, bool bIsLeftEye);

    // Updates the estimator with a user's response at a location
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    void UpdateWithResponse(const FVector& Location, float StimulusIntensity, bool bSeen);

    // Gets the next stimulus intensity for a location (in decibels)
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    float GetNextStimulusIntensityInDb(const FVector& Location);

    // Gets the next luminance for a location (in nits)
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    float GetNextLuminanceForLocation(const FVector& Location);

    // Checks if threshold estimation is complete for a location
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    bool IsThresholdEstimationComplete(const FVector& Location);

    // Gets the estimated threshold for a location (in decibels)
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    float GetThresholdEstimateInDb(const FVector& Location);

    // Calculates final thresholds and sensitivities after the test is complete
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    void CalculateFinalThresholds();
    
	// Calculates final sensitivities based on thresholds
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    void CalculateFinalSensitivities();

    // Gets the final thresholds (in decibels)
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    const TMap<FVector, float>& GetFinalThresholdsInDb() const;

    // Gets the final sensitivities (based on thresholds)
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    const TMap<FVector, float>& GetFinalSensitivities() const;

    // Records a stimulus result
    void RecordStimulusResult(const FVector& Location, bool bSeen, float ThresholdLevel);

    // Checks if retesting can be skipped at a location
    UFUNCTION(BlueprintCallable, Category = "Threshold Estimation")
    bool ShouldSkipRetest(const FVector& Location);

private:
    // Inner class for per-location threshold estimation
    class LocationEstimator
    {
    public:
        LocationEstimator(UThresholdEstimator* InParentEstimator);
        ~LocationEstimator();

        void Initialize();

        void UpdateProbabilityDistribution(float StimulusIntensity, bool bSeen);

        float SelectNextStimulusIntensityInDb();
        float SelectNextLuminance();  // Converts dB to luminance in nits

        bool IsThresholdEstimationComplete(float StoppingCriterion);

        float GetThresholdEstimateInDb();

        // Keeps track of consistent responses
        int32 ConsistentResponsesCount;

        // Stores whether estimation is complete
        bool bEstimationComplete;

    private:
        // Pointer to the outer UThresholdEstimator class
        UThresholdEstimator* ParentEstimator;

        TArray<float> PossibleThresholdLevelsInDb;  // Threshold levels in decibels
        TArray<float> ProbabilityDistribution;

        void NormalizeProbabilityDistribution();

        float PsychometricFunction(float StimulusIntensity, float ThresholdLevel);  

        // Parameters for the psychometric function
        float Slope;
        float GuessRate;
        float LapseRate;

        // Stopping criterion for standard deviation
        float StoppingCriterion;

        // Helper to convert dB to luminance (nits)
        float ConvertDbToLuminance(float dBValue);
    };

    // Maps to store results for both eyes separately
    TMap<FVector, float> LeftEyeThresholds;
    TMap<FVector, float> RightEyeThresholds;

    // Sensitivities for each location
    TMap<FVector, float> LeftEyeSensitivities;
    TMap<FVector, float> RightEyeSensitivities;

    // Store the current test settings and type
    FTestSettings CurrentTestSettings;
    ETestType CurrentTestType;

    // Helper functions for eye-specific data management
    TMap<FVector, float>& GetCurrentThresholdMap();
    TMap<FVector, float>& GetCurrentSensitivityMap();

    // Smart pointer for memory management
    TMap<FVector, TUniquePtr<LocationEstimator>> LocationEstimators;

    // Final thresholds and sensitivities for each location
    TMap<FVector, float> FinalThresholdsInDb;
    TMap<FVector, float> FinalSensitivities;

    // Test results
    TArray<FTestResults> TestResultsArray;

    // Consistency map for retest skipping
    TMap<FVector, int32> ConsistencyMap;

    // Threshold estimation parameters
    float MinThresholdInDb;
    float MaxThresholdInDb;
    float StepSize;
    float ThresholdStepSizeInDb;
    float StoppingCriterionInDb;

    // Is left eye being tested
    bool bIsLeftEye;

    // Helper functions
    LocationEstimator* GetOrCreateLocationEstimator(const FVector& Location);
    void CleanupEstimators();
};