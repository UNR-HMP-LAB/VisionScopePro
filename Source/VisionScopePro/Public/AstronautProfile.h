// AstronautProfile.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AstronautProfile.generated.h"

// Represents a single test record for an astronaut, storing key metadata and file paths for results
USTRUCT(BlueprintType)
struct FAstronautTestRecord
{
    GENERATED_BODY()

    // Timestamp for when the test was conducted, formatted in ISO 8601 for consistency and sorting
    UPROPERTY()
    FString Timestamp;

    // File path for results of the RAPD test, stored as a CSV for data analysis
    UPROPERTY()
    FString RAPDTestFile;

    // File path for results of the Static Visual Acuity test
    UPROPERTY()
    FString StaticVATestFile;

    // File path for results of the Color Vision test
    UPROPERTY()
    FString ColorTestFile;

    // File path for results of the Contrast Sensitivity test
    UPROPERTY()
    FString ContrastTestFile;

    // File path for results of the Visual Field test
    UPROPERTY()
    FString VisualFieldTestFile;
};

// Represents an astronaut's profile, including a unique identifier and a history of test records
// Facilitates profile-based management of test data for individual astronauts across multiple sessions
USTRUCT(BlueprintType)
struct FAstronautProfile
{
    GENERATED_BODY()

    // Unique identifier for the astronaut, used to link profiles to their respective data files
    UPROPERTY()
    FString AstronautID;

    // Array of test records, storing results from multiple test sessions for the astronaut
    UPROPERTY()
    TArray<FAstronautTestRecord> TestHistory;
};
