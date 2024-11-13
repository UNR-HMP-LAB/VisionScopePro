// Fill out your copyright notice in the Description page of Project Settings.

#include "EyeModules.h"

FString UAssessmentMetrics_C::LogMARtoDecimal(float logMAR) {
	return FString::SanitizeFloat(FMath::Pow(10, -logMAR));
}


FString UAssessmentMetrics_C::LogMARtoSnellenM(float logMAR) {
	FString retSnellenM = "6/";
	retSnellenM.Append(FString::SanitizeFloat(6 / FMath::Pow(10, -logMAR)));
	return retSnellenM;
}

FString UAssessmentMetrics_C::LogMARtoSnellenft(float logMAR) {
	FString retSnellenM = "20/";
	retSnellenM.Append(FString::SanitizeFloat(20 / FMath::Pow(10, -logMAR)));
	return retSnellenM;
}

FString UAssessmentMetrics_C::LogCStoWeber(float logCS) {
	return FString::SanitizeFloat(100 * FMath::Pow(10, -logCS));
}

FString UAssessmentMetrics_C::LogCStoMichelson(float logCS) {
	float weber = FMath::Pow(10, -logCS);
	return FString::SanitizeFloat((100 * weber) / (2 - weber));
}

FString UAssessmentMetrics_C::LogMAR(float logMAR) {
	return FString::SanitizeFloat(logMAR);
}

FString UAssessmentMetrics_C::LogCS(float logCS) {
	return FString::SanitizeFloat(logCS);
}

float UAssessmentMetrics_C::MichelsontoLogCS(float michelson) {
	return -FMath::LogX(10, 2 * michelson / (1 + michelson));
}

float UAssessmentMetrics_C::MARtoLogMAR(float MAR) {
	return FMath::LogX(10, MAR);
}

bool UAssessmentMetrics_C::LoadTextFromFile(FString FileName, TArray<FString>& TextArray, FString& TextString)
{

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileName))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Find File"));
		return false;
	}
	else
	{
		// Convert filepath to character array and save to array
		const TCHAR* FILEPATH = *FileName;
		return FFileHelper::LoadFileToStringArray(TextArray, *FileName);
		//return FFileHelper::LoadFileToString(SaveString, *FileName);
	}
}

bool UAssessmentMetrics_C::SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting = false)
{
	// Set complete file path
	SaveDirectory += "\\";
	SaveDirectory += FileName;

	if (!AllowOverwriting)
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveDirectory))
		{
			return false;
		}
	}

	FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SaveDirectory);
	FString FinalString = "";
	for (FString& Each : SaveText)
	{
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}

	return FFileHelper::SaveStringToFile(FinalString, *SaveDirectory);

}

bool UAssessmentMetrics_C::DeleteTextFile(FString SaveDirectory, FString FileName)
{
	// Set complete file path
	SaveDirectory += "\\";
	SaveDirectory += FileName;

	return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SaveDirectory);
}

