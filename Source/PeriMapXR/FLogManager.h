// FLogManager.h

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

/**
 *
 */
class PERIMAPXR_API FLogManager
{
public:
	// Constructor to initialize the log file with the current date and time
	FLogManager();
	~FLogManager();

	// Function to log messages
	void LogMessage(const FString& Message, ELogVerbosity::Type Verbosity = ELogVerbosity::Log, float DisplayTime = 5.0f,
		bool bEnableConsoleMessages = true, bool bEnableOnScreenMessages = true, bool bEnableSaveToLog = true);

	// Static function to get the singleton instance
	static FLogManager& Get();

private:
	// The path to the log file
	FString LogFilePath;

	// Function to get the current date and time in a specific format
	FString GetDateTimeString() const;

	// Function to display messages on the screen
	void DisplayOnScreen(const FString& Message, ELogVerbosity::Type Verbosity, float DisplayTime);

	// Function to log messages to the console
	void LogToConsole(const FString& Message, ELogVerbosity::Type Verbosity);
};