// UThresholdEstimator.cpp

#include "UThresholdEstimator.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <cmath>

// Constructor
UThresholdEstimator::UThresholdEstimator()
{
    // Default parameters for dB thresholds
    MinThresholdInDb = 0.0f;
    MaxThresholdInDb = 40.0f;
    ThresholdStepSizeInDb = 1.0f;
    StoppingCriterionInDb = 1.0f; // Standard deviation threshold for stopping
    bIsLeftEye = true;
}

// Destructor
UThresholdEstimator::~UThresholdEstimator()
{
    CleanupEstimators();
}

// Initializes the estimator for a new test
void UThresholdEstimator::Initialize(const FTestSettings& TestSettings, ETestType TestType, bool bLeftEye)
{
    bLeftEye = bIsLeftEye;
    CurrentTestSettings = TestSettings;
    CurrentTestType = TestType;

    // Cleanup any existing estimators
    CleanupEstimators();

    // Initialize estimation parameters if needed
    // MinThresholdInDb, MaxThresholdInDb, ThresholdStepSizeInDb can be set based on TestSettings or TestType
}

// Updates the estimator with a user's response at a location
void UThresholdEstimator::UpdateWithResponse(const FVector& Location, float StimulusIntensity, bool bSeen)
{
    LocationEstimator* Estimator = GetOrCreateLocationEstimator(Location);
    if (Estimator && !Estimator->bEstimationComplete)
    {
        // Debugging: Log the response details before updating the probability distribution
        UE_LOG(LogTemp, Warning, TEXT("Updating probability distribution for location %s with intensity %f dB, Seen: %s"), *Location.ToString(), StimulusIntensity, bSeen ? TEXT("True") : TEXT("False"));

        Estimator->UpdateProbabilityDistribution(StimulusIntensity, bSeen);
        RecordStimulusResult(Location, bSeen, StimulusIntensity);

        if (bSeen)
        {
            Estimator->ConsistentResponsesCount++;
            ConsistencyMap.FindOrAdd(Location)++;
        }
        else
        {
            Estimator->ConsistentResponsesCount = 0;
            ConsistencyMap.FindOrAdd(Location) = 0;
        }

        if (Estimator->IsThresholdEstimationComplete(StoppingCriterionInDb))
        {
            float EstimatedThresholdInDb = Estimator->GetThresholdEstimateInDb();
            GetCurrentThresholdMap().Add(Location, EstimatedThresholdInDb); // Use GetCurrentThresholdMap() here
            Estimator->bEstimationComplete = true;

            // Debugging: Log when the threshold estimation is completed
            UE_LOG(LogTemp, Warning, TEXT("Threshold estimation complete for location %s. Estimated threshold: %f dB"), *Location.ToString(), EstimatedThresholdInDb);
        }
    }
}

// Gets the next stimulus intensity for a location (in decibels)
float UThresholdEstimator::GetNextStimulusIntensityInDb(const FVector& Location)
{
    LocationEstimator* Estimator = GetOrCreateLocationEstimator(Location);
    if (Estimator)
    {
        return Estimator->SelectNextStimulusIntensityInDb();
    }
    return (MaxThresholdInDb - MinThresholdInDb) / 2.0f;  // Return default if no estimator
}

// Gets the next luminance for a location (in nits)
float UThresholdEstimator::GetNextLuminanceForLocation(const FVector& Location)
{
    LocationEstimator* Estimator = GetOrCreateLocationEstimator(Location);
    if (Estimator)
    {
        return Estimator->SelectNextLuminance();
    }
    return 0.0f;  // Return 0 if no estimator
}

// Checks if threshold estimation is complete for a location
bool UThresholdEstimator::IsThresholdEstimationComplete(const FVector& Location)
{
    LocationEstimator* Estimator = GetOrCreateLocationEstimator(Location);
    if (Estimator)
    {
        return Estimator->bEstimationComplete;
    }
    return true;
}

// Gets the estimated threshold for a location (in decibels)
float UThresholdEstimator::GetThresholdEstimateInDb(const FVector& Location)
{
    if (GetCurrentThresholdMap().Contains(Location))
    {
        return GetCurrentThresholdMap()[Location];
    }
    return 0.0f;
}

// Calculates final thresholds after the test is complete
void UThresholdEstimator::CalculateFinalThresholds()
{
    // Thresholds are already calculated and stored in FinalThresholdsInDb
}

