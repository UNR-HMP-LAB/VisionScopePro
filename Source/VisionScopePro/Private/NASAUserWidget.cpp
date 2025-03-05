// NASAUserWidget.cpp
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#include "NASAUserWidget.h"
#include "NASAGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Configures the widget after its construction and links it to the active game instance
// Ensures that the game instance can reference this widget for updates and actions
void UNASAUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Attempt to retrieve the game instance and set this widget as the current active widget
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GameInstance->CurrentWidget = this;
    }
}

// Updates the selected astronaut in the game instance, enabling test data retrieval and updates
// Ensures that the correct profile is loaded for subsequent test sessions
void UNASAUserWidget::SelectAstronaut(int32 AstronautNumber)
{
    // Retrieve the game instance to update the current astronaut profile
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GameInstance->SetCurrentAstronautProfile(AstronautNumber);
    }
}

// Validates the game state and initiates the test sequence for the selected astronaut
// Ensures that both the test sequence and astronaut profile are initialized before starting
void UNASAUserWidget::StartTestSequence()
{
    // Retrieve the game instance for managing the test sequence
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        // Validate that the test sequence is properly initialized
        if (GameInstance->TestSequence.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Test sequence is uninitialized!"));
            return;
        }

        // Validate that the astronaut profile is correctly loaded
        if (GameInstance->CurrentAstronautProfile.AstronautID.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Astronaut profile is uninitialized!"));
            return;
        }

        // Proceed to launch the next test in the sequence
        GameInstance->LaunchNextTest();
    }
}