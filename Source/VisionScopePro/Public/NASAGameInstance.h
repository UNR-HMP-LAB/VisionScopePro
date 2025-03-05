// NASAGameInstance.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AstronautProfile.h"
#include "NASAUserWidget.h"
#include "NASATestDescriptionWidget.h"
#include "NASAGameInstance.generated.h"

// UNASAGameInstance: Extends UGameInstance to centralize test flow, profile management, and UI handling
UCLASS()
class VISIONSCOPEPRO_API UNASAGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Initializes game-specific variables like test sequence order
    UFUNCTION(BlueprintCallable, Category = "Initialization")
    void InitializeGameInstance();

    // Blueprint-accessible array to hold sequence of tests to control test order and logic
    UPROPERTY(BlueprintReadWrite, Category = "TestSequence")
    TArray<FString> TestSequence;

    // Tracks the active test's index within the sequence for state management
    UPROPERTY(BlueprintReadWrite, Category = "TestSequence")
    int32 CurrentTestIndex;

    // Specifies the test type being executed; used for logic branching at runtime
    UPROPERTY(BlueprintReadWrite, Category = "TestSequence")
    FString DesiredTestType;

    // Stores data for the currently active astronaut profile
    UPROPERTY(BlueprintReadWrite, Category = "Profile")
    FAstronautProfile CurrentAstronautProfile;

    // References the current active UI widget for menu navigation or updates
    UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    UUserWidget* CurrentWidget;

    // References the widget to be used for the test description
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> TestDescriptionWidgetClass;

    // Updates the currently loaded astronaut profile using the provided ID
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    void SetCurrentAstronautProfile(int32 AstronautID);

    // Attempts to load a saved astronaut profile from disk, initializing new profiles as needed
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    bool LoadAstronautProfile(FString AstronautID, FAstronautProfile& OutProfile);

    // Persists the provided astronaut profile to disk in JSON format for future retrieval
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    bool SaveAstronautProfile(const FAstronautProfile& Profile);

    // Logs test results for a specific astronaut by appending new test data to their profile
    UFUNCTION(BlueprintCallable, Category = "AstronautProfiles")
    void LogTestResult(FString AstronautID, const FAstronautTestRecord& NewTestRecord);

    // Advances the test sequence to the next step and updates the desired test type
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void LaunchNextTest();

    // Updates the displayed test description shown in the UI for the active test
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ShowTestDescription();
};
