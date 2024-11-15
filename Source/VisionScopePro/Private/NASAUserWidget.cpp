// NASAUserWidget.cpp

#include "NASAUserWidget.h"
#include "NASAGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"

void UNASAUserWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UNASAUserWidget::SelectAstronaut(int32 AstronautNumber)
{
    // Get astronaut profile using GameInstance
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        FString AstronautID = FString::Printf(TEXT("Astronaut %d"), AstronautNumber);
        FAstronautProfile Profile;
        GameInstance->LoadAstronautProfile(AstronautID, Profile);
        // Now you can display or modify this profile as needed
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
    // Log the starting of each test, this would interact with the GameInstance for each test
    if (UNASAGameInstance* GameInstance = Cast<UNASAGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        FAstronautTestRecord NewTestRecord;
        NewTestRecord.Timestamp = FDateTime::UtcNow().ToIso8601();
        NewTestRecord.RAPDTestFile = "RAPD_" + NewTestRecord.Timestamp + ".csv";
        NewTestRecord.StaticVATestFile = "StaticVA_" + NewTestRecord.Timestamp + ".csv";
        NewTestRecord.ColorTestFile = "Color_" + NewTestRecord.Timestamp + ".csv";
        NewTestRecord.ContrastTestFile = "Contrast_" + NewTestRecord.Timestamp + ".csv";
        NewTestRecord.VisualFieldTestFile = "VisualField_" + NewTestRecord.Timestamp + ".csv";

        GameInstance->LogTestResult("AstronautID", NewTestRecord);
    }
}
