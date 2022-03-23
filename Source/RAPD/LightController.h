// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "FoveHMD.h"													//FOVE
#include "FoveVRFunctionLibrary.h"										//FOVE
#include "SRanipalEye_Core.h"											//HTC VIVE
#include "SRanipalEye_Framework.h"
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

	UPROPERTY(BlueprintReadWrite, Category = "Materials")
	FLinearColor color_;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	int32 repititions;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool do_calibration;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool after_accommodation;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool disaccommodation;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool alter;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	bool show_X;

	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	bool show_X_;

	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	UMaterialInstanceDynamic* current_mat;

	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	bool disaccommodation_;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float initial_light_intensity;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float light_duration;
	
	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float intermediate_dark_duration;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	float pause_duration;

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

	TArray<FString> CSV_file = {"TimeStamp,Intensity_Left,Pupil_Diameter_Left,Intensity_Right,Pupil_Diameter_Right,GazeOrigin.x,GazeOrigin.y,GazeOrigin.z,GazeDirection.x,GazeDirection.y,GazeDirection.z"};
	FString SavingLocation = "E:\\Unreal Projects\\RAPD\\Saved\\Processed_Data";//

	TArray<float> current_intensity = {0, 0};

	UPROPERTY(BlueprintReadWrite, Category = "Color Calibration Properties")
	TArray<FLinearColor> luv_from_fove = { FLinearColor(0.524839313, 0.420008266, 0.372208174),FLinearColor(0.512692981, 0.381000841, 0.340125648),FLinearColor(0.555505369, 0.386885991, 0.359811282),FLinearColor(0.54805557, 0.356190608, 0.333232281),FLinearColor(0.495866142, 0.281469298, 0.26982147),FLinearColor(0.604261162, 0.342906559, 0.34684573),FLinearColor(0.540466333, 0.24299642, 0.278767582),FLinearColor(0.54805557, 0.207939083, 0.26982147),FLinearColor(0.623607999, 0.179516747, 0.311373113),FLinearColor(0.469072772, 0.368864729, 0.326152573),FLinearColor(0.384850736, 0.420008266, 0.326152573),FLinearColor(0.459667505, 0.362599068, 0.318871774),FLinearColor(0.487155857, 0.374996087, 0.384100889),FLinearColor(0.504374403, 0.381000841, 0.437599308),FLinearColor(0.482720729, 0.349629931, 0.442502913),FLinearColor(0.473681209, 0.321635576, 0.456814893),FLinearColor(0.491537075, 0.321635576, 0.496669115),FLinearColor(0.445051196, 0.253326654, 0.466047736),FLinearColor(0.449994684, 0.232066388, 0.492442334),FLinearColor(0.562822532, 0.281469298, 0.626080293),FLinearColor(0.495866142, 0.120303472, 0.569637124) };

	bool save_on_pause = false;

	FString tempstring = FDateTime().Now().ToString();
	
	UPROPERTY(BlueprintReadWrite, Category = "Protocol Properties")
	bool eye_tracking_ready = false;

	UPROPERTY(BlueprintReadWrite, Category = "Terminate")
		bool session_complete = false;

	UPROPERTY(EditAnywhere, Category = "Protocol Properties")
	int8 device_id;

	SRanipalEye_Core* eye_core_vive;
	FFoveHMD* eye_core_fove;
	
	UPROPERTY(EditAnywhere, Category = "Subject_ID")
	FString ID;

	UPROPERTY(EditAnywhere, Category = "Session_ID")
	FString Session_ID;

	FTimerHandle LightTimerHandle, DarkTimerHandle, EyeTimerHandle, PauseTimeHandle;
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
		void Pause(TArray<AStaticMeshActor*> lights);
	
	UFUNCTION(BlueprintCallable, Category = "Function")
		void TestProtocol(TArray<AStaticMeshActor*> lights);

	UFUNCTION(BlueprintCallable, Category = "Function")
		void UIProtocol(FString Patient_ID, int32 Protocol_ID, int32 start, int32 end, FString filepath);

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
