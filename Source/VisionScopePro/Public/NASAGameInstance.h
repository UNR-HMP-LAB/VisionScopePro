// NASAGameInstance.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AstronautProfile.h"
#include "NASAUserWidget.h"
#include "NASATestDescriptionWidget.h"
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

    // Variable that tells the level how to control test-specific logic
    UPROPERTY(BlueprintReadWrite, Category = "TestSequence")
    FString DesiredTestType;

    // Current Astronaut Profile
    UPROPERTY(BlueprintReadWrite, Category = "Profile")
    FAstronautProfile CurrentAstronautProfile;

    // Reference to the active widget
    UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    UUserWidget* CurrentWidget;

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

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void LaunchNextTest();

    // Updates the displayed test description
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ShowTestDescription();
};
