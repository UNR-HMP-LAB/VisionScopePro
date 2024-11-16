// NASAUserWidget.cpp
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#include "NASAUserWidget.h"
#include "NASAGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Components/TextBlock.h"

void UNASAUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Reset widget state on initialization
    ResetWidgetState();

    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GameInstance->CurrentWidget = this;
    }
}

void UNASAUserWidget::ResetWidgetState()
{
    // Clear any previously displayed text or states
    if (UTextBlock* TestDescriptionText = Cast<UTextBlock>(GetWidgetFromName(TEXT("TestDescriptionText"))))
    {
        TestDescriptionText->SetText(FText::GetEmpty());
    }
    // Additional reset logic for other widgets can go here
}

void UNASAUserWidget::SelectAstronaut(int32 AstronautNumber)
{
    // Get astronaut profile using GameInstance
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        // Set the selected astronaut profile in GameInstance
        GameInstance->SetCurrentAstronautProfile(AstronautNumber);
    }
}

FString UNASAUserWidget::GetTestOverview() const
{
    FString OverviewText = "Test Sequence:\n";
    OverviewText += "1. RAPD - Rapid Pupillary Response Test (~2 mins)\n";
    OverviewText += "2. Static VA - Visual Acuity Test (~5 mins)\n";
    OverviewText += "3. Color - Color Vision Test (~3 mins)\n";
    OverviewText += "4. Contrast - Contrast Sensitivity Test (~4 mins)\n";
    OverviewText += "5. Visual Field - Peripheral Vision Test (~10 mins)\n";
    return OverviewText;
}

void UNASAUserWidget::StartTestSequence()
{
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        // Error handling for uninitialized TestSequence or CurrentAstronautProfile
        if (GameInstance->TestSequence.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Test sequence is uninitialized!"));
            return;
        }

        if (GameInstance->CurrentAstronautProfile.AstronautID.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Astronaut profile is uninitialized!"));
            return;
        }

        // Launch the next test in the sequence
        GameInstance->LaunchNextTest();
    }
}

void UNASAUserWidget::UpdateTestDescription(const FString& TestDescription)
{
    if (UTextBlock* TestDescriptionText = Cast<UTextBlock>(GetWidgetFromName(TEXT("TestDescriptionText"))))
    {
        TestDescriptionText->SetText(FText::FromString(TestDescription));
    }
}

FString UNASAUserWidget::ShowProgress() const
{
    if (const UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        // Error handling for uninitialized TestSequence
        if (GameInstance->TestSequence.IsEmpty())
        {
            return FString(TEXT("No tests available."));
        }

        // Return the progress as "Test X of Y"
        return FString::Printf(TEXT("Test %d of %d"),
            GameInstance->CurrentTestIndex + 1,
            GameInstance->TestSequence.Num());
    }

    return FString(TEXT("GameInstance is not valid."));
}