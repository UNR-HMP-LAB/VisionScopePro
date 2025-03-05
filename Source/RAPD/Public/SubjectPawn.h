// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EyeModules.h"
#include "EyeCamera.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "GameFramework/Pawn.h"
#include "SubjectPawn.generated.h"

UCLASS()
class RAPD_API ASubjectPawn : public APawn
{
	GENERATED_BODY()
private:
	TArray<FString> CSV_file = { "TimeStamp, Eye, Stimuli_Orientation, Response_Orientation, Correct, Threshold" };
	FString SavingLocation = FPaths::ProjectSavedDir();
public:
	// Sets default values for this pawn's properties
	ASubjectPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli")
	AEyeCamera* CalibratedCamera;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Subject")
	int subjectID;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli")
	TArray<FString> direct = { "E", "NE", "N", "NW", "W", "SW", "S", "SE" };

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli")
	FVector direction;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void RecordResponse(float time, FString stimEye, FString stimOr, FString responseOr, bool correct, float param);

	UFUNCTION(BlueprintCallable)
	void WritePatientRecord(FString test, float param, bool wait);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void FlowAssess(bool& done);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SelectedResponse();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SelectedAssessment();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
