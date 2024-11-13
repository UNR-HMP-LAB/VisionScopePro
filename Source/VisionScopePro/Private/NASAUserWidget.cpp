// Fill out your copyright notice in the Description page of Project Settings.


#include "NASAUserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

void UNASAUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize the test sequence
    SetupTestSequence();
}

void UNASAUserWidget::SelectAstronaut(int32 AstronautNumber)
{
    // Set the selected astronaut based on the number
    SelectedAstronaut = FString::Printf(TEXT("Astronaut %d"), AstronautNumber);
    FString Description = GetAstronautDescription(AstronautNumber);

    // Display the astronaut description (TextBlock in the UI for this)
    if (UTextBlock* AstronautInfoText = Cast<UTextBlock>(GetWidgetFromName("AstronautInfoText")))
    {
        AstronautInfoText->SetText(FText::FromString(Description));
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
    // Start the first test
    if (TestSequence.Num() > 0)
    {
        FString FirstTest = TestSequence[0];
        // Call a function to launch the first test based on FirstTest name (through GameMode)
        // Assuming a LaunchSpecificTest() function exists in GameMode or another manager
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PC->ConsoleCommand(FString::Printf(TEXT("LaunchSpecificTest %s"), *FirstTest));
        }
    }
}

void UNASAUserWidget::SetupTestSequence()
{
    // Define the sequence of tests
    TestSequence.Add("RAPD");
    TestSequence.Add("Static VA");
    TestSequence.Add("Color");
    TestSequence.Add("Contrast");
    TestSequence.Add("Visual Field");
}

FString UNASAUserWidget::GetAstronautDescription(int32 AstronautNumber) const
{
    switch (AstronautNumber)
    {
    case 1:
        return "Astronaut 1: .";
    case 2:
        return "Astronaut 2: .";
    case 3:
        return "Astronaut 3: .";
    case 4:
        return "Astronaut 4: .";
    default:
        return "Unknown Astronaut";
    }
}