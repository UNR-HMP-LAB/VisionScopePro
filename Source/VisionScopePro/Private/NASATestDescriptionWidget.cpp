// NASATestDescriptionWidget.cpp
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#include "NASATestDescriptionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UNASATestDescriptionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind the "Continue" button click event
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UNASATestDescriptionWidget::HandleContinueClicked);
    }
}

void UNASATestDescriptionWidget::SetTestDescription(const FString& Description)
{
    if (TestDescriptionText)
    {
        TestDescriptionText->SetText(FText::FromString(Description));
    }
}

void UNASATestDescriptionWidget::HandleContinueClicked()
{
    // Trigger the delegate
    OnContinueClicked.Broadcast();
}