// Calculates sensitivities based on the final thresholds
void UThresholdEstimator::CalculateFinalSensitivities()
{
    for (const auto& Pair : GetCurrentThresholdMap())
    {
        FVector Location = Pair.Key;
        float ThresholdInDb = Pair.Value;
        float Sensitivity = 1.0f / ThresholdInDb;  // Sensitivity is the inverse of the threshold
        GetCurrentSensitivityMap().Add(Location, Sensitivity);

        UE_LOG(LogTemp, Warning, TEXT("Sensitivity at location %s: %f"), *Location.ToString(), Sensitivity);
    }
}

// Gets the final thresholds (in decibels)
const TMap<FVector, float>& UThresholdEstimator::GetFinalThresholdsInDb() const
{
    return bIsLeftEye ? LeftEyeThresholds : RightEyeThresholds;
}

// Gets the final sensitivities
const TMap<FVector, float>& UThresholdEstimator::GetFinalSensitivities() const
{
    return bIsLeftEye ? LeftEyeSensitivities : RightEyeSensitivities;
}

// Records a stimulus result
void UThresholdEstimator::RecordStimulusResult(const FVector& Location, bool bSeen, float ThresholdLevel)
{
    FTestResults NewResult(Location, bSeen, ThresholdLevel);
    TestResultsArray.Add(NewResult);

    UE_LOG(LogTemp, Warning, TEXT("Recorded result at location %s: %s at %f dB"), *Location.ToString(), bSeen ? TEXT("Seen") : TEXT("Not Seen"), ThresholdLevel);
}

// Checks if retesting can be skipped at a location
bool UThresholdEstimator::ShouldSkipRetest(const FVector& Location)
{
    int32* ConsistencyCount = ConsistencyMap.Find(Location);
    return ConsistencyCount && *ConsistencyCount >= 3;
}

// Helper function to get or create a LocationEstimator for a location
UThresholdEstimator::LocationEstimator* UThresholdEstimator::GetOrCreateLocationEstimator(const FVector& Location)
{
    if (!LocationEstimators.Contains(Location))
    {
        TUniquePtr<LocationEstimator> NewEstimator = MakeUnique<LocationEstimator>(this);
        NewEstimator->Initialize();
        LocationEstimators.Add(Location, MoveTemp(NewEstimator));
    }
    return LocationEstimators[Location].Get();
}

// Cleans up all location estimators
void UThresholdEstimator::CleanupEstimators()
{
    LocationEstimators.Empty();
    FinalThresholdsInDb.Empty();
    TestResultsArray.Empty();
    ConsistencyMap.Empty();
}

// Returns the current threshold map for the active eye
TMap<FVector, float>& UThresholdEstimator::GetCurrentThresholdMap()
{
    return bIsLeftEye ? LeftEyeThresholds : RightEyeThresholds;
}

// Returns the current sensitivity map for the active eye
TMap<FVector, float>& UThresholdEstimator::GetCurrentSensitivityMap()
{
    return bIsLeftEye ? LeftEyeSensitivities : RightEyeSensitivities;
}

///////////////////////////////////////////////////////////
// Implementation of LocationEstimator inner class

// Constructor
UThresholdEstimator::LocationEstimator::LocationEstimator(UThresholdEstimator* InParentEstimator)
    : ParentEstimator(InParentEstimator)  // Set the pointer to the parent
{
    this->Slope = 3.0f;  // Use 'this->' to access non-static members
    this->GuessRate = 0.5f;
    this->LapseRate = 0.01f;
    this->ConsistentResponsesCount = 0;
    this->bEstimationComplete = false;
}

// Destructor
UThresholdEstimator::LocationEstimator::~LocationEstimator()
{
}

// Initializes the estimator for a location
void UThresholdEstimator::LocationEstimator::Initialize()
{
    PossibleThresholdLevelsInDb.Empty();
    ProbabilityDistribution.Empty();

    for (float Level = ParentEstimator->MinThresholdInDb; Level <= ParentEstimator->MaxThresholdInDb; Level += ParentEstimator->ThresholdStepSizeInDb)
    {
        PossibleThresholdLevelsInDb.Add(Level);
        ProbabilityDistribution.Add(1.0f); // Uniform prior
    }

    NormalizeProbabilityDistribution();
}

// Normalizes the probability distribution
void UThresholdEstimator::LocationEstimator::NormalizeProbabilityDistribution()
{
    float Sum = 0.0f;

    // Accumulate sum of probabilities
    for (float Prob : ProbabilityDistribution)
    {
        Sum += Prob;
    }

    // Normalize if the sum is positive
    if (Sum > 0.0f)
    {
        const float InvSum = 1.0f / Sum;
        for (float& Prob : ProbabilityDistribution)
        {
            Prob *= InvSum;
        }
    }
    else
    {
        // If all probabilities are zero, reset to a uniform distribution
        const float UniformProb = 1.0f / ProbabilityDistribution.Num();
        for (float& Prob : ProbabilityDistribution)
        {
            Prob = UniformProb;
        }
    }
}

