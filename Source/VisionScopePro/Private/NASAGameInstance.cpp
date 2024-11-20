// NASAGameInstance.cpp
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#include "NASAGameInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Kismet/GameplayStatics.h"

// Loads a saved astronaut profile from a JSON file or initializes a new profile if unavailable
// Returns true if the profile was successfully loaded, false if a new profile was created
bool UNASAGameInstance::LoadAstronautProfile(FString AstronautID, FAstronautProfile& OutProfile)
{
    // Determine file path for the astronaut profile
    FString FilePath = FPaths::ProjectSavedDir() + AstronautID + "_Profile.json";

    FString FileContent;
    // If the file does not exist, return a new profile with the given ID
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        // Initialize new profile if no file is found
        OutProfile.AstronautID = AstronautID;
        return false;
    }

    // Parse the JSON content into a profile object
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        OutProfile.AstronautID = JsonObject->GetStringField("AstronautID");

        // Deserialize test history if present in the JSON
        const TArray<TSharedPtr<FJsonValue>>* TestHistoryArray;
        if (JsonObject->TryGetArrayField("TestHistory", TestHistoryArray))
        {
            for (const TSharedPtr<FJsonValue>& Value : *TestHistoryArray)
            {
                TSharedPtr<FJsonObject> TestObject = Value->AsObject();
                FAstronautTestRecord Record;
                Record.Timestamp = TestObject->GetStringField("Timestamp");
                Record.RAPDTestFile = TestObject->GetStringField("RAPDTestFile");
                Record.StaticVATestFile = TestObject->GetStringField("StaticVATestFile");
                Record.ColorTestFile = TestObject->GetStringField("ColorTestFile");
                Record.ContrastTestFile = TestObject->GetStringField("ContrastTestFile");
                Record.VisualFieldTestFile = TestObject->GetStringField("VisualFieldTestFile");
                OutProfile.TestHistory.Add(Record);
            }
        }
        return true;
    }
    return false;
}

// Saves the given astronaut profile to a JSON file
// Serializes profile data, including test history, for persistent storage
bool UNASAGameInstance::SaveAstronautProfile(const FAstronautProfile& Profile)
{
    // Construct file path based on the astronaut ID
    FString FilePath = FPaths::ProjectSavedDir() + Profile.AstronautID + "_Profile.json";

    // Create a JSON object to represent the profile
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("AstronautID", Profile.AstronautID);

    // Serialize test history into a JSON array
    TArray<TSharedPtr<FJsonValue>> TestHistoryArray;
    for (const FAstronautTestRecord& Record : Profile.TestHistory)
    {
        TSharedPtr<FJsonObject> TestObject = MakeShareable(new FJsonObject);
        TestObject->SetStringField("Timestamp", Record.Timestamp);
        TestObject->SetStringField("RAPDTestFile", Record.RAPDTestFile);
        TestObject->SetStringField("StaticVATestFile", Record.StaticVATestFile);
        TestObject->SetStringField("ColorTestFile", Record.ColorTestFile);
        TestObject->SetStringField("ContrastTestFile", Record.ContrastTestFile);
        TestObject->SetStringField("VisualFieldTestFile", Record.VisualFieldTestFile);
        TestHistoryArray.Add(MakeShareable(new FJsonValueObject(TestObject)));
    }
    JsonObject->SetArrayField("TestHistory", TestHistoryArray);

    // Write JSON data to a string and save it to the file
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

// Logs a new test result by appending it to the astronaut's profile
// If the profile does not exist, initializes a new profile
void UNASAGameInstance::LogTestResult(FString AstronautID, const FAstronautTestRecord& NewTestRecord)
{
    FAstronautProfile Profile;
    // Load existing profile or create a new one if not found
    if (!LoadAstronautProfile(AstronautID, Profile))
    {
        Profile.AstronautID = AstronautID;
    }

    // Append the new test record and save the updated profile
    Profile.TestHistory.Add(NewTestRecord);
    SaveAstronautProfile(Profile);
}

// Initializes the test sequence and resets relevant variables for gameplay
void UNASAGameInstance::InitializeGameInstance()
{
    // Define the sequence of tests in the desired order
    TestSequence.Add("RAPD");
    TestSequence.Add("Static VA");
    TestSequence.Add("Color");
    TestSequence.Add("Contrast");
    TestSequence.Add("Visual Field");

    // Reset the current test index to the beginning of the sequence
    CurrentTestIndex = 0;
}

// Sets the active astronaut profile using the given ID
// Attempts to load the profile from disk, creating a new one if it doesn't exist
void UNASAGameInstance::SetCurrentAstronautProfile(int32 AstronautID)
{
    // Convert the astronaut ID to a string for file handling
    FString AstronautIDString = FString::FromInt(AstronautID);

    // Set the Astronaut ID in the CurrentAstronautProfile struct
    CurrentAstronautProfile.AstronautID = AstronautIDString;

    // Load the astronaut profile or initialize a new one if not found
    if (!LoadAstronautProfile(AstronautIDString, CurrentAstronautProfile))
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating new profile for Astronaut ID: %d"), AstronautID);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Loaded existing profile for Astronaut ID: %d"), AstronautID);
    }

    // Update the test description widget if it's active
    if (CurrentWidget)
    {
        if (UNASATestDescriptionWidget* TestDescriptionWidget = Cast<UNASATestDescriptionWidget>(CurrentWidget))
        {
            TestDescriptionWidget->UpdateTestUI();
        }
    }
}

