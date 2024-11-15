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
    // Initialize variables needed for the game session
    UFUNCTION(BlueprintCallable, Category = "Initialization")
    void InitializeGameInstance();

    // Array to hold the sequence of tests
    UPROPERTY(BlueprintReadWrite, Category = "TestSequence")
    TArray<FString> TestSequence;

    // Index to track current test
    UPROPERTY(BlueprintReadWrite, Category = "TestSequence")
    int32 CurrentTestIndex;

    // Current Astronaut Profile
    UPROPERTY(BlueprintReadWrite, Category = "Profile")
    FAstronautProfile CurrentAstronautProfile;

    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    void SetCurrentAstronautProfile(int32 AstronautID);

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
