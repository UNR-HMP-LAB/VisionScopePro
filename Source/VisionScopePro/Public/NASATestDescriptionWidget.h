// NASATestDescriptionWidget.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NASATestDescriptionWidget.generated.h"

// Delegate to signal when the "Continue" button is clicked, allowing external logic to proceed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContinueClicked);

// UNASATestDescriptionWidget: Handles animated text display and user interaction for test descriptions
UCLASS()
class VISIONSCOPEPRO_API UNASATestDescriptionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Initializes the animated typewriter effect for displaying a test description
    UFUNCTION(BlueprintCallable, Category = "UI")
    void StartTestDescription(const FString& Description, float TypingSpeed = 0.05f);

    // Exposes an event triggered when the user clicks "Continue", enabling integration with other systems
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnContinueClicked OnContinueClicked;

protected:
    // Sets up widget bindings and initializes variables when the widget is constructed
    virtual void NativeConstruct() override;

private:
    // Types out characters one by one for the typewriter effect
    void TypeNextCharacter();

    // Toggles caret visibility to create a blinking effect, enhancing text readability
    void ToggleCaretVisibility();

    // Handles "Continue" button clicks, ensuring proper cleanup and signaling external logic to proceed
    UFUNCTION()
    void HandleContinueClicked();

    // Holds the complete text to display via the typewriter effect
    FString DescriptionText;

    // Tracks the current state of the text being typed, allowing incremental updates
    FString CurrentTypedText;

    // Represents a caret (e.g., "_") for visual effects during typing
    FString Caret;

    // Controls the speed of the typewriter effect in seconds per character
    float TypingSpeed;

    // Timer handles to manage the typewriter effect and caret blinking
    FTimerHandle TypingTimerHandle;
    FTimerHandle CaretBlinkTimerHandle;

    // Indicates whether the caret is currently visible, used to alternate its state
    bool bCaretVisible;

    // UI element to display the typed text with optional caret
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TestDescriptionText;

    // UI element for the "Continue" button, allowing user interaction
    UPROPERTY(meta = (BindWidget))
    class UButton* ContinueButton;
};