// Psychometric function (cumulative Gaussian)
float UThresholdEstimator::LocationEstimator::PsychometricFunction(float StimulusIntensity, float ThresholdLevel)
{
    float Probability = GuessRate + (1.0f - GuessRate - LapseRate) * 0.5f * (1.0f + std::erf((StimulusIntensity - ThresholdLevel) / (Slope * FMath::Sqrt(2.0f))));
    return Probability;
}

// Updates the probability distribution based on user response
void UThresholdEstimator::LocationEstimator::UpdateProbabilityDistribution(float StimulusIntensity, bool bSeen)
{
    for (int32 i = 0; i < PossibleThresholdLevelsInDb.Num(); ++i)
    {
        float ThresholdLevel = PossibleThresholdLevelsInDb[i];
        float ProbabilityOfSeeing = PsychometricFunction(StimulusIntensity, ThresholdLevel);

        // Likelihood of the response given the threshold level
        float Likelihood = bSeen ? ProbabilityOfSeeing : (1.0f - ProbabilityOfSeeing);

        // Update the probability
        ProbabilityDistribution[i] *= Likelihood;
    }

    // Normalize the updated probability distribution
    NormalizeProbabilityDistribution();

    // Debugging: Log the updated probability distribution for analysis
    FString ProbabilityLog = "Updated Probability Distribution: ";
    for (int32 i = 0; i < ProbabilityDistribution.Num(); ++i)
    {
        ProbabilityLog += FString::Printf(TEXT("[%f dB: %f] "), PossibleThresholdLevelsInDb[i], ProbabilityDistribution[i]);
    }

    UE_LOG(LogTemp, Warning, TEXT("LocationEstimator - %s - Probability distribution after response: %s"), *ParentEstimator->GetName(), *ProbabilityLog);
}

// Selects the next stimulus intensity to present (in dB)
float UThresholdEstimator::LocationEstimator::SelectNextStimulusIntensityInDb()
{
    // Calculate the expected threshold (mean of the distribution)
    float ExpectedThresholdInDb = 0.0f;
    for (int32 i = 0; i < PossibleThresholdLevelsInDb.Num(); ++i)
    {
        ExpectedThresholdInDb += PossibleThresholdLevelsInDb[i] * ProbabilityDistribution[i];
    }

    // Ensure the intensity is within the valid range
    ExpectedThresholdInDb = FMath::Clamp(ExpectedThresholdInDb, PossibleThresholdLevelsInDb[0], PossibleThresholdLevelsInDb.Last());

    return ExpectedThresholdInDb;
}

// Selects the next luminance (in nits) based on the selected dB value
float UThresholdEstimator::LocationEstimator::SelectNextLuminance()
{
    float SelectedThresholdInDb = SelectNextStimulusIntensityInDb();
    return ConvertDbToLuminance(SelectedThresholdInDb);
}

// Helper function to convert dB to luminance (nits)
float UThresholdEstimator::LocationEstimator::ConvertDbToLuminance(float dBValue)
{
    float MaxLuminanceInNits = 60.0f;  // Maximum luminance of the Pico 4 display
    float Luminance = MaxLuminanceInNits * FMath::Pow(10.0f, -dBValue / 10.0f);
    return Luminance;
}

// Checks if threshold estimation is complete for this location
bool UThresholdEstimator::LocationEstimator::IsThresholdEstimationComplete(float StoppingCriterionValue)
{
    // Calculate the standard deviation of the probability distribution
    float Mean = 0.0f;
    float MeanSquare = 0.0f;
    for (int32 i = 0; i < PossibleThresholdLevelsInDb.Num(); ++i)
    {
        float Level = PossibleThresholdLevelsInDb[i];
        float Prob = ProbabilityDistribution[i];
        Mean += Level * Prob;
        MeanSquare += Level * Level * Prob;
    }
    float Variance = MeanSquare - Mean * Mean;
    float StandardDeviation = FMath::Sqrt(FMath::Max(Variance, 0.0f));

    // Check if standard deviation is below the stopping criterion
    return StandardDeviation <= StoppingCriterionValue;
}

// Gets the estimated threshold for this location (in decibels)
float UThresholdEstimator::LocationEstimator::GetThresholdEstimateInDb()
{
    float EstimatedThresholdInDb = 0.0f;
    for (int32 i = 0; i < PossibleThresholdLevelsInDb.Num(); ++i)
    {
        EstimatedThresholdInDb += PossibleThresholdLevelsInDb[i] * ProbabilityDistribution[i];
    }
    return EstimatedThresholdInDb;
}
