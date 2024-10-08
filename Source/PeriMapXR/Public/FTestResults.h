// FTestResults.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FTestResults.generated.h"

USTRUCT(BlueprintType)
struct PERIMAPXR_API FTestResults
{
    GENERATED_BODY()

public:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stimulus Result")
    FVector Location;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stimulus Result")
    bool bSeen;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stimulus Result")
    float ThresholdLevel;

    // Constructor for ease of use
    FTestResults(FVector Location = FVector::ZeroVector, bool Seen = false, float Level = 0.0f)
        : Location(Location), bSeen(Seen), ThresholdLevel(Level) {}
};
