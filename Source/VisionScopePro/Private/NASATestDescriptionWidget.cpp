// NASATestDescriptionWidget.cpp
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#include "NASATestDescriptionWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "TimerManager.h"

// Initializes the widget by setting up bindings and preparing visual states
void UNASATestDescriptionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize caret visual for typewriter effect
    Caret = "_";
    bCaretVisible = true;
    CurrentTypedText = "";

    // Bind the "Continue" button to handle user clicks and allow test progression
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UNASATestDescriptionWidget::HandleContinueClicked);
    }

    // Make the text box read-only
    if (TestDescriptionText)
    {
        TestDescriptionText->SetIsReadOnly(true); 
    }
}

// Begins the typewriter effect to display the provided description with animated typing
// Clears any previous animations to ensure a consistent start
void UNASATestDescriptionWidget::StartTestDescription(const FString& Description, float NewTypingSpeed)
{
    // Initialize typing state with new description and reset caret visibility
    DescriptionText = Description;
    CurrentTypedText = "";
    NewTypingSpeed = TypingSpeed;
    bCaretVisible = true;

    // Clear existing timers to prevent overlapping animations
    GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CaretBlinkTimerHandle);

    // Start typing animation by appending characters incrementally
    GetWorld()->GetTimerManager().SetTimer(
        TypingTimerHandle,
        this,
        &UNASATestDescriptionWidget::TypeNextCharacter,
        this->TypingSpeed,
        true
    );

    // Start caret blinking to enhance user feedback during typing
    GetWorld()->GetTimerManager().SetTimer(
        CaretBlinkTimerHandle,
        this,
        &UNASATestDescriptionWidget::ToggleCaretVisibility,
        0.5f, // Blink interval in seconds
        true
    );
}

// Appends the next character of the description to simulate typing
// Stops the timer once the entire text has been displayed
void UNASATestDescriptionWidget::TypeNextCharacter()
{
    // Ensure there are still characters to type
    if (CurrentTypedText.Len() < DescriptionText.Len())
    {
        // Append the next character to the displayed text
        CurrentTypedText += DescriptionText[CurrentTypedText.Len()];

        // Update the text block to reflect the current state with the caret
        if (TestDescriptionText)
        {
            FString DisplayText = DescriptionText + (bCaretVisible ? Caret : "");
            TestDescriptionText->SetText(FText::FromString(DisplayText));
        }
    }
    else {
        // Stop typing animation once all characters are displayed
        GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);

        // Finalize the display by removing the caret
        if (TestDescriptionText)
        {
            TestDescriptionText->SetText(FText::FromString(CurrentTypedText));
        }
    }
}

// Toggles the visibility of the caret for a blinking effect during typing
void UNASATestDescriptionWidget::ToggleCaretVisibility()
{
    // Toggle caret visibility state
    bCaretVisible = !bCaretVisible;

    // Update the displayed text to include or exclude the caret as needed
    if (bCaretVisible)
    {
        // Declare DisplayText with caret to output to display
        FString DisplayText = CurrentTypedText + (bCaretVisible ? Caret : "");
        TestDescriptionText->SetText(FText::FromString(DisplayText));
    }
}

// Handles the "Continue" button click by stopping animations and signaling readiness to proceed
void UNASATestDescriptionWidget::HandleContinueClicked()
{
    // Stop all timers to finalize the typewriter and caret animations
    GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CaretBlinkTimerHandle);

    // Ensure the final state of the text is displayed without the caret
    if (TestDescriptionText)
    {
        TestDescriptionText->SetText(FText::FromString(CurrentTypedText));
    }

    // Trigger the delegate to signal the continue event that the user is ready to continue
    OnContinueClicked.Broadcast();
}
