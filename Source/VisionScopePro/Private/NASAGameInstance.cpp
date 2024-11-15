// NASAGameInstance.cpp

#include "NASAGameInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

bool UNASAGameInstance::LoadAstronautProfile(FString AstronautID, FAstronautProfile& OutProfile)
{
    FString FilePath = FPaths::ProjectSavedDir() + AstronautID + "_Profile.json";

    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        // If loading fails, it may be a new profile
        OutProfile.AstronautID = AstronautID;
        return false;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        OutProfile.AstronautID = JsonObject->GetStringField("AstronautID");

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

bool UNASAGameInstance::SaveAstronautProfile(const FAstronautProfile& Profile)
{
    FString FilePath = FPaths::ProjectSavedDir() + Profile.AstronautID + "_Profile.json";

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("AstronautID", Profile.AstronautID);

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

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

void UNASAGameInstance::LogTestResult(FString AstronautID, const FAstronautTestRecord& NewTestRecord)
{
    FAstronautProfile Profile;
    if (!LoadAstronautProfile(AstronautID, Profile))
    {
        // New profile if loading fails
        Profile.AstronautID = AstronautID;
    }

    Profile.TestHistory.Add(NewTestRecord);
    SaveAstronautProfile(Profile);
}
