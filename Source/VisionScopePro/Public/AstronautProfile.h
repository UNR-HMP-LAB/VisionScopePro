// AstronautProfile.h

#pragma once

#include "CoreMinimal.h"
#include "AstronautProfile.generated.h"

// Structure to hold test data information with timestamp and CSV filename
USTRUCT(BlueprintType)
struct FAstronautTestRecord
{
    GENERATED_BODY()

    UPROPERTY()
    FString Timestamp; // ISO 8601 format

    UPROPERTY()
    FString RAPDTestFile;

    UPROPERTY()
    FString StaticVATestFile;

    UPROPERTY()
    FString ColorTestFile;

    UPROPERTY()
    FString ContrastTestFile;

    UPROPERTY()
    FString VisualFieldTestFile;
};

// Structure to hold the astronaut's profile information with a history of test records
USTRUCT(BlueprintType)
struct FAstronautProfile
{
    GENERATED_BODY()

    UPROPERTY()
    FString AstronautID;

    UPROPERTY()
    TArray<FAstronautTestRecord> TestHistory; // Collection of test sessions with timestamps
};
