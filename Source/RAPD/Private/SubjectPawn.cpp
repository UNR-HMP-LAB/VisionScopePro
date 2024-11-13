#include "SubjectPawn.h"
// Fill out your copyright notice in the Description page of Project Settings.


#include "SubjectPawn.h"

// Sets default values
ASubjectPawn::ASubjectPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASubjectPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASubjectPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASubjectPawn::RecordResponse(float time, FString stimEye, FString stimOr, FString responseOr, bool correct, float param)
{
	FString c = correct ? "true" : "false";
	CSV_file.Add(FString::SanitizeFloat(time) + "," + stimEye + "," + stimOr + ","+responseOr + ","+c + ","+FString::SanitizeFloat(param));
}

void ASubjectPawn::WritePatientRecord(FString test, float param, bool wait)
{
	if (test == "CS Mono") {
		CSV_file.Add("logCS :," + FString::SanitizeFloat(param));
		CSV_file.Add("Michelson :," + UAssessmentMetrics_C::LogCStoMichelson(param));
		CSV_file.Add("Weber :," + UAssessmentMetrics_C::LogCStoWeber(param));
	}
	else {
		CSV_file.Add("logMAR :," + FString::SanitizeFloat(param));
		CSV_file.Add("Decimal :," + UAssessmentMetrics_C::LogMARtoDecimal(param));
		CSV_file.Add("Snellen ft :," + UAssessmentMetrics_C::LogMARtoSnellenft(param));
		CSV_file.Add("Snellen m :," + UAssessmentMetrics_C::LogMARtoSnellenM(param));
	}
	if (!wait) {
		FString savfile = FString::FromInt(subjectID) + "_" + test + "_" + FDateTime::Now().ToString() + ".csv";
		UAssessmentMetrics_C::SaveArrayText(SavingLocation, savfile, CSV_file, true);
		CSV_file.Empty();
	}
}

// Called to bind functionality to input
void ASubjectPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

