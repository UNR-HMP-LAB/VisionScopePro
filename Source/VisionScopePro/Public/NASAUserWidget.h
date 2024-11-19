// NASAUserWidget.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AstronautProfile.h"
#include "NASAUserWidget.generated.h"

// UNASAUserWidget: Handles UI logic for astronaut selection, test sequence management, and progress tracking.
UCLASS()
class VISIONSCOPEPRO_API UNASAUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Updates the game instance with the selected astronaut, ensuring their profile is loaded
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SelectAstronaut(int32 AstronautNumber);

    // Starts the predefined sequence of tests for the currently selected astronaut
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void StartTestSequence();

protected:
    // Initializes the widget by linking it to the game instance and setting up default behavior
    virtual void NativeConstruct() override;
};