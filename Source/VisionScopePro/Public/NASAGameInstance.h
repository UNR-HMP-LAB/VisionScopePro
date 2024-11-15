// NASAGameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AstronautProfile.h"
#include "NASAGameInstance.generated.h"

UCLASS()
class VISIONSCOPEPRO_API UNASAGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Load an astronaut profile from a saved file
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    bool LoadAstronautProfile(FString AstronautID, FAstronautProfile& OutProfile);

    // Save an astronaut profile to a saved file
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    bool SaveAstronautProfile(const FAstronautProfile& Profile);

    // Log test results for a specific astronaut, updating their profile with a new test record
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    void LogTestResult(FString AstronautID, const FAstronautTestRecord& NewTestRecord);
};
