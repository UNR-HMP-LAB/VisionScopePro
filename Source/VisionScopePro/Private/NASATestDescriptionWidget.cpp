// NASATestDescriptionWidget.cpp
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#include "NASATestDescriptionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "TimerManager.h"

void UNASATestDescriptionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize the caret
    Caret = "_";
    bCaretVisible = true;
    CurrentTypedText = "";

    // Bind the "Continue" button click event
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UNASATestDescriptionWidget::HandleContinueClicked);
    }
}

void UNASATestDescriptionWidget::StartTestDescription(const FString& Description, float NewTypingSpeed)
{
    // Initialize variables
    DescriptionText = Description;
    CurrentTypedText = "";
    NewTypingSpeed = TypingSpeed;
    bCaretVisible = true;

    // Clear any existing timers
    GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CaretBlinkTimerHandle);

    // Start the typing effect
    GetWorld()->GetTimerManager().SetTimer(
        TypingTimerHandle,
        this,
        &UNASATestDescriptionWidget::TypeNextCharacter,
        this->TypingSpeed,
        true
    );

    // We need a way to animate the caret blinking
    GetWorld()->GetTimerManager().SetTimer(
        CaretBlinkTimerHandle,
        this,
        &UNASATestDescriptionWidget::ToggleCaretVisibility,
        0.5f, // Blink interval
        true
    );
}

void UNASATestDescriptionWidget::TypeNextCharacter()
{
    // Ensure there are still characters to type
    if (CurrentTypedText.Len() < DescriptionText.Len())
    {
        // Append the next character to the typed text
        CurrentTypedText += DescriptionText[CurrentTypedText.Len()];

        // Update the text block with the current text and caret
        if (TestDescriptionText)
        {
            FString DisplayText = DescriptionText + (bCaretVisible ? Caret : "");
            TestDescriptionText->SetText(FText::FromString(DisplayText));
        }
    }
    else {
        // Typing complete so we stop appending
        GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);

        // End display of caret
        if (TestDescriptionText)
        {
            TestDescriptionText->SetText(FText::FromString(CurrentTypedText));
        }
    }
}

void UNASATestDescriptionWidget::ToggleCaretVisibility()
{
    // Toggle caret visibility
    bCaretVisible = !bCaretVisible;

    // Update the text blocks
    if (bCaretVisible)
    {
        // Declare DisplayText with caret to output to display
        FString DisplayText = CurrentTypedText + (bCaretVisible ? Caret : "");
        TestDescriptionText->SetText(FText::FromString(DisplayText));
    }
}

void UNASATestDescriptionWidget::HandleContinueClicked()
{
    // Clear all timers
    GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CaretBlinkTimerHandle);

    // Ensure final text is shown without the caret
    if (TestDescriptionText)
    {
        TestDescriptionText->SetText(FText::FromString(CurrentTypedText));
    }

    // Trigger the delegate to signal the continue event
    OnContinueClicked.Broadcast();
}
