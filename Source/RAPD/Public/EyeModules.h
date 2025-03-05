// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EyeModules.generated.h"


UCLASS()
class UAssessmentMetrics_C : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static FString LogMARtoDecimal(float logMAR);
	UFUNCTION(BlueprintCallable)
	static FString LogMARtoSnellenM(float logMAR);
	UFUNCTION(BlueprintCallable)
	static FString LogMARtoSnellenft(float logMAR);
	UFUNCTION(BlueprintCallable)
	static FString LogCStoWeber(float logCS);
	UFUNCTION(BlueprintCallable)
	static FString LogCStoMichelson(float logCS);
	UFUNCTION(BlueprintCallable)
	static FString LogMAR(float logMAR);
	UFUNCTION(BlueprintCallable)
	static FString LogCS(float logCS);
	UFUNCTION(BlueprintCallable)
	static float MichelsontoLogCS(float michelson);
	UFUNCTION(BlueprintCallable)
	static float MARtoLogMAR(float MAR);
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Load"))
	static bool LoadTextFromFile(FString FileName, TArray<FString>& TextArray, FString& TextString);
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
	static bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting);
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
	static bool DeleteTextFile(FString SaveDirectory, FString FileName);
};

UENUM(BlueprintType)
enum class Eye :uint8
{
	Left     UMETA(DisplayName = "Left"),
	Right      UMETA(DisplayName = "Right"),
	Both   UMETA(DisplayName = "Both"),
};

USTRUCT(BlueprintType)
struct FScotomata_C
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	FLinearColor MeanColor = FLinearColor(0.5f, 0.5f, 0.0f, 0.0f);
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	float Sigma = 0.015f;
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	float Weight = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	float Rotation = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	float Distortion = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	FLinearColor Boundary = FLinearColor::Black;
	UPROPERTY(BlueprintReadWrite, Category = "Scotomata")
	float Tesselation = 0.0f;
};

USTRUCT(BlueprintType)
struct FScotoma_C {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Scotoma")
	TArray<FScotomata_C> layers;

	UPROPERTY(BlueprintReadWrite, Category = "Scotoma")
	TArray<bool> layers_active = { false, false, false };
};

USTRUCT(BlueprintType)
struct FPatient_Assessment_C
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	bool StaticVAValid = false;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	bool DynamicVAValid = false;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	bool CSValid = false;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	bool ScotomaParametersValid = false;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	TMap<FString, float> StaticVA;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	float DynamicVA = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	float CS = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	TArray<FScotoma_C> ScotomaParameters;
	UPROPERTY(BlueprintReadWrite, Category = "Patient_Assessment")
	FDateTime Time;
};

USTRUCT(BlueprintType)
struct FPatient_C {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Patient")
	FString Name = "John Doe";
	UPROPERTY(BlueprintReadWrite, Category = "Patient")
	int32 ID = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Patient")
	int32 Age = 25;
	UPROPERTY(BlueprintReadWrite, Category = "Patient")
	FString Sex = "Male";
	UPROPERTY(BlueprintReadWrite, Category = "Patient")
	FPatient_Assessment_C Assessments;
};


USTRUCT(BlueprintType)
struct FAmsler_Chart_C {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Amsler Grid")
	float GridThickness = 0.02;
	UPROPERTY(BlueprintReadWrite, Category = "Amsler Grid")
	int32 Lines = 5;
	UPROPERTY(BlueprintReadWrite, Category = "Amsler Grid")
	float scale = 4.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Amsler Grid")
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	UPROPERTY(BlueprintReadWrite, Category = "Amsler Grid")
	FLinearColor LineColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
};
