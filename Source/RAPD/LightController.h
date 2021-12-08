// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "SRanipalEye_Core.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "LightController.generated.h"

UCLASS()
class RAPD_API ALightController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALightController();
	UPROPERTY(EditAnywhere, Category = "Materials")
	TArray<UMaterial*> Left_and_right;

	UPROPERTY(BlueprintReadWrite, Category = "Materials")
	TArray<UMaterialInstanceDynamic*> D_left_and_right;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterial* Dark_Material;

	UPROPERTY(EditAnywhere, Category = "Materials")
	FLinearColor color;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	int32 repititions;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool do_calibration;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool after_accommodation;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool disaccommodation;

	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	bool disaccommodation_;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float initial_light_intensity;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float light_duration;
	
	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float dark_duration;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float start_time;
		
	UPROPERTY(EditAnywhere, Category = "Protocol Properties") 
	TArray<float> dropoff_left;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	TArray<float> dropoff_right;

	TArray<float> construct_full_presentation_sequence;
	int32 position_in_sequence = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	TArray<FString> interval_list;
	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	int32 current_interval_position = 0;

	TArray<FString> CSV_file = {"TimeStamp,Intensity_Left,Pupil_Diameter_Left,Intensity_Right,Pupil_Diameter_Right"};
	FString SavingLocation = "E:\\Unreal Projects\\RAPD\\Saved\\Processed_Data";//"C:\\Users\\prith\\Documents\\Unreal Projects\\RAPD\\Saved\\Processed_Data";//

	TArray<float> current_intensity = {0, 0};

	FString tempstring = FDateTime().Now().ToString();
	
	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	bool eye_tracking_ready = false;
	SRanipalEye_Core* eye_core;
	
	UPROPERTY(EditAnywhere, Category = "Subject_ID")
	FString ID;

	UPROPERTY(EditAnywhere, Category = "Session_ID")
	FString Session_ID;

	FTimerHandle LightTimerHandle, DarkTimerHandle, EyeTimerHandle;
	float Elapsed_time = 0.0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Function")
		void IncreaseLuminance(TArray<AStaticMeshActor*> lights);

	UFUNCTION(BlueprintCallable, Category = "Function")
		void Darkness(TArray<AStaticMeshActor*> lights);
	
	UFUNCTION(BlueprintCallable, Category = "Function")
		void TestProtocol(TArray<AStaticMeshActor*> lights);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Load"))
		static bool LoadTextFromFile(FString FileName, TArray<FString>& TextArray);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
		static bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
		static bool DeleteTextFile(FString SaveDirectory, FString FileName);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Start"))
		void Start_calibration();

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Start"))
		void eyeTick();
};