// Advances to the next test in the sequence by changing levels and updating test-specific logic
void UNASAGameInstance::LaunchNextTest()
{
    // Determine the current test based on the sequence index
    if (!TestSequence.IsValidIndex(CurrentTestIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid CurrentTestIndex: %d"), CurrentTestIndex);
        return;
    }
    FString CurrentTest = TestSequence[CurrentTestIndex];

    // Open the corresponding level and set the desired test type
    if (CurrentTest == "RAPD")
    {
        DesiredTestType = "RAPD";
        UGameplayStatics::OpenLevel(this, FName("RAPD"));
    }
    else if (CurrentTest == "Static VA")
    {
        DesiredTestType = "Static VA";
        UGameplayStatics::OpenLevel(this, FName("EyeTests"));
    }
    else if (CurrentTest == "Color")
    {
        DesiredTestType = "Color";
        UGameplayStatics::OpenLevel(this, FName("ColorTest"));
    }
    else if (CurrentTest == "Contrast")
    {
        DesiredTestType = "CS";
        UGameplayStatics::OpenLevel(this, FName("EyeTests"));
    }
    else if (CurrentTest == "Visual Field")
    {
        DesiredTestType = "10-2";
        UGameplayStatics::OpenLevel(this, FName("VisualField"));
    }

    // Increment to the next test for subsequent calls
    CurrentTestIndex++;

    // Update the test description widget if it's active
    if (CurrentWidget)
    {
        if (UNASATestDescriptionWidget* TestDescriptionWidget = Cast<UNASATestDescriptionWidget>(CurrentWidget))
        {
            TestDescriptionWidget->UpdateTestUI();
        }
    }
}

// Displays a description for the current test based on its type
// Removes any existing widget before allowing a Blueprint to handle UI setup
void UNASAGameInstance::ShowTestDescription()
{
    // Validate that the test sequence and current index are properly initialized
    if (TestSequence.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Test sequence is uninitialized!"));
        return;
    }

    if (!TestSequence.IsValidIndex(CurrentTestIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid CurrentTestIndex: %d"), CurrentTestIndex);
        return;
    }

    // Determine the description for the current test
    FString CurrentTest = TestSequence[CurrentTestIndex];
    FString TestDescription;

    if (CurrentTest == "RAPD")
    {
        TestDescription = "This is the RAPD test. It will assess your pupillary response to light (~2 mins).";
    }
    else if (CurrentTest == "Static VA")
    {
        TestDescription = "This is the Static Visual Acuity test. It evaluates your ability to see stationary letters (~5 mins).";
    }
    else if (CurrentTest == "Color")
    {
        TestDescription = "This is the Color Vision test. It evaluates your ability to distinguish colors (~3 mins).";
    }
    else if (CurrentTest == "Contrast")
    {
        TestDescription = "This is the Contrast Sensitivity test. It evaluates your ability to detect contrast (~4 mins).";
    }
    else if (CurrentTest == "Visual Field")
    {
        TestDescription = "This is the Visual Field test. It evaluates your peripheral vision (~10 mins).";
    }
    else
    {
        TestDescription = "Unknown test.";
        UE_LOG(LogTemp, Warning, TEXT("Unknown test type: %s"), *CurrentTest);
    }

    // Remove the current widget from the viewport if one exists
    if (CurrentWidget)
    {
        CurrentWidget->RemoveFromParent();
        CurrentWidget = nullptr;
    }

    // Spawn the Test Description Widget
    if (UWorld* World = GetWorld())
    {
        if (TestDescriptionWidgetClass)
        {
            CurrentWidget = CreateWidget<UUserWidget>(World, TestDescriptionWidgetClass);
            if (CurrentWidget)
            {
                CurrentWidget->AddToViewport();

                if (UNASATestDescriptionWidget* TestDescriptionWidget = Cast<UNASATestDescriptionWidget>(CurrentWidget))
                {
                    TestDescriptionWidget->StartTestDescription(TestDescription);

                    // Bind the "Continue" button to start the RAPD test or next action
                    TestDescriptionWidget->OnContinueClicked.AddDynamic(this, &UNASAGameInstance::LaunchNextTest);
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("TestDescriptionWidgetClass is not set!"));
        }
    }
}