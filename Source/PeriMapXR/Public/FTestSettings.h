// FTestSettings.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FTestSettings.generated.h"

USTRUCT(BlueprintType)
struct PERIMAPXR_API FTestSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float StimuliRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float StimuliDiameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float FixationPointDiameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float DegreeStep; // Degree separation between test points

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    int32 NumStimuli; // Number of stimuli points

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float MaxVerticalAngle; // Max vertical angle for stimuli distribution

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float MaxHorizontalAngle; // Max horizontal angle for stimuli distribution

    FTestSettings()
        : StimuliRadius(0.0f), StimuliDiameter(0.0f), FixationPointDiameter(0.0f), DegreeStep(0.0f), NumStimuli(0), MaxVerticalAngle(0.0f), MaxHorizontalAngle(0.0f)
    {}

    FTestSettings(float InStimuliRadius, float InStimuliDiameter, float InFixationPointDiameter, float InDegreeStep, int32 InNumStimuli, float InMaxVerticalAngle, float InMaxHorizontalAngle)
        : StimuliRadius(InStimuliRadius), StimuliDiameter(InStimuliDiameter), FixationPointDiameter(InFixationPointDiameter), DegreeStep(InDegreeStep), NumStimuli(InNumStimuli), MaxVerticalAngle(InMaxVerticalAngle), MaxHorizontalAngle(InMaxHorizontalAngle)
    {}
};