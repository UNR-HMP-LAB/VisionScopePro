// FLogManager.cpp


#include "FLogManager.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"

FLogManager::FLogManager()
{
    // Get the current date and time for the filename
    FString DateTimeString = GetDateTimeString();
    FString LogFileName = FString::Printf(TEXT("Log_%s.txt"), *DateTimeString);

    // Set the path for the log file
    LogFilePath = FPaths::ProjectLogDir() + LogFileName;

    // Make sure the directory exists
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FPaths::ProjectLogDir()))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::ProjectLogDir());
    }

    // Create or clear the log file
    FFileHelper::SaveStringToFile(TEXT("Log Initialized\n"), *LogFilePath);
}

FLogManager::~FLogManager()
{
}

void FLogManager::LogMessage(const FString& Message, ELogVerbosity::Type Verbosity, float DisplayTime,
    bool bEnableConsoleMessages, bool bEnableOnScreenMessages, bool bEnableSaveToLog)
{
    // Log to the console if enabled
    if (bEnableConsoleMessages)
    {
        LogToConsole(Message, Verbosity);
    }

    // Log to the file if enabled
    if (bEnableSaveToLog)
    {
        // Prepend the message with a timestamp
        FString LogEntry = FString::Printf(TEXT("[%s] %s\n"), *FDateTime::Now().ToString(), *Message);

        // Append the log entry to the log file
        FFileHelper::SaveStringToFile(LogEntry, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
    }

    // Display the message on screen if enabled
    if (bEnableOnScreenMessages)
    {
        DisplayOnScreen(Message, Verbosity, DisplayTime);
    }
}

FString FLogManager::GetDateTimeString() const
{
    FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%Y%m%d_%H%M%S"));
}

// Display messages on the screen with verbosity color-coding
void FLogManager::DisplayOnScreen(const FString& Message, ELogVerbosity::Type Verbosity, float DisplayTime)
{
    if (GEngine && GEngine->IsInitialized())
    {
        FColor DisplayColor;
        switch (Verbosity)
        {
        case ELogVerbosity::Warning:
            DisplayColor = FColor::Yellow;
            break;
        case ELogVerbosity::Error:
            DisplayColor = FColor::Red;
            break;
        case ELogVerbosity::Fatal:
            DisplayColor = FColor::Magenta;
            break;
        default:
            DisplayColor = FColor::Green;
            break;
        }

        GEngine->AddOnScreenDebugMessage(-1, DisplayTime, DisplayColor, Message);
    }
    else
    {
        return;
    }
}

// Log messages to the console based on verbosity
void FLogManager::LogToConsole(const FString& Message, ELogVerbosity::Type Verbosity)
{
    switch (Verbosity)
    {
    case ELogVerbosity::Warning:
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
        break;
    case ELogVerbosity::Error:
        UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
        break;
    case ELogVerbosity::Fatal:
        UE_LOG(LogTemp, Fatal, TEXT("%s"), *Message);
        break;
    default:
        UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
        break;
    }
}

// Singleton instance of the log manager
FLogManager& FLogManager::Get()
{
    static FLogManager Instance;
    return Instance;
}