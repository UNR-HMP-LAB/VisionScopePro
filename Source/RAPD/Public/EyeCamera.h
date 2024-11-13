// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "EyeCamera.generated.h"

/**
 * 
 */
UCLASS()
class RAPD_API AEyeCamera : public ACameraActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	FVector pointer_origin_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	float bound_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	bool to_limit_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	float gap_C = 20.0f;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int previous_orientation_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int current_orientation_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int count_right_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int total_seen_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	bool no_incorrect_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	TArray<float> michelson_contrast_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	UMaterialInstanceDynamic* current_optotype_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	UMaterialInstanceDynamic* left_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	UMaterialInstanceDynamic* right_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int current_suppression_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int current_metamorphopsia_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	TArray<FString> scotoma_parameters_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	float weight_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	bool recent_right_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Subject Information")
	FString which_eye_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	float previous_log_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	float current_log_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	TArray<FPostProcessSettings> post_process_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Information")
	int total_CS_C;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Stimuli Response")
	TMap<FString, float> mapping_C = { {"NW", 0.375}, {"N", 0.25}, {"NE", 0.125}, {"E", 0.0}, {"SE", 0.875}, {"S", 0.75}, {"SW", 0.625}, {"W", 0.5}};

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MovePointer(FVector direction, float speed, FVector& new_direction);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetOriginBound(FVector origin, float bound);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void TowardsOrigin(FVector location, FVector& new_direction);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MoveAlong(float times, FVector dir);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MonocularLeft();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MonocularRight();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BinocularStatic();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BinocularDynamic();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void CS();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ResizeOptotype(bool& completed);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RandomOrientation(int va);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void AssessResponses(const FString& ro, const FString& test, bool& completed);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ObtainResults(const FString& test, float& result);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyBino(const FString& test, FName& name);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void GetDistance(float& distance);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void LoadMetamorphopsia(bool up, int& current);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitiatePostProcess();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ReadingTest();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaterialToPost();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DifferentSuppression(bool up);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void CSRight();
};
