// Fill out your copyright notice in the Description page of Project Settings.


#include "LightController.h"

// Sets default values
ALightController::ALightController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ALightController::IncreaseLuminance(TArray<AStaticMeshActor*> lights)
{
	//GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Red, FString::Printf(TEXT("Output: %d"), position_in_sequence));
	if (position_in_sequence >= construct_full_presentation_sequence.Num())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Red, FString::Printf(TEXT("Output: %d"), position_in_sequence));
		GetWorldTimerManager().ClearTimer(LightTimerHandle);
		if(intermediate_dark_duration == 0.0f) Darkness(lights);
		return;
	}

	UMaterialInstanceDynamic* mat;
	float intensity = initial_light_intensity *construct_full_presentation_sequence[position_in_sequence];
	
	if (intensity > 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Light Left"));
		mat = D_left_and_right[0];
		current_intensity = { intensity, 0 };
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, TEXT("Light Right"));
		mat = D_left_and_right[1];
		intensity = -intensity;
		current_intensity = { 0, intensity };
	}

	current_mat = mat;

	mat->SetScalarParameterValue(TEXT("intensity"), intensity);
	mat->SetVectorParameterValue(TEXT("Color"), color_);

	for (int32 i = 0; i < lights.Num(); i++)
	{
		lights[i]->GetStaticMeshComponent()->SetMaterial(0, mat);
	}
	position_in_sequence++;
}

void ALightController::Pause(TArray<AStaticMeshActor*> lights) {
	GetWorldTimerManager().ClearTimer(LightTimerHandle);
	GetWorldTimerManager().ClearTimer(DarkTimerHandle);
	Darkness(lights);
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::White, TEXT("Pause"));
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate;
	LightTimerDelegate.BindUFunction(this, FName("IncreaseLuminance"), lights);
	DarkTimerDelegate.BindUFunction(this, FName("Darkness"), lights);
	if (light_duration > 0.0f) GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + intermediate_dark_duration, true, pause_duration);
	if (intermediate_dark_duration > 0.0f) GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + intermediate_dark_duration, true, pause_duration + light_duration);
}


void ALightController::Darkness(TArray<AStaticMeshActor*> lights)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Dark"));
	UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(Dark_Material, this);
	for (int32 i = 0; i < lights.Num(); i++)
	{
		lights[i]->GetStaticMeshComponent()->SetMaterial(0, mat);
	}
	if (position_in_sequence >= construct_full_presentation_sequence.Num())
	{
		GetWorldTimerManager().ClearTimer(DarkTimerHandle);
		GetWorldTimerManager().ClearTimer(PauseTimeHandle);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("Done"));
		if(dropoff_left[0] == dropoff_right[0])
			SaveArrayText(SavingLocation, ID + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) +"_B_" + FString::SanitizeFloat(dropoff_left[0], 2) + ".csv", CSV_file, true);
		else if(dropoff_left[0] != 1.0f)
			SaveArrayText(SavingLocation, ID + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) +"_L_"+ FString::SanitizeFloat(dropoff_left[0], 2) + ".csv", CSV_file, true);
		else
			SaveArrayText(SavingLocation, ID + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_R_"+FString::SanitizeFloat(dropoff_right[0], 2)+".csv", CSV_file, true);
		return;
	}
	current_intensity = { 0, 0 };
}

void ALightController::Start_calibration() {
	if (device_id == 0) {
		SRanipalEye_Framework::Instance()->StartFramework(SupportedEyeVersion::version2);
		eye_core_vive = SRanipalEye_Core::Instance();
		if (do_calibration) eye_core_vive->LaunchEyeCalibration_(nullptr);
	}
	else if (device_id == 1) {
		eye_core_fove = FFoveHMD::Get();					//FOVE
		EFoveErrorCode error;																					
		TArray<EFoveClientCapabilities> caps;																	
		caps.Add(EFoveClientCapabilities::EyeTracking);															
		caps.Add(EFoveClientCapabilities::PupilRadius);															
		error = eye_core_fove->RegisterCapabilities(caps);															
	}
	eye_tracking_ready = true;

	if (!after_accommodation) {
		FTimerDelegate EyeDelegate;
		EyeDelegate.BindUFunction(this, FName("eyeTick"));
		current_intensity = { 0 , 0 };
		GetWorldTimerManager().SetTimer(EyeTimerHandle, EyeDelegate, .008, true, 0.0f);
	}
}

