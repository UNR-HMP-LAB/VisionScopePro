// NASAUserWidget.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AstronautProfile.h"
#include "NASAUserWidget.generated.h"

UCLASS()
class VISIONSCOPEPRO_API UNASAUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Called when the user selects an astronaut
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SelectAstronaut(int32 AstronautNumber);

    // Retrieve the overview of the current test sequence
    UFUNCTION(BlueprintCallable, Category = "Menu")
    FString GetTestOverview() const;

    // Starts the test sequence for the currently selected astronaut
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void StartTestSequence();

    // Display progress of the current test sequence
    UFUNCTION(BlueprintCallable, Category = "Menu")
    FString ShowProgress() const;

    // Function to update the test description
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateTestDescription(const FString& TestDescription);

protected:
    // Initialize widget behavior
    virtual void NativeConstruct() override;

private:
    void ResetWidgetState();
};