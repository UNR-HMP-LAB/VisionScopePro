// TestResults.h
#pragma once

#include "CoreMinimal.h"
#include "TestResults.generated.h"
#include "HAL/PlatformFilemanager.h"

USTRUCT(BlueprintType)
struct FTestResults {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stimulus Result")
    FVector StimulusLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stimulus Result")
    bool bStimulusSeen;

    // Constructor for ease of use
    FTestResults(FVector Location = FVector::ZeroVector, bool Seen = false)
        : StimulusLocation(Location), bStimulusSeen(Seen) {}
};