void ALightController::TestProtocol(TArray<AStaticMeshActor*> lights)
{
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate, PauseTimerDelegate;

	D_left_and_right.Empty();

	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[0], this));
	D_left_and_right[0]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);
	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[1], this));
	D_left_and_right[1]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);

	construct_full_presentation_sequence.Empty();

	for (int32 i = 0; i < dropoff_left.Num(); i++) {
		if (alter && i%2==1) {
			for (int32 j = 0; j < repititions; j++) {
				construct_full_presentation_sequence.Add(dropoff_left[i]);
				construct_full_presentation_sequence.Add(-dropoff_right[i]);
			}
		}
		else{
			for (int32 j = 0; j < repititions; j++) {
				construct_full_presentation_sequence.Add(-dropoff_right[i]);
				construct_full_presentation_sequence.Add(dropoff_left[i]);
			}
		}
	}

	position_in_sequence = 0;

	LightTimerDelegate.BindUFunction(this, FName("IncreaseLuminance"), lights);
	DarkTimerDelegate.BindUFunction(this, FName("Darkness"), lights);
	PauseTimerDelegate.BindUFunction(this, FName("Pause"), lights);

	
	if (after_accommodation) {
		FTimerDelegate EyeDelegate;
		EyeDelegate.BindUFunction(this, FName("eyeTick"));
		current_intensity = { 0 , 0 };
		GetWorldTimerManager().SetTimer(EyeTimerHandle, EyeDelegate, .008, true, 0.0f);
	}
	if(light_duration>0.0f) GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + intermediate_dark_duration, true, start_time);
	if(intermediate_dark_duration >0.0f) GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + intermediate_dark_duration, true, start_time + light_duration);
	if(pause_duration >0.0f) GetWorldTimerManager().SetTimer(PauseTimeHandle, PauseTimerDelegate, (light_duration + intermediate_dark_duration) * repititions *2 - intermediate_dark_duration + pause_duration, true, \
		start_time + (light_duration+intermediate_dark_duration)*repititions * 2 - intermediate_dark_duration - 0.01f);
}

bool ALightController::LoadTextFromFile(FString FileName, TArray<FString>& TextArray)
{

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileName))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Find File"));
		return false;
	}
	else
	{
		// Convert filepath to character array and save to array
		const TCHAR* FILEPATH = *FileName;
		return FFileHelper::LoadFileToStringArray(TextArray, *FileName);
	}
}

bool ALightController::SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting = false)
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

bool ALightController::DeleteTextFile(FString SaveDirectory, FString FileName)
{
	// Set complete file path
	SaveDirectory += "\\";
	SaveDirectory += FileName;

	return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SaveDirectory);
}

// Called when the game starts or when spawned
void ALightController::BeginPlay()
{
	disaccommodation_ = disaccommodation;
	show_X_ = show_X;
	color_ = color;
	LoadTextFromFile(SavingLocation + "\\intervals.csv", interval_list);
	Super::BeginPlay();	
}

void ALightController::eyeTick() {
	Elapsed_time += .008;
	float l = -1.0f, r = -1.0f;

	FString TimeStamp = FString::SanitizeFloat(Elapsed_time), Intensity_Left = FString::SanitizeFloat(current_intensity[0]), \
		Intensity_Right = FString::SanitizeFloat(current_intensity[1]);

	FString Pupil_Diameter_Left = "", Pupil_Diameter_Right = "";
	FString Gaze_Direction = "", Gaze_Origin = "";
	FVector gaze_origin, gaze_direction;

	//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Position in Sequence: %d"), position_in_sequence));

	if (eye_tracking_ready) {
		if (device_id == 0) {
			ViveSR::anipal::Eye::EyeData_v2 data;
			int error = eye_core_vive->GetEyeData_v2(&data);

			Pupil_Diameter_Left = FString::SanitizeFloat(data.verbose_data.left.pupil_diameter_mm);
			Pupil_Diameter_Right = FString::SanitizeFloat(data.verbose_data.right.pupil_diameter_mm);
			gaze_direction = data.verbose_data.combined.eye_data.gaze_direction_normalized;
			gaze_origin = data.verbose_data.combined.eye_data.gaze_origin_mm;
		}
		else if (device_id == 1) {
			FFoveFrameTimestamp out_ftm;
			float left_pupil_radius, right_pupil_radius;
			eye_core_fove->FetchEyeTrackingData(out_ftm);

			eye_core_fove->GetPupilRadius(EFoveEye::Left, left_pupil_radius);
			eye_core_fove->GetPupilRadius(EFoveEye::Right, right_pupil_radius);

			eye_core_fove->GetCombinedGazeRay(gaze_origin, gaze_direction);

			Pupil_Diameter_Left = FString::SanitizeFloat(left_pupil_radius * 2000);
			Pupil_Diameter_Right = FString::SanitizeFloat(right_pupil_radius * 2000);
		}

		Gaze_Origin = FString::SanitizeFloat(gaze_origin.X) + "," + FString::SanitizeFloat(gaze_origin.Y) + "," + FString::SanitizeFloat(gaze_origin.Z);
		Gaze_Direction = FString::SanitizeFloat(gaze_direction.X) + "," + FString::SanitizeFloat(gaze_direction.Y) + "," + FString::SanitizeFloat(gaze_direction.Z);

		//GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::White, FString::Printf(TEXT("Output: %f"), data.verbose_data.left.pupil_diameter_mm));
	}

	CSV_file.Add(TimeStamp + "," + Intensity_Left + "," + Pupil_Diameter_Left + "," + Intensity_Right + "," + Pupil_Diameter_Right + "," +Gaze_Origin+","+Gaze_Direction);
	//SaveArrayText(SavingLocation, ID + "_" + tempstring + ".csv", CSV_file, true);
}


// Called every frame
void ALightController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

