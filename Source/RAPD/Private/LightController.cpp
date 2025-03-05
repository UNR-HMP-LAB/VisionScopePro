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
		if (intermediate_dark_duration == 0.0f) Darkness(lights);
		return;
	}

	UMaterialInstanceDynamic* mat;
	float intensity = initial_light_intensity * construct_full_presentation_sequence[position_in_sequence];

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

	TArray<float> temp_dropoff = { -construct_full_presentation_sequence[position_in_sequence - 2], construct_full_presentation_sequence[position_in_sequence - 1] };
	FString device_name = "pico";
	switch (device_id)
	{
	case VRDeviceType::Pico:
		break;
	case VRDeviceType::Vive:
		device_name = "vive";
		break;
	case VRDeviceType::Fove:
		device_name = "fove";
		break;
	case VRDeviceType::Other:
		device_name = "vr";
		break;
	default:
		break;
	}
	FString savfile = "";
	if (save_on_pause) {
		if (temp_dropoff[0] == temp_dropoff[1]) {
			savfile = ID + "_" + device_name + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_B_" + FString::SanitizeFloat(temp_dropoff[0], 2) + "_Time_" + FDateTime::Now().ToString() + ".csv";
			SaveArrayText(SavingLocation, savfile, CSV_file, true);
		}
		else if (temp_dropoff[1] != 1.0f) {
			savfile = ID + "_" + device_name + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_L_" + FString::SanitizeFloat(temp_dropoff[1], 2) + "_Time_" + FDateTime::Now().ToString() + ".csv";
			SaveArrayText(SavingLocation, savfile, CSV_file, true);
		}
		else {
			savfile = ID + "_" + device_name + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_R_" + FString::SanitizeFloat(temp_dropoff[0], 2) + "_Time_" + FDateTime::Now().ToString() + ".csv";
			SaveArrayText(SavingLocation, savfile, CSV_file, true);
		}
		/*
		if (proto_id != 8) {
			FString prth = FPaths::LaunchDir() + "\\imageFileGeneration";//"C:\\Users\\znasi\\Downloads\\imageFileGeneration";	//"e: && cd \"E:\\Unreal Projects\\Vision Assessments\" && imageFileGeneration.exe 2015_vive_5_On_3.00_Off_0.00_B_1.00_Time_2023.03.14-15.40.05.csv";
			FString prthfile = FPaths::LaunchDir() + " " + savfile;
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, prth);
			FWindowsPlatformProcess::ExecProcess(*prth, *prthfile, 0, 0, 0);
		}
		*/
		CSV_file.Empty();
		CSV_file = { "TimeStamp,Intensity_Left,Pupil_Diameter_Left,Intensity_Right,Pupil_Diameter_Right,GazeOrigin.x,GazeOrigin.y,GazeOrigin.z,GazeDirection.x,GazeDirection.y,GazeDirection.z" };
	}
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::White, TEXT("Pause"));
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate;
	LightTimerDelegate.BindUFunction(this, FName("IncreaseLuminance"), lights);
	DarkTimerDelegate.BindUFunction(this, FName("Darkness"), lights);
	if (light_duration > 0.0f) GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + intermediate_dark_duration, true, pause_duration);
	if (intermediate_dark_duration > 0.0f) GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + intermediate_dark_duration, true, pause_duration + light_duration);
}

void ALightController::DarkAdaptAnywhere(TArray<AStaticMeshActor*> lights) {
	GetWorldTimerManager().ClearTimer(LightTimerHandle);
	GetWorldTimerManager().ClearTimer(DarkTimerHandle);
	GetWorldTimerManager().ClearTimer(PauseTimeHandle);
	GetWorldTimerManager().ClearTimer(DarkAdaptTimerHandle);

	Darkness(lights);
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::White, TEXT("Pause"));
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate, PauseTimerDelegate, DarkAdaptTimerDelegate;
	LightTimerDelegate.BindUFunction(this, FName("IncreaseLuminance"), lights);
	DarkTimerDelegate.BindUFunction(this, FName("Darkness"), lights);
	PauseTimerDelegate.BindUFunction(this, FName("Pause"), lights);
	DarkAdaptTimerDelegate.BindUFunction(this, FName("DarkAdaptAnywhere"), lights);
	float sm = 0.0;
	track_dark_adaptation++;
	if (track_dark_adaptation - 1 < dark_adaptation_starts.Num()) {
		sm = dark_adaptation_durations[track_dark_adaptation - 1];
		if (track_dark_adaptation < dark_adaptation_starts.Num()) {
			GetWorldTimerManager().SetTimer(DarkAdaptTimerHandle, DarkAdaptTimerDelegate, 100.0f, true, dark_adaptation_starts[track_dark_adaptation]);
		}
	}
	if (light_duration > 0.0f) GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + intermediate_dark_duration, true, sm);
	if (intermediate_dark_duration > 0.0f) GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + intermediate_dark_duration, true, sm + light_duration);
	if (pause_duration > 0.0f) GetWorldTimerManager().SetTimer(PauseTimeHandle, PauseTimerDelegate, (light_duration + intermediate_dark_duration) * repititions * 2 - intermediate_dark_duration + pause_duration, true, \
		sm + (light_duration + intermediate_dark_duration) * repititions * 2 - intermediate_dark_duration - 0.01f);

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
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("Done"));
		session_complete = true;

		FString device_name = "pico";
		switch (device_id)
		{
		case VRDeviceType::Pico:
			break;
		case VRDeviceType::Vive:
			device_name = "vive";
			break;
		case VRDeviceType::Fove:
			device_name = "fove";
			break;
		case VRDeviceType::Other:
			device_name = "vr";
			break;
		default:
			break;
		}
		FString savfile = "";
		if (!save_on_pause && dropoff_left[0] == dropoff_right[0]) {
			savfile = ID + "_" + device_name + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_B_" + FString::SanitizeFloat(dropoff_left[0], 2) + "_Time_" + FDateTime::Now().ToString() + ".csv";
			SaveArrayText(SavingLocation, savfile, CSV_file, true);
		}
		else if (!save_on_pause && dropoff_left[0] != 1.0f) {
			savfile = ID + "_" + device_name + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_L_" + FString::SanitizeFloat(dropoff_left[0], 2) + "_Time_" + FDateTime::Now().ToString() + ".csv";
			SaveArrayText(SavingLocation, savfile, CSV_file, true);
		}
		else if (!save_on_pause) {
			savfile = ID + "_" + device_name + "_" + Session_ID + "_On_" + FString::SanitizeFloat(light_duration, 2) + "_Off_" + FString::SanitizeFloat(intermediate_dark_duration, 2) + "_R_" + FString::SanitizeFloat(dropoff_right[0], 2) + "_Time_" + FDateTime::Now().ToString() + ".csv";
			SaveArrayText(SavingLocation, savfile, CSV_file, true);
		}
		/*
		if (proto_id != 8) {
			FString prth = FPaths::LaunchDir() + "\\imageFileGeneration";//"C:\\Users\\znasi\\Downloads\\imageFileGeneration";	//"e: && cd \"E:\\Unreal Projects\\Vision Assessments\" && imageFileGeneration.exe 2015_vive_5_On_3.00_Off_0.00_B_1.00_Time_2023.03.14-15.40.05.csv";
			FString prthfile = FPaths::LaunchDir() + " " + savfile;
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, prth);
			FWindowsPlatformProcess::ExecProcess(*prth, *prthfile, 0, 0, 0);
		}
		*/
		return;
	}
	current_intensity = { 0, 0 };
}

void ALightController::Start_calibration() {
	int calibration_status = 0;
	FString device_name = "pico";
	switch (device_id)
	{
	case VRDeviceType::Pico:
	{
		bool eyesSupported;
		TArray<EPXREyeTrackingMode> temparr;
		picoxr.GetEyeTrackingSupported(eyesSupported, temparr);
		if (!eyesSupported) {
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Eye Tracking Not Supported")));
			return;
		}

		FPXREyeTrackingStartInfo startInfo;
		startInfo.NeedCalibration = do_calibration;
		startInfo.StartMode = EPXREyeTrackingMode::PXR_ETM_BOTH;
		if (!picoxr.StartEyeTracking(startInfo)) {
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Failed to Start Eye Tracking")));
			return;
		}
		eye_tracking_ready = true;
		/*TArray<FString> foveation_file = {"Foveation On, Level"};
		FString fov_lvl = "";
		EPICOXRFoveationLevel lvl;
		if (UPICOXRHMDFunctionLibrary::PXR_GetFoveationLevel(lvl)) {
			fov_lvl = "true, ";
			fov_lvl += UEnum::GetValueAsString(lvl);
		}
		else {
			fov_lvl = "false, ";
			fov_lvl += "unspecified";
		}
		foveation_file.Add(fov_lvl);
		lvl = EPICOXRFoveationLevel::None;
		if (UPICOXRHMDFunctionLibrary::PXR_SetFoveationLevel(lvl)) {
			fov_lvl = "true, ";
			fov_lvl += UEnum::GetValueAsString(lvl);
		}
		else {
			fov_lvl = "false, ";
			fov_lvl += "unspecified";
		}
		foveation_file.Add(fov_lvl);
		if (UPICOXRHMDFunctionLibrary::PXR_GetFoveationLevel(lvl)) {
			fov_lvl = "true, ";
			fov_lvl += UEnum::GetValueAsString(lvl);
		}
		else {
			fov_lvl = "false, ";
			fov_lvl += "unspecified";
		}
		foveation_file.Add(fov_lvl);
		SaveArrayText(SavingLocation, "foveation.csv", foveation_file, true);*/
	}
	break;
	case VRDeviceType::Vive:
	{
	/*SRanipalEye_Framework::Instance()->StartFramework(SupportedEyeVersion::version2);
	eye_core_vive = SRanipalEye_Core::Instance();
	if (do_calibration) {
		calibration_status = eye_core_vive->LaunchEyeCalibration_(nullptr);
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Error: %d"), calibration_status));
	}*/
		eye_tracking_ready = true;
	}
		break;
	case VRDeviceType::Fove:
	{
		/*eye_core_fove = FFoveHMD::Get();					//FOVE
		EFoveErrorCode error;
		TArray<EFoveClientCapabilities> caps;
		caps.Add(EFoveClientCapabilities::EyeTracking);
		caps.Add(EFoveClientCapabilities::PupilRadius);
		error = eye_core_fove->RegisterCapabilities(caps);*/
		eye_tracking_ready = true;
	}
		break;
	case VRDeviceType::Other:
		break;
	default:
		break;
	}
	if (calibration_status != 0) {
		eye_tracking_ready = false;
	}

	if (!after_accommodation) {
		FTimerDelegate EyeDelegate;
		EyeDelegate.BindUFunction(this, FName("eyeTick"));
		current_intensity = { 0 , 0 };
		GetWorldTimerManager().SetTimer(EyeTimerHandle, EyeDelegate, .008, true, 0.0f);
	}
}

void ALightController::TestProtocol(TArray<AStaticMeshActor*> lights)
{
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate, PauseTimerDelegate, DarkAdaptTimerDelegate;

	D_left_and_right.Empty();

	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[0], this));
	D_left_and_right[0]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);
	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[1], this));
	D_left_and_right[1]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);

	construct_full_presentation_sequence.Empty();

	for (int32 i = 0; i < dropoff_left.Num(); i++) {
		if (dropoff_left[i] < 0.0 && dropoff_right[i] < 0.0) {
			continue;
		}
		if (dropoff_left[i] < 0.0) {
			for (int32 j = 0; j < repititions; j++) {
				construct_full_presentation_sequence.Add(-dropoff_left[i]);
				construct_full_presentation_sequence.Add(-dropoff_right[i]);
			}
			continue;
		}
		if (dropoff_right[i] < 0.0) {
			for (int32 j = 0; j < repititions; j++) {
				construct_full_presentation_sequence.Add(dropoff_right[i]);
				construct_full_presentation_sequence.Add(dropoff_left[i]);
			}
			continue;
		}
		if (alter && i % 2 == 1) {
			for (int32 j = 0; j < repititions; j++) {
				construct_full_presentation_sequence.Add(dropoff_left[i]);
				construct_full_presentation_sequence.Add(-dropoff_right[i]);
			}
		}
		else {
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
	DarkAdaptTimerDelegate.BindUFunction(this, FName("DarkAdaptAnywhere"), lights);

	if (after_accommodation) {
		FTimerDelegate EyeDelegate;
		EyeDelegate.BindUFunction(this, FName("eyeTick"));
		current_intensity = { 0 , 0 };
		GetWorldTimerManager().SetTimer(EyeTimerHandle, EyeDelegate, .008, true, 0.0f);
	}
	if (light_duration > 0.0f) GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + intermediate_dark_duration, true, start_time);
	if (intermediate_dark_duration > 0.0f) GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + intermediate_dark_duration, true, start_time + light_duration);
	if (pause_duration > 0.0f) GetWorldTimerManager().SetTimer(PauseTimeHandle, PauseTimerDelegate, (light_duration + intermediate_dark_duration) * repititions * 2 - intermediate_dark_duration + pause_duration, true, \
		start_time + (light_duration + intermediate_dark_duration) * repititions * 2 - intermediate_dark_duration - 0.01f);
	if (dark_adaptation_starts.Num() > 0) {
		GetWorldTimerManager().SetTimer(DarkAdaptTimerHandle, DarkAdaptTimerDelegate, 100.0f, true, dark_adaptation_starts[track_dark_adaptation]);
	}
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
	//FWindowsPlatformProcess::CreateProc(TEXT("python E:\\Unreal Projects\\Vision Assessments\\RAPD\\Source\\RAPD\\abc.py"), nullptr, true, false, false, nullptr, 0, nullptr, nullptr);
	//FString prth = FPaths::LaunchDir()+"\\imageFileGeneration.exe "+FPaths::ProjectSavedDir() + "\\Processed_Data\\2016_vive_5_On_2.00_Off_0.00_B_1.00_Time_2023.03.14-15.42.46.csv";
	//FString prth = "C:\\Users\\znasi\\Downloads\\imageFileGeneration";	//"e: && cd \"E:\\Unreal Projects\\Vision Assessments\" && imageFileGeneration.exe 2015_vive_5_On_3.00_Off_0.00_B_1.00_Time_2023.03.14-15.40.05.csv";
	//FString prthfile = "C:\\Users\\znasi\\Downloads 2015_vive_5_On_3.00_Off_0.00_B_1.00_Time_2023.03.14-15.40.05.csv";
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, prth);
	//FWindowsPlatformProcess::ExecProcess(*prth, *prthfile, 0, 0, 0);
	/*FString prth = FPaths::LaunchDir() + "\\imageFileGeneration";//"C:\\Users\\znasi\\Downloads\\imageFileGeneration";	//"e: && cd \"E:\\Unreal Projects\\Vision Assessments\" && imageFileGeneration.exe 2015_vive_5_On_3.00_Off_0.00_B_1.00_Time_2023.03.14-15.40.05.csv";
	FString tmp = FPaths::LaunchDir();
	FString prthfile = tmp + " " + "2015_vive_5_On_3.00_Off_0.00_B_1.00_Time_2023.03.14-15.40.05.csv";
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, prthfile);
	FWindowsPlatformProcess::ExecProcess(*prth, *prthfile, 0, 0, 0);*/

	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, "python "+ prth + "\\abc.py");// FPaths::LaunchDir());
	//if(execdone) GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("Success")));
	//else GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Failed")));
	Super::BeginPlay();
}

void ALightController::eyeTick() {
	Elapsed_time += .008;
	float l = -1.0f, r = -1.0f;

	FString TimeStamp = FString::SanitizeFloat(Elapsed_time), Intensity_Left = FString::SanitizeFloat(current_intensity[0]), \
		Intensity_Right = FString::SanitizeFloat(current_intensity[1]);

	FString Pupil_Diameter_Left = "", Pupil_Diameter_Right = "";
	FString Gaze_Direction = "", Gaze_Origin = "", Gaze_Status = "";
	FVector gaze_origin, gaze_direction;
	float left_openness = 1.0, right_openness = 1.0;
	bool left_blink = false, right_blink = false;
	//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Position in Sequence: %d"), position_in_sequence));

	if (eye_tracking_ready) {
		switch (device_id)
		{
		case VRDeviceType::Pico:
		{
			/*
			float left_pupil_radius, right_pupil_radius;

			UEyeTrackerFunctionLibrary::GetGazeData(pico);
			gaze_origin = pico.GazeOrigin;
			gaze_direction = pico.GazeDirection;
			left_pupil_radius = pico.LeftPupilDiameter;
			right_pupil_radius = pico.RightPupilDiameter;
			Pupil_Diameter_Left = FString::SanitizeFloat(left_pupil_radius);
			Pupil_Diameter_Right = FString::SanitizeFloat(right_pupil_radius);


			FPXREyeTrackingDataGetInfo picoeyeinfo;
			FPXREyeTrackingData picoeye;
			bool eyesSupported;
			TArray<EPXREyeTrackingMode> temparr;
			picoxr.GetEyeTrackingSupported(eyesSupported, temparr);
			if (eyesSupported) {
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("Supported")));
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Not Supported")));
			}
			eyesSupported = picoxr.GetEyeTrackingData(1.0, picoeyeinfo, picoeye);

			if (eyesSupported) {
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("Active")));
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Not Active")));
			}
			gaze_origin = picoeye.PerEyeDatas[PxrPerEyeUsage::combined].Position;
			//gaze_direction = picoeye.PerEyeDatas[PxrPerEyeUsage::combined].Orientation;

			FPXREyePupilInfo picopupil;
			eyesSupported = picoxr.GetEyePupilInfo(picopupil);

			if (eyesSupported) {
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("Tracking")));
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Not Tracking")));
			}

			left_pupil_radius = picopupil.LeftEyePupilDiameter;
			right_pupil_radius = picopupil.RightEyePupilDiameter;
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::Printf(TEXT("left Pupil : %f Right Pupil: %f"), left_pupil_radius, right_pupil_radius));
			*/

			bool isTracking;
			FPXREyeTrackingState trackingState;
			picoxr.GetEyeTrackingState(isTracking, trackingState);
			if (!isTracking) {
				//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Eye Tracking Not Working")));
				return;
			}

			FPXREyeTrackingDataGetInfo getInfo;
			FPXREyeTrackingData eyeData;
			if (!picoxr.GetEyeTrackingData(1.0, getInfo, eyeData)) {
				//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Failed to Get Eye Tracking Data")));
				return;
			}

			FPXREyePupilInfo pupilInfo;
			if (!picoxr.GetEyePupilInfo(pupilInfo)) {
				//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Failed to Get Pupil Info")));
				return;
			}
			float left_pupil_radius = pupilInfo.LeftEyePupilDiameter + 1.0f;
			float right_pupil_radius = pupilInfo.RightEyePupilDiameter + 1.0f;
			gaze_origin = eyeData.PerEyeDatas[PxrPerEyeUsage::combined].Position;
			gaze_direction = eyeData.PerEyeDatas[PxrPerEyeUsage::combined].Orientation.RotateVector(FVector::UpVector);
			Pupil_Diameter_Left = FString::SanitizeFloat(left_pupil_radius);
			Pupil_Diameter_Right = FString::SanitizeFloat(right_pupil_radius);
			picoxr.GetEyeOpenness(left_openness, right_openness);
			int64 temp;
			picoxr.GetEyeBlink(temp, left_blink, right_blink);
		}
			break;
		case VRDeviceType::Vive:
		{
			/*ViveSR::anipal::Eye::EyeData_v2 data;
			int error = eye_core_vive->GetEyeData_v2(&data);

			Pupil_Diameter_Left = FString::SanitizeFloat(data.verbose_data.left.pupil_diameter_mm);
			Pupil_Diameter_Right = FString::SanitizeFloat(data.verbose_data.right.pupil_diameter_mm);
			gaze_direction = data.verbose_data.combined.eye_data.gaze_direction_normalized;
			gaze_origin = data.verbose_data.combined.eye_data.gaze_origin_mm;*/
		}
			break;
		case VRDeviceType::Fove:
		{
			/*FFoveFrameTimestamp out_ftm;
			float left_pupil_radius, right_pupil_radius;
			eye_core_fove->FetchEyeTrackingData(out_ftm);

			eye_core_fove->GetPupilRadius(EFoveEye::Left, left_pupil_radius);
			eye_core_fove->GetPupilRadius(EFoveEye::Right, right_pupil_radius);

			eye_core_fove->GetCombinedGazeRay(gaze_origin, gaze_direction);

			Pupil_Diameter_Left = FString::SanitizeFloat(left_pupil_radius * 2000);
			Pupil_Diameter_Right = FString::SanitizeFloat(right_pupil_radius * 2000);*/
		}
			break;
		case VRDeviceType::Other:
			break;
		default:
			break;
		}
		Gaze_Origin = FString::SanitizeFloat(gaze_origin.X) + "," + FString::SanitizeFloat(gaze_origin.Y) + "," + FString::SanitizeFloat(gaze_origin.Z);
		Gaze_Direction = FString::SanitizeFloat(gaze_direction.X) + "," + FString::SanitizeFloat(gaze_direction.Y) + "," + FString::SanitizeFloat(gaze_direction.Z);
		Gaze_Status = FString::SanitizeFloat(left_openness) + "," + FString::SanitizeFloat(right_openness) + "," + (left_blink ? "Yes" : "No") +"," + (right_blink ? "Yes" : "No");
		//GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::White, FString::Printf(TEXT("Output: %f"), data.verbose_data.left.pupil_diameter_mm));
	}

	CSV_file.Add(TimeStamp + "," + Intensity_Left + "," + Pupil_Diameter_Left + "," + Intensity_Right + "," + Pupil_Diameter_Right + "," + Gaze_Origin + "," + Gaze_Direction+","+Gaze_Status);
	//SaveArrayText(SavingLocation, ID + "_" + tempstring + ".csv", CSV_file, true);
}

// Called every frame
void ALightController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALightController::UIProtocol(FString Patient_ID, int32 Protocol_ID, int32 start, int32 end) {
	// When both dropoffs are negative, it signifies a darkness period of that amount to both eyes. 
	// When only one eye is negative, that means that eye is presented the stimuli first
	ID = Patient_ID;
	do_calibration = true;
	save_on_pause = false;
	proto_id = Protocol_ID;
	if (Protocol_ID == 1) {
		repititions = 3;
		Session_ID = "1";
		intermediate_dark_duration = 0.0f;
		pause_duration = 0.0f;
		light_duration = 3.0f;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		{
			dropoff_left = { 1, 1, 1, -5, -1, -0.5, -0.25 };
			dropoff_right = { 1, 0.5, 0.25, -5, 1, 1, 1 };
		}break;
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, 1, 1, -5, -1, -0.3, -0.16 };
			dropoff_right = { 1, 0.3, 0.16, -5, 1, 1, 1 };
			
		}
			break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
			break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1};
			dropoff_right = { 1, 1};
			break;
		}
	}
	else if (Protocol_ID == 2) {
		repititions = 3;
		Session_ID = "2";
		intermediate_dark_duration = 0.0f;
		pause_duration = 0.0f; // Intermediate Illumination Dark (Both Eyes) Duration
		light_duration = 2.0f;	// Illumination Duration in Each Eye
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		{
			// -5 = 5 second dark duration mid-test
			// -1 = Illumination starts from the particular eye
			dropoff_left = { 1, 1, 1, -5, -1, -0.5, -0.25 };  
			dropoff_right = { 1, 0.5, 0.25, -5, 1, 1, 1 };
		}break;
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, 1, 1, -5, -1, -0.3, -0.16 };
			dropoff_right = { 1, 0.3, 0.16, -5, 1, 1, 1 };
			
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	else if (Protocol_ID == 3) {
		repititions = 3;
		Session_ID = "3";
		intermediate_dark_duration = 0.0f;
		pause_duration = 5.0f;
		light_duration = 3.0f;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		{
			dropoff_left = { 1, 1, 1, -1, -0.5, -0.25 };
			dropoff_right = { 1, 0.5, 0.25, 1, 1, 1 };
		}break;
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, 1, 1, -1, -0.3, -0.16 };
			dropoff_right = { 1, 0.3, 0.16, 1, 1, 1 };
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	else if (Protocol_ID == 4) {
		repititions = 3;
		Session_ID = "4";
		intermediate_dark_duration = 0.0f;
		pause_duration = 5.0f;
		light_duration = 2.0f;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		{
			dropoff_left = { 1, 1, 1, -1, -0.5, -0.25 };
			dropoff_right = { 1, 0.5, 0.25, 1, 1, 1 };
		}
		break;
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, 1, 1, -1, -0.3, -0.16 };
			dropoff_right = { 1, 0.3, 0.16, 1, 1, 1 };
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	else if (Protocol_ID == 5) {
		repititions = 3;
		Session_ID = "5";
		pause_duration = 5.0f;
		light_duration = 3.0f;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, -1, 1, -1 };
			dropoff_right = { 1, 1, 1, 1 };
			//dropoff_left = { 1, 1, 1, 0.3, 0.16 };
			//dropoff_right = { 1, 0.3, 0.16, 1, 1 };
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	else if (Protocol_ID == 6) {
		repititions = 3;
		Session_ID = "6";
		light_duration = 3;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, 1, 1, 0.3, 0.16 };
			dropoff_right = { 1, 0.3, 0.16, 1, 1 };
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	else if (Protocol_ID == 7) {
		repititions = 3;
		Session_ID = "7";
		light_duration = 2;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		case VRDeviceType::Vive:
		{
			dropoff_left = { 1, 1, 1, 0.3, 0.16 };
			dropoff_right = { 1, 0.3, 0.16, 1, 1 };
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	else if (Protocol_ID == 8) {
		repititions = 4;
		save_on_pause = true;
		pause_duration = 6.0f;
		Session_ID = "8";
		light_duration = 2;
		dropoff_left.Empty();
		dropoff_right.Empty();

		TArray<float> temp_dropoff_left, temp_dropoff_right;
		switch (device_id)
		{
		case VRDeviceType::Pico:
		case VRDeviceType::Vive:
		{
			temp_dropoff_left = { 1, 0.82, 0.64, 0.5, 0.39, 0.3, 0.23, 0.18, 0.14, 0.11, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			temp_dropoff_right = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.82, 0.64, 0.5, 0.39, 0.3, 0.23, 0.18, 0.14, 0.11 };
		}
		break;
		case VRDeviceType::Fove:
		{
			temp_dropoff_left = { 1, 0.82, 0.64, 0.5, 0.39, 0.3, 0.23, 0.18, 0.14, 0.11, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			temp_dropoff_right = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.82, 0.64, 0.5, 0.39, 0.3, 0.23, 0.18, 0.14, 0.11 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
		for (int32 i = start - 1; i < end; i++) {
			dropoff_left.Add(temp_dropoff_left[i]);
			dropoff_right.Add(temp_dropoff_right[i]);
		}
	}
	else if (Protocol_ID == 9) {
		repititions = 1;
		Session_ID = "9";
		light_duration = 5;
		dropoff_left.Empty();
		dropoff_right.Empty();
		switch (device_id)
		{
		case VRDeviceType::Pico:
		case VRDeviceType::Vive:
		{
			dropoff_left = { 0.1, 0.16, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 };
			dropoff_right = { 0.1, 0.16, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 };
		}
		break;
		case VRDeviceType::Fove:
		{
			dropoff_left = { 5, 5, 5, 0.39, 0.2 };
			dropoff_right = { 5, 0.39, 0.2, 5, 5 };
		}
		break;
		case VRDeviceType::Other:
		default:
			dropoff_left = { 1, 1 };
			dropoff_right = { 1, 1 };
			break;
		}
	}
	dark_adaptation_starts.Empty();
	dark_adaptation_durations.Empty();
	int non = 0;
	float curtm = start_time - 0.02f;
	for (int32 i = 0; i < dropoff_left.Num(); i++) {
		if (dropoff_left[i] < 0.0 && dropoff_right[i] < 0.0) {
			dark_adaptation_starts.Add(non * repititions * 2 * (light_duration + intermediate_dark_duration) + pause_duration * non - intermediate_dark_duration + curtm);
			curtm += -dropoff_left[i];
			dark_adaptation_durations.Add(-dropoff_left[i]);
			non = 0;
		}
		else {
			non += 1;
		}
	}
}

FLinearColor ALightController::convert_hex() {
	return FLinearColor(FColor::FromHex(hexColor));
}

void ALightController::Initialize_colors_for_test() {
	//TArray<FString> protan_hex = { "#94656d", "#986971", "#81525a", "#794a52", "#85565e", "#81525a", "#95666e", "#8d5e65", "#976870", "#83545c", "#976870", "#76474f", "#784951", "#94646c", "#8e5f67", "#976870", "#85565e", "#895a62", "#7e4f57", "#805159", "#75454d", "#7b4c54", "#7d4e56", "#73434c", "#906169", "#8c5d65", "#906168", "#8d5e66", "#7e4f57", "#885860", "#97686f", "#996a72", "#815159", "#976870", "#93646c", "#784951", "#704149", "#83545c", "#96676e", "#986970", "#8a5b62", "#85565e", "#976870", "#784951", "#7f5058", "#95666e", "#704149", "#97686f", "#996a71", "#7e4f57", "#85565e", "#704149", "#8b5c64", "#976870", "#73444c", "#8b5b63", "#8c5d65", "#9a6b72", "#8c5d64", "#885961", "#76474f", "#81525a", "#96676f", "#96676f", "#7e4f57", "#72434b", "#784951", "#986970", "#71414a", "#794a52", "#82535b", "#7c4d55", "#97686f", "#704049", "#8b5c64", "#74454d", "#7a4a52", "#91626a", "#7a4b53", "#8a5b63", "#75464e", "#794a52", "#93646c", "#895a62", "#8a5b63", "#8b5c64", "#7a4b53", "#74454d", "#7d4e56", "#72424b", "#75464e", "#8b5c64", "#94656d", "#75464e", "#9a6b73", "#87585f", "#6f4048", "#8f6068", "#704149", "#83545c", "#8f4a56", "#914c58", "#7e3946", "#a8636e", "#87424e", "#87424e", "#924d59", "#8e4955", "#7c3744", "#833e4a", "#96515d", "#a6616d", "#803b48", "#98535f", "#803b48", "#87424e", "#94505c", "#aa6570", "#995561", "#87424f", "#aa6570", "#a6616d", "#823d4a", "#9b5662", "#7f3a47", "#a05b67", "#9d5864", "#a15c67", "#803b48", "#97525e", "#823d4a", "#a4606b", "#9e5965", "#95505c", "#9c5763", "#86414e", "#823d4a", "#a15c68", "#8f4a57", "#98535f", "#96515d", "#944f5b", "#98535f", "#944f5b", "#a6616d", "#944f5b", "#a05b67", "#995460", "#8d4855", "#8b4652", "#904b57", "#a05b66", "#8c4754", "#a96470", "#a05b67", "#9b5662", "#97535f", "#9d5864", "#8a4552", "#a05b67", "#a05b67", "#9a5561", "#813c49", "#7d3845", "#9e5a65", "#a05b67", "#904b57", "#a96470", "#96525e", "#9b5662", "#98535f", "#7d3845", "#a8636e", "#9d5964", "#94505c", "#a5606c", "#7c3643", "#843f4b", "#a6616c", "#823d49", "#9c5863", "#7c3744", "#87424e", "#a5606c", "#86414e", "#a05b67", "#894451", "#87424e", "#8a4652", "#a8636f", "#a8636e", "#894451", "#843f4b", "#914c58", "#843f4c", "#a35e6a", "#8d4955", "#904b57", "#813c48", "#9a5561", "#b45b6b", "#a94f5f", "#a24859", "#8c3144", "#a54b5c", "#b35969", "#882d40", "#903648", "#a84e5f", "#8c3244", "#963c4d", "#b65c6c", "#af5565", "#b75d6d", "#a24959", "#b55b6b", "#993f51", "#b65c6c", "#892f41", "#b55b6b", "#8b3043", "#b35969", "#a54b5c", "#993f50", "#a54b5c", "#923849", "#af5565", "#b55b6b", "#983f50", "#a74d5d", "#ab5161", "#963c4e", "#9f4656", "#9e4555", "#b05666", "#9a4152", "#8e3346", "#882e40", "#93394a", "#892f41", "#913749", "#a94f5f", "#93394b", "#ac5363", "#8a3042", "#923849", "#903648", "#b35969", "#a54b5c", "#af5566", "#b35969", "#983e4f", "#93394b", "#af5565", "#913749", "#92384a", "#963d4e", "#a54c5c", "#93394b", "#b75d6d", "#ad5364", "#b05767", "#ad5464", "#9e4455", "#8b3143", "#a64d5d", "#b65c6c", "#b65c6c", "#ab5161", "#993f51", "#9f4556", "#9d4354", "#ab5262", "#983e4f", "#a74e5e", "#a54c5c", "#ae5464", "#b75d6d", "#9e4455", "#a14758", "#a04657", "#af5565", "#af5566", "#9d4354", "#a74e5e", "#ae5465", "#b85e6e", "#9f4656", "#a94f5f", "#8e3446", "#993f50", "#903648", "#953b4c", "#b15767", "#963c4e", "#903648", "#a54b5c", "#913749", "#8e3446", "#a84e5f", "#b7495f", "#a83a51", "#ad3f55", "#95253e", "#c05267", "#b14359", "#bd4e64", "#9b2b44", "#a6374e", "#af4157", "#b04258", "#ba4c61", "#962740", "#aa3c52", "#b84a5f", "#b5475d", "#ac3e54", "#9f3048", "#a3354c", "#a3344b", "#b14359", "#b4465c", "#93233c", "#c15267", "#be5065", "#992942", "#a1324a", "#962740", "#a6374e", "#9c2d45", "#a13249", "#b94b60", "#a1334a", "#94243d", "#c25368", "#94243d", "#c25368", "#af4157", "#b7495e", "#af4157", "#a4354d", "#a6374e", "#9f3048", "#ba4b61", "#bc4e63", "#992a42", "#a93b51", "#b14359", "#b24359", "#a93b51", "#9b2b44", "#bf5065", "#c4556a", "#bc4e63", "#be5065", "#9f3048", "#a2344b", "#a2334b", "#9a2b43", "#bd4f64", "#b94a60", "#b6485e", "#b84a5f", "#ad3e55", "#a1324a", "#a5364d", "#bf5166", "#a2334a", "#9a2a43", "#9f3048", "#9b2c44", "#9d2e46", "#af4057", "#be5065", "#b94a60", "#af4157", "#a83950", "#b8495f", "#9a2b44", "#962740", "#9b2c44", "#a6384f", "#a6374e", "#b04258", "#bf5166", "#ae4056", "#a3344b", "#a6384f", "#a3344c", "#a5374e", "#aa3c52", "#a7384f", "#be5065", "#9a2b43", "#a2334b", "#a6384f", "#b5465c", "#95253e", "#9d2e46", "#b3455b", "#b5324f", "#bf3c58", "#b83552", "#b73350", "#b4314e", "#ab2746", "#c94661", "#be3b57", "#a52041", "#a11b3d", "#c5425d", "#bc3955", "#c84560", "#bc3955", "#b63250", "#b02d4b", "#a92444", "#b4304e", "#a82343", "#c3415c", "#c3415c", "#a31d3e", "#ce4b66", "#a11a3d", "#b93653", "#bf3c58", "#af2b49", "#b02c4a", "#aa2545", "#c7445f", "#a41e3f", "#aa2645", "#a21b3d", "#ab2746", "#cb4863", "#c84561", "#b93653", "#aa2545", "#c84560", "#aa2545", "#ac2747", "#b32f4d", "#a11a3c", "#b73351", "#a72242", "#a0193c", "#bd3a56", "#b83552", "#b5324f", "#c3405c", "#c84560", "#ab2746", "#cd4a65", "#c3415c", "#b93653", "#a82343", "#b22e4c", "#a11a3c", "#a11b3d", "#b63350", "#b4314e", "#c3405c", "#c3405c", "#c13f5a", "#a31d3f", "#cd4a65", "#ce4b66", "#a31c3e", "#ae2a48", "#cf4c66", "#bc3956", "#a21b3d", "#a31d3e", "#ac2746", "#c5425e", "#b63350", "#cd4b65", "#b63250", "#b02c4a", "#c7445f", "#be3b57", "#c84661", "#b5324f", "#b32f4d", "#a92444", "#aa2545", "#b02c4b", "#c3405c", "#b73350", "#ca4862", "#c5425e", "#be3b57", "#bc3955", "#c13e5a", "#b83552", "#a72142", "#c03d59", "#a31c3e", "#a21b3d", "#ca4762", "#c3294f", "#ca3155", "#ce3659", "#b91d46", "#b61943", "#c0264c", "#bb2048", "#c42b50", "#be244b", "#b81b45", "#bb1f47", "#d94263", "#da4264", "#c42a50", "#b1123f", "#c83054", "#d23b5d", "#ac0a3a", "#d33b5e", "#ca3256", "#d1395b", "#c0264c", "#bf254b", "#bb1f47", "#b51842", "#a90538", "#bc2149", "#d33b5d", "#b51742", "#a70135", "#d53d5f", "#d74061", "#cc3457", "#c72e53", "#ad0b3b", "#b71b44", "#cb3357", "#bf244b", "#c72f53", "#ba1e47", "#d53d5f", "#c0264c", "#b81c45", "#d43c5e", "#b0113e", "#ad0b3b", "#ca3155", "#bc2149", "#ac0a3a", "#d53e5f", "#a70235", "#c72e52", "#a80337", "#d63f60", "#b81c45", "#ba1f47", "#d1395c", "#ae0d3c", "#d94162", "#c2294f", "#c1284e", "#bd2249", "#d63e60", "#a80336", "#d33b5d", "#b1123f", "#a70135", "#ba1e46", "#b61a44", "#ba1e47", "#b1133f", "#b81c45", "#ca3255", "#cb3356", "#ae0e3c", "#ae0d3c", "#b2133f", "#d53e5f", "#b0103d", "#a90437", "#d94162", "#b31541", "#ba1f47", "#bc2249", "#b21340", "#cb3357", "#d0385a", "#b41641", "#a80437", "#aa0538", "#cd3558", "#ae0d3c", "#ba1f47", "#cf375a", "#cb3356", "#c1274d", "#d0385b", "#b1113e", "#c83054", "#d94163", "#d42452", "#bf033f", "#c40b43", "#c71146", "#d01e4e", "#e43660", "#e0335d", "#c10742", "#cd1a4b", "#cb184a", "#d72855", "#dd2f5a", "#d01f4f", "#ca1649", "#b50036", "#b10033", "#d82956", "#c61046", "#ba003b", "#b30035", "#b9003a", "#ca1749", "#be023e", "#e3355f", "#b60037", "#c20842", "#c81448", "#c61046", "#ca1649", "#bc003d", "#d72754", "#b30034", "#d62654", "#c40d44", "#ba003b", "#d72855", "#be023f", "#cf1e4e", "#b10032", "#d72855", "#b60037", "#b50036", "#e2355f", "#e1335e", "#c91549", "#d72855", "#b9003a", "#b60037", "#b60037", "#b10032", "#ce1b4c", "#c60f45", "#b50037", "#c10641", "#b40036", "#e1335d", "#dd2f5a", "#df315c", "#cd1a4c", "#da2b57", "#b10032", "#ba003b", "#c71146", "#bf023f", "#bc003d", "#c10641", "#b30035", "#bc003d", "#d01f4f", "#b9003a", "#cb174a", "#b20033", "#d1204f", "#dd2e5a", "#bf0440", "#b70038", "#c10741", "#b00032", "#dc2d59", "#db2d59", "#c10741", "#cb174a", "#af0031", "#c10741", "#e2355f", "#ba003b", "#d92a56", "#ba003b", "#d32251", "#be023e", "#b70038", "#e3355f", "#c71247", "#d72855", "#de305b", "#b20034", "#be013e", "#ca1649", "#bd013e", "#c30a43", "#cf0042", "#c6003a", "#be0033", "#c6003a", "#e01551", "#e82158", "#c30037", "#ce0041", "#cb003e", "#ce0041", "#ea245a", "#ea245a", "#bb0030", "#c9003d", "#bf0033", "#b8002d", "#d60348", "#b7002c", "#c40038", "#ec265b", "#eb255b", "#bc0031", "#cb003e", "#c40038", "#c7003b", "#bc0031", "#e71f56", "#c60039", "#c7003b", "#df1450", "#ea2459", "#ec265b", "#e01551", "#d70349", "#db0c4d", "#c8003c", "#ca003d", "#cf0042", "#d9074a", "#c7003a", "#cb003e", "#b7002c", "#c30037", "#e31953", "#d8064a", "#c30037", "#c9003d", "#bb0030", "#c7003b", "#de114f", "#df1350", "#c30037", "#cb003f", "#c8003b", "#c20036", "#df1450", "#ea245a", "#d10044", "#dd104e", "#e11752", "#df1350", "#b9002e", "#cf0042", "#d00042", "#bb002f", "#ed275c", "#de124f", "#c40038", "#e31a54", "#ca003d", "#ca003d", "#ec265b", "#bc0031", "#cc003f", "#db0c4c", "#e51d55", "#d40046", "#ce0041", "#e41b54", "#e01451", "#db0d4d", "#d40046", "#c10035", "#b9002e", "#dc0f4e", "#d50147", "#e71f56", "#d40046", "#e82057", "#db0b4c", "#b9002e", "#b9002e", "#c8003b", "#cb003e", "#df1350", "#e82158", "#c9003d", "#c10036", "#ec275b", "#cb003e", "#ed0151", "#d20039", "#c00028", "#e5004a", "#cf0036", "#ec0050", "#c5002d", "#cf0036", "#bf0027", "#cb0033", "#de0044", "#cf0036", "#ce0035", "#d5003b", "#c4002c", "#de0044", "#dc0042", "#d9003f", "#d00037", "#cd0034", "#f20954", "#d10038", "#ec004f", "#c80030", "#cf0036", "#cd0035", "#ee0251", "#c2002a", "#e50049", "#d00037", "#ed0251", "#e20047", "#f10754", "#e40049", "#bf0028", "#f10754", "#d8003e", "#d3003a", "#d3003a", "#c90031", "#be0027", "#e00045", "#d8003e", "#d4003b", "#d4003a", "#f20b55", "#d10038", "#e30048", "#e40048", "#f51158", "#e10047", "#c3002b", "#d3003a", "#e9004e", "#c10029", "#e6004b", "#f61359", "#cc0033", "#e6004a", "#d00037", "#e20047", "#ea004e", "#e9004e", "#df0044", "#d00037", "#e20047", "#e8004d", "#d00037", "#c3002c", "#d6003d", "#ca0032", "#f20b55", "#e40049", "#ef0452", "#cf0037", "#e7004b", "#c6002e", "#d10038", "#d00037", "#e6004b", "#d5003c", "#c80030", "#cf0037", "#c4002c", "#d30039", "#bf0027", "#f00553", "#f10954", "#dd0043", "#d00038", "#cf0036", "#e9004d", "#f41057", "#d6003d", "#ec004f", "#ee0251", "#cf0036", "#cb0033", "#e20047", "#f00653", "#f6004d", "#e0003a", "#fa0051", "#fa0050", "#e6003f", "#cc0028", "#fb0051", "#f5004c", "#db0035", "#ca0026", "#ee0046", "#cf002b", "#e4003d", "#e80041", "#e3003c", "#cf002b", "#e90042", "#fe0054", "#c90025", "#e3003d", "#e70040", "#ef0047", "#d3002e", "#d90034", "#cc0028", "#f3004b", "#cf002a", "#d0002b", "#d3002e", "#c60022", "#d0002c", "#cf002b", "#dc0036", "#de0038", "#dc0037", "#fd0053", "#f4004b", "#f3004a", "#fb0052", "#d50030", "#dc0036", "#cd0029", "#df0039", "#e2003c", "#c60022", "#e1003b", "#fc0053", "#fc0052", "#ee0046", "#d1002d", "#c90025", "#e5003e", "#df0039", "#ee0046", "#e80041", "#f4004c", "#c60022", "#c90025", "#ed0045", "#c90024", "#ef0047", "#f10049", "#fd0053", "#f00048", "#f00048", "#df0039", "#d2002e", "#ca0026", "#df0039", "#ef0047", "#e90042", "#cc0027", "#fd0053", "#f4004b", "#dc0037", "#e80041", "#d4002f", "#ea0042", "#d70032", "#ed0045", "#f8004f", "#cf002b", "#cc0027", "#c70023", "#f90050", "#ef0047", "#db0035", "#f10049", "#fe0054", "#fc0052", "#d80033", "#e80041", "#e0003a", "#f3004b", "#c90025", "#d70032", "#fe0054", "#d1002d", "#cb0027", "#ec0045" };
	//TArray<FString> deutan_hex = { "#74425f", "#713f5d", "#7c4a67", "#855470", "#6e3c5a", "#82506d", "#794764", "#6c3957", "#956480", "#82516d", "#855471", "#814f6c", "#814f6c", "#7e4c69", "#956480", "#7b4967", "#764462", "#865571", "#8f5e7a", "#855370", "#804f6c", "#8c5a77", "#865471", "#956480", "#784663", "#81506c", "#956380", "#81506c", "#895874", "#83526e", "#814f6c", "#875672", "#6d3b59", "#865571", "#713e5c", "#92617d", "#6e3c5a", "#6c3a58", "#8e5d79", "#8f5e7a", "#93617d", "#7d4b68", "#7a4866", "#6f3c5a", "#744260", "#7d4b68", "#83526e", "#7a4865", "#6d3a58", "#7e4c69", "#6e3b59", "#93617e", "#8f5e7a", "#6e3b59", "#855470", "#74415f", "#72405e", "#7d4b68", "#915f7c", "#6b3957", "#764461", "#74425f", "#8e5d79", "#865471", "#84526f", "#7e4c69", "#7e4c69", "#7f4d6a", "#6d3a58", "#723f5d", "#855470", "#84526f", "#7b4966", "#82506d", "#8d5c78", "#885673", "#6c3957", "#774563", "#94627e", "#94637f", "#784663", "#82516d", "#895774", "#8e5d79", "#774562", "#804e6b", "#895774", "#73415f", "#703e5c", "#875572", "#784663", "#82516e", "#713f5d", "#7e4d69", "#885773", "#91607c", "#905f7b", "#92617d", "#885673", "#885773", "#813862", "#8c446d", "#934c74", "#89416a", "#843b65", "#964e76", "#89416a", "#7c325d", "#954e76", "#7d345e", "#974f77", "#8d456e", "#7b315c", "#833a64", "#853d66", "#954d75", "#a05980", "#8f4770", "#9d557d", "#99527a", "#894069", "#995179", "#9d567e", "#a35c83", "#7c335d", "#7b315c", "#904871", "#9f587f", "#8c446d", "#7a305b", "#792f5a", "#8d456d", "#974f77", "#8c446d", "#873e68", "#803761", "#a35c83", "#813962", "#9b547c", "#934b74", "#894069", "#7b325d", "#7a305b", "#934b73", "#843c65", "#843b65", "#8c446d", "#9e577e", "#8b436c", "#a05980", "#914971", "#8d456e", "#823963", "#8d456e", "#873f68", "#9b547b", "#8c446d", "#8e466f", "#954e76", "#884069", "#9e577f", "#833a64", "#9e577e", "#a25b83", "#9b547c", "#964f77", "#8d456e", "#924a73", "#914972", "#a25b82", "#884069", "#974f77", "#863d67", "#a35c83", "#853c66", "#914971", "#884069", "#a15a82", "#934b73", "#833b64", "#954d75", "#975078", "#8a416a", "#944d75", "#853c66", "#9a527a", "#9e577e", "#803761", "#a05980", "#863e67", "#79305b", "#833b64", "#9b547c", "#7b325d", "#843b65", "#944d75", "#9e577f", "#9a527a", "#a25b82", "#944d75", "#87255f", "#a7497e", "#a6497d", "#882660", "#87255f", "#a24479", "#93336b", "#8b2a63", "#9a3b71", "#ad5083", "#a94c80", "#a04277", "#b05386", "#86245e", "#85225d", "#93336a", "#a24479", "#a6487d", "#993b71", "#903068", "#9a3b71", "#85225d", "#a24479", "#a24479", "#b05386", "#9b3c72", "#96376e", "#8d2c65", "#892761", "#9b3d72", "#ad4f83", "#96376d", "#8c2b64", "#8e2d65", "#85225d", "#913169", "#8c2b64", "#af5285", "#93336b", "#ac4f83", "#8c2b64", "#8e2d65", "#ac4f83", "#85225d", "#9f4176", "#9b3d73", "#8d2d65", "#8c2b64", "#af5286", "#9b3c72", "#a04277", "#a24479", "#a6487c", "#8c2b64", "#9c3e73", "#af5285", "#a24479", "#8a2862", "#a6487d", "#96376d", "#9e3f75", "#a84a7f", "#a7497d", "#86245f", "#8f2e66", "#8d2d65", "#8a2862", "#913168", "#94356c", "#9e4076", "#8e2d66", "#85225d", "#a5477c", "#96366d", "#a6487d", "#94356c", "#993a70", "#892761", "#85225d", "#ae5084", "#993a70", "#9d3e74", "#92326a", "#87255f", "#a94c80", "#8a2862", "#913168", "#ad5083", "#9a3b71", "#993a70", "#882660", "#94356c", "#a7497e", "#8a2962", "#87255f", "#a04177", "#902f67", "#a5477c", "#96366d", "#8a2962", "#911161", "#a0276f", "#a62f75", "#bc498a", "#a62f75", "#b03b7f", "#a62f75", "#b74385", "#a83277", "#af3a7d", "#ae397c", "#ad387c", "#921262", "#ab3579", "#a83277", "#a12970", "#b03b7e", "#a22970", "#a42d73", "#9a1e69", "#921262", "#911061", "#b94586", "#911061", "#b74284", "#b54183", "#a0276f", "#931463", "#a62f75", "#951764", "#a32b72", "#b13c7f", "#a12970", "#b84486", "#aa3479", "#b23d80", "#921262", "#bd498a", "#b74284", "#9c226c", "#9c226b", "#a93377", "#a42c73", "#981d68", "#931463", "#ab3579", "#a42d73", "#910f60", "#961966", "#ae397d", "#a52e74", "#bd498a", "#bd498a", "#ae387c", "#951764", "#af3a7d", "#931363", "#9b206a", "#921161", "#af397d", "#b64284", "#ad377b", "#931563", "#9d246d", "#a62e74", "#9d236c", "#a12870", "#921262", "#9d236c", "#ba4687", "#bb4788", "#9e256d", "#ad387c", "#b44082", "#951865", "#921262", "#9d236c", "#991d68", "#a62f75", "#b54083", "#ad387c", "#b74284", "#9a1f69", "#b94586", "#9f266e", "#a0276f", "#9e246d", "#bb4789", "#981b67", "#941664", "#991d68", "#951865", "#991d68", "#ae397d", "#b54183", "#941563", "#9c226c", "#ac377b", "#bd498a", "#9c216b", "#9e0066", "#ba2a80", "#b01d77", "#b8287e", "#aa1171", "#9d0065", "#ba2b80", "#c83c8d", "#b01c76", "#9e0066", "#b4237b", "#ad1774", "#aa1071", "#c03386", "#a80e70", "#b11d77", "#9f0067", "#b5237b", "#a10069", "#9d0065", "#ae1975", "#bc2d81", "#ab1472", "#aa1171", "#bf3184", "#ac1573", "#ac1573", "#a5076d", "#a70b6e", "#9d0065", "#a80d6f", "#aa1271", "#c93d8e", "#ac1473", "#a90f70", "#b7267d", "#b4227a", "#c23587", "#a5076d", "#9f0067", "#9f0067", "#c03285", "#a2016a", "#b5237b", "#a6096e", "#af1a76", "#ac1573", "#a3026a", "#c13386", "#b7267d", "#a80e6f", "#b9297f", "#bf3184", "#aa1171", "#9f0067", "#9d0065", "#c73b8c", "#b01c77", "#a2016a", "#c23588", "#c63a8b", "#ad1874", "#bf3185", "#a10069", "#ad1674", "#b92a7f", "#af1a76", "#af1b76", "#a2016a", "#a91070", "#bf3184", "#ae1975", "#bc2d82", "#a2016a", "#9d0065", "#a2016a", "#c6398b", "#bf3285", "#a70c6f", "#c33789", "#be3084", "#b5237b", "#b4227a", "#a20169", "#be2f83", "#c73b8c", "#c63a8b", "#b9297f", "#aa1171", "#ae1975", "#a00068", "#bc2d82", "#a00068", "#bd2e83", "#c93d8d", "#bb2c81", "#9e0066", "#c93d8e", "#b22079", "#b11d77", "#b70076", "#a9006a", "#b30073", "#ce258c", "#c00b7f", "#b50075", "#b30073", "#ca1e88", "#ca1f88", "#ae006e", "#ce258c", "#b60075", "#d42d91", "#cd248b", "#b20072", "#c51583", "#ab006b", "#be067d", "#bf077e", "#bd037c", "#aa006b", "#c00b7f", "#af0070", "#b40074", "#b20072", "#c81b86", "#c51583", "#ac006d", "#ca1e88", "#ad006d", "#d42e91", "#c10d80", "#c00a7f", "#c31081", "#aa006b", "#ac006d", "#ca1f88", "#d22a8f", "#bf077e", "#bc017b", "#c00b7f", "#d1298e", "#ca1f88", "#c91d87", "#ad006d", "#c71985", "#bb007a", "#c81b86", "#d1298e", "#b90078", "#b30073", "#ae006e", "#cf268d", "#d0288e", "#d1298e", "#cd228a", "#d32c90", "#a70068", "#be057d", "#d22a8f", "#ac006d", "#a90069", "#d22a8f", "#b80078", "#cb2089", "#be067d", "#d32c90", "#cd238a", "#ce258c", "#cb2089", "#ad006d", "#ae006e", "#ce258b", "#bd037c", "#ae006e", "#ca1f88", "#cd238a", "#b20072", "#cf278d", "#b00070", "#c61784", "#c41382", "#c51583", "#c81b86", "#c81b86", "#b20072", "#b10071", "#b60076", "#c21081", "#b70076", "#d32c90", "#cc2189", "#b10071", "#b80077", "#be067d", "#c81a86", "#b60076", "#c61784", "#a80069", "#b50074", "#c8007f", "#cb0082", "#de1494", "#cc0084", "#b2006c", "#ca0082", "#b2006c", "#c4007d", "#c4007c", "#d9078f", "#bb0074", "#dd1192", "#d10088", "#dc0f91", "#c90081", "#d5008c", "#b70071", "#d8058e", "#ba0073", "#c2007b", "#c2007a", "#ba0073", "#c00078", "#ba0073", "#bf0078", "#b5006f", "#bf0077", "#d6018c", "#bb0074", "#c10079", "#b1006b", "#da0c90", "#b6006f", "#da0c90", "#c5007d", "#b4006e", "#d7028d", "#da0b90", "#c5007d", "#c3007b", "#d5008c", "#d5008b", "#d8058e", "#df1694", "#b3006d", "#ce0085", "#c7007f", "#ca0081", "#b6006f", "#c4007c", "#ce0086", "#d3008a", "#bb0074", "#cf0086", "#d4008b", "#b90072", "#cb0082", "#e01795", "#c4007c", "#b90072", "#ce0085", "#dc1092", "#c2007a", "#b60070", "#d8058e", "#d5008c", "#b3006d", "#dc0f91", "#b60070", "#ba0073", "#cf0086", "#c5007d", "#d5008c", "#bf0078", "#d4008b", "#cf0086", "#c7007f", "#c3007b", "#bc0075", "#bb0074", "#ba0073", "#c5007d", "#cd0084", "#dc0f91", "#c6007e", "#b3006d", "#d00087", "#b2006c", "#d8058e", "#b80072", "#b60070", "#c6007e", "#c7007f", "#c2007a", "#c3007b", "#cf0086", "#dd1393", "#bc0075", "#de1594", "#d4008b", "#e50094", "#c9007b", "#c00073", "#c10074", "#d90089", "#cc007e", "#c10074", "#d90089", "#c30076", "#ca007c", "#d50085", "#cb007d", "#c00073", "#cb007d", "#e00090", "#e00090", "#e70096", "#c30075", "#d60087", "#e80096", "#e70095", "#cd007f", "#e90097", "#d30084", "#ca007c", "#e30092", "#c20075", "#dd008c", "#ea0098", "#e30092", "#d20083", "#cc007d", "#d60087", "#cf0081", "#c30076", "#bf0072", "#d50086", "#c8007a", "#d30084", "#ce007f", "#be0071", "#e80096", "#d20083", "#ce007f", "#e50093", "#c00073", "#c10074", "#e70096", "#c9007b", "#da008a", "#bc006f", "#c10074", "#c8007a", "#ce007f", "#e70096", "#ea0098", "#d20083", "#cd007f", "#e20091", "#cc007d", "#d10082", "#cf0080", "#c50078", "#cb007d", "#c00073", "#e40093", "#e70096", "#d10082", "#bf0072", "#be0071", "#c10074", "#d70087", "#c70079", "#e20091", "#bc006f", "#c30076", "#c70079", "#d40084", "#c10074", "#dc008b", "#c10074", "#cc007e", "#cc007e", "#c9007b", "#d50086", "#be0071", "#e90098", "#d30084", "#c60078", "#c8007a", "#c70079", "#e80097", "#e70096", "#ea0098", "#d00081", "#e80096", "#df008e", "#d50085", "#d50086", "#d40084", "#cd0079", "#dc0086", "#da0085", "#d3007f", "#d4007f", "#d1007d", "#d50080", "#e1008b", "#dc0087", "#ce007a", "#d1007c", "#ca0076", "#c70074", "#e70090", "#c60073", "#f00098", "#c80075", "#f00098", "#ed0095", "#d2007d", "#ee0096", "#ef0097", "#df0089", "#dd0087", "#f10099", "#c80075", "#eb0093", "#e2008c", "#db0085", "#d80082", "#d60081", "#e70090", "#da0085", "#ef0097", "#f00098", "#cb0078", "#e80091", "#e5008e", "#e5008e", "#d3007e", "#cd0079", "#f20099", "#ca0076", "#de0088", "#c90075", "#c80075", "#f4009c", "#e2008b", "#cb0078", "#eb0094", "#d0007b", "#e1008b", "#d3007f", "#e3008c", "#e6008f", "#c70074", "#ec0095", "#c80075", "#df0089", "#cd0079", "#ee0096", "#f10099", "#df0089", "#e5008e", "#e5008e", "#c80075", "#ec0094", "#cf007b", "#d60081", "#de0088", "#ea0092", "#f20099", "#c70074", "#ef0097", "#f2009a", "#df0089", "#ea0093", "#cb0077", "#e90091", "#c70074", "#e6008f", "#f3009b", "#eb0093", "#e0008a", "#e70090", "#f4009c", "#c90075", "#e70090", "#eb0093", "#e80091", "#e2008b", "#e3008c", "#de0088", "#e6008f", "#c60073", "#d50080", "#f20099", "#ee0096", "#db0085", "#f3009b", "#d20079", "#d5007b", "#e40088", "#e8008c", "#d6007c", "#d5007b", "#fb009c", "#e7008a", "#d30079", "#e10086", "#fc009d", "#f70098", "#dc0081", "#d6007c", "#da007f", "#f40096", "#ee0090", "#f60098", "#eb008e", "#ee0091", "#fb009c", "#ef0091", "#e6008a", "#de0083", "#fc009d", "#d8007e", "#e10085", "#ec008f", "#e20086", "#ea008d", "#ea008d", "#e30087", "#f10094", "#de0083", "#d20079", "#ef0091", "#da007f", "#d6007c", "#f9009a", "#ee0090", "#e50089", "#e50089", "#f10094", "#d9007e", "#d6007c", "#e30087", "#d8007e", "#fe009f", "#e50089", "#d00077", "#d00077", "#e00085", "#e40088", "#eb008e", "#f10093", "#d7007d", "#df0084", "#e7008b", "#ed0090", "#e30087", "#eb008e", "#fb009c", "#d20078", "#de0083", "#d20079", "#d10077", "#f50097", "#f10094", "#e8008b", "#f70099", "#f10093", "#f50097", "#f30095", "#ea008e", "#fd009e", "#ef0092", "#d9007f", "#d10077", "#ea008d", "#f10094", "#d7007d", "#fc009d", "#d30079", "#f60098", "#d5007b", "#e8008c", "#ef0092", "#ec008f", "#f10094", "#d30079", "#e6008a", "#d6007c", "#e6008a", "#fe009e", "#d7007d", "#d10077", "#da007f", "#e8008c", "#ef0091", "#f50097" };
	//TArray<FString> tritan_hex = { "#5a5072", "#403558", "#463c5e", "#463b5e", "#504668", "#483e60", "#605677", "#544a6c", "#443a5c", "#605677", "#4d4365", "#453a5c", "#584e6f", "#53496b", "#615879", "#5a5072", "#5e5576", "#605677", "#453a5c", "#544a6c", "#63597a", "#3f3557", "#443a5c", "#5a5072", "#5e5475", "#5c5274", "#595071", "#594f70", "#5b5172", "#544a6c", "#413759", "#4c4264", "#5a5172", "#53496a", "#564c6e", "#544a6b", "#403658", "#584e6f", "#473d5f", "#473d5f", "#43385b", "#5b5173", "#5e5576", "#3f3557", "#63597a", "#53496b", "#5b5273", "#5f5577", "#5b5173", "#615879", "#483e60", "#4c4264", "#4f4567", "#625879", "#463c5e", "#504667", "#5d5475", "#473d5f", "#483e60", "#615879", "#42385a", "#504667", "#554b6c", "#53496b", "#5d5475", "#493e60", "#564d6e", "#423759", "#554b6c", "#53496a", "#453b5d", "#42375a", "#5a5072", "#43385b", "#504667", "#5d5374", "#554b6d", "#5f5677", "#5c5273", "#483e60", "#453a5c", "#493f61", "#4d4364", "#413759", "#504667", "#544b6c", "#463b5d", "#584e6f", "#4c4264", "#5c5274", "#443a5c", "#554b6c", "#514769", "#453b5d", "#534a6b", "#5b5172", "#4f4567", "#493f61", "#403658", "#5a5072", "#5f5081", "#4c3d6f", "#4b3c6e", "#615283", "#514173", "#5d4e7f", "#433366", "#534475", "#605182", "#635485", "#5a4b7c", "#49396b", "#5d4e7f", "#4a3a6c", "#5f5081", "#453568", "#4f3f71", "#4e3e70", "#625384", "#5a4b7c", "#615384", "#5e4f80", "#534375", "#4f4072", "#433265", "#574879", "#4b3b6d", "#4f3f71", "#524274", "#5b4c7d", "#605182", "#5d4e7f", "#453567", "#5b4c7e", "#433366", "#5d4e7f", "#4d3d6f", "#574879", "#5d4e7f", "#453568", "#635586", "#5a4b7c", "#514273", "#453568", "#544577", "#5d4e7f", "#574879", "#4e3e70", "#4e3f70", "#49396b", "#615383", "#605283", "#443366", "#615283", "#48386a", "#473769", "#544577", "#5e4f80", "#5e5081", "#5c4d7e", "#605182", "#5d4e7f", "#534476", "#5e4f80", "#5c4d7e", "#443366", "#5b4c7d", "#534375", "#5d4e7f", "#473769", "#615383", "#504172", "#514274", "#504172", "#504072", "#4e3e70", "#443467", "#625484", "#423265", "#504072", "#605182", "#655687", "#594a7b", "#48386a", "#5b4c7d", "#443467", "#453567", "#645586", "#615283", "#423265", "#554577", "#48386a", "#554677", "#544576", "#625384", "#554577", "#594a7b", "#423265", "#574879", "#635485", "#493276", "#5c4788", "#4d377a", "#675393", "#5e4a8b", "#634f8f", "#644f90", "#4b3478", "#5b4687", "#543e80", "#4c3679", "#472f73", "#614d8d", "#4c3578", "#4e387b", "#5e498a", "#462e73", "#462e73", "#614c8d", "#5f4a8b", "#634e8f", "#462e73", "#574283", "#4a3376", "#452d72", "#483074", "#452d72", "#624d8e", "#493276", "#503a7d", "#4e377a", "#604b8c", "#584384", "#5b4687", "#554082", "#473074", "#4a3377", "#665293", "#4c3679", "#574183", "#594485", "#493175", "#4a3477", "#513b7e", "#462e73", "#574284", "#5c4788", "#5f4a8b", "#4b3478", "#4e377a", "#5a4586", "#4c3679", "#624e8f", "#675393", "#4f397c", "#5b4688", "#4f397c", "#594385", "#584384", "#655192", "#594485", "#4d377a", "#655091", "#5f4a8b", "#4e387b", "#5b4687", "#543e80", "#594486", "#5a4587", "#513b7e", "#553f81", "#554082", "#5f4b8c", "#5f4a8b", "#5b4687", "#614c8d", "#5c4789", "#543f81", "#675394", "#564183", "#483075", "#5c4788", "#483074", "#624e8f", "#624e8f", "#584385", "#4e377a", "#624e8f", "#564082", "#645090", "#493276", "#4b3478", "#503a7d", "#5a4586", "#5e498a", "#523d7f", "#4d377a", "#554082", "#4b3477", "#5e498a", "#64499b", "#5d4193", "#5e4294", "#604496", "#664b9d", "#5f4395", "#583b8e", "#56398d", "#674d9e", "#684d9e", "#492881", "#654a9b", "#4d2e84", "#513388", "#593d90", "#5e4294", "#664b9c", "#4e2f85", "#53358a", "#5b3e91", "#674c9d", "#573a8d", "#56398d", "#654a9c", "#624798", "#5e4294", "#664b9c", "#55388c", "#4c2c83", "#64499b", "#573a8e", "#5a3e91", "#624798", "#5f4395", "#4a2a81", "#54378b", "#56398d", "#694fa0", "#4c2c83", "#664b9c", "#64499b", "#55388c", "#56388c", "#503287", "#4b2c83", "#5f4496", "#63489a", "#56398d", "#482780", "#56398d", "#624798", "#64499b", "#503287", "#583b8f", "#674d9e", "#5c4093", "#4b2b82", "#5f4395", "#523589", "#55378b", "#573a8e", "#503187", "#492981", "#482780", "#5f4395", "#4f3086", "#56388c", "#513388", "#5c4093", "#4c2c83", "#513288", "#4c2d83", "#664b9c", "#573a8e", "#64489a", "#4d2e84", "#664b9c", "#624799", "#503287", "#503287", "#55388c", "#55388c", "#4e2f85", "#523489", "#664b9d", "#4d2e85", "#57398d", "#614597", "#634899", "#4c2d84", "#4d2e85", "#4e2f85", "#4d2d84", "#5d4193", "#513288", "#54368b", "#634899", "#604597", "#4d2e84", "#573a8e", "#552e97", "#542b95", "#6c4aac", "#5e399f", "#4d218f", "#5d389e", "#5f3ba0", "#6a48ab", "#552e97", "#6845a8", "#562f98", "#4c208f", "#6947aa", "#542c96", "#5a349b", "#5f3a9f", "#6745a8", "#552e97", "#5e399f", "#522893", "#5b369c", "#4f2491", "#6c4aad", "#5b359c", "#5f3aa0", "#623ea2", "#6542a6", "#512893", "#4f2491", "#502692", "#4f2591", "#562f97", "#532a94", "#502692", "#59329a", "#6947aa", "#6340a4", "#4f2592", "#613da1", "#573199", "#5f3ba0", "#6947aa", "#6b49ab", "#532a95", "#59339a", "#552d96", "#5f3aa0", "#613da2", "#603ba0", "#6643a7", "#4f2491", "#583299", "#5d389e", "#562f97", "#6744a7", "#59339b", "#6a47aa", "#4e2390", "#5b369c", "#6340a4", "#6a47aa", "#512893", "#512793", "#6542a6", "#613da2", "#6542a6", "#5c369c", "#613da2", "#583199", "#603ca1", "#6b48ab", "#6542a5", "#5c379d", "#6a47aa", "#5c379d", "#512793", "#6d4bae", "#542c96", "#4f2592", "#4d2190", "#6946a9", "#522893", "#633fa3", "#512893", "#5f3a9f", "#562f98", "#5e3a9f", "#502692", "#603ca1", "#532b95", "#6542a5", "#623ea3", "#603ca1", "#5d389e", "#583199", "#512793", "#6542a6", "#5f3ba0", "#6440a4", "#6a48aa", "#5319a1", "#5722a4", "#6233ad", "#6d43b8", "#6639b1", "#541aa1", "#6f44b9", "#6b40b6", "#541ba1", "#5b28a7", "#5620a3", "#5925a5", "#6437b0", "#6437af", "#7046bb", "#6f45ba", "#5924a5", "#6538b0", "#541ba2", "#5926a6", "#5b29a7", "#5a28a7", "#561fa3", "#541ca2", "#673bb2", "#5d2ca9", "#5319a1", "#7046bb", "#531aa1", "#6d42b8", "#6031ac", "#551da2", "#6c41b7", "#5d2ca9", "#5d2ca9", "#5f2eaa", "#51149f", "#5318a1", "#51149f", "#5722a4", "#5e2eaa", "#6538b0", "#5b29a7", "#5318a1", "#5f2faa", "#683cb3", "#6538b0", "#673ab2", "#5b29a7", "#5e2da9", "#5926a6", "#551ea2", "#6b3fb6", "#6132ad", "#673ab2", "#6335ae", "#6437b0", "#6538b0", "#6030ab", "#5e2daa", "#6638b1", "#683cb3", "#5f2fab", "#5319a1", "#6334ae", "#6537b0", "#5b28a7", "#541ba2", "#5216a0", "#5e2da9", "#6c41b7", "#5318a0", "#6335ae", "#6132ad", "#5722a4", "#673bb2", "#7046bb", "#5b28a7", "#5215a0", "#5721a4", "#6a3eb5", "#5823a5", "#551ea2", "#5721a3", "#50129f", "#683bb3", "#6a3fb5", "#5722a4", "#6436af", "#5f2faa", "#683cb3", "#7045ba", "#5f2fab", "#5c2aa8", "#5d2ba9", "#5721a4", "#6437af", "#673ab2", "#531aa1", "#5319a1", "#6e37c3", "#6529bc", "#6a31c0", "#662abc", "#6122b8", "#6b33c1", "#5804b2", "#713cc6", "#6529bb", "#733ec8", "#5b12b4", "#6d36c2", "#703bc6", "#733fc8", "#692fbe", "#6c35c2", "#6d36c3", "#6324b9", "#723dc7", "#662abc", "#6c34c1", "#5f1db7", "#6f39c4", "#6121b8", "#6528bb", "#601fb7", "#672cbd", "#6b33c1", "#5a0eb3", "#5f1db7", "#6325ba", "#5e19b6", "#6120b8", "#5806b2", "#703ac5", "#6e37c3", "#5c13b4", "#5f1cb7", "#6122b9", "#6121b8", "#682ebe", "#5805b2", "#733fc8", "#6f38c4", "#723dc7", "#5909b3", "#5a0db3", "#672dbd", "#6c35c2", "#6326ba", "#5a0db3", "#5700b2", "#5d17b5", "#682fbe", "#703bc6", "#5907b2", "#733ec8", "#5909b3", "#5e1bb6", "#713cc6", "#6223b9", "#5600b1", "#662bbd", "#6426ba", "#5600b1", "#5e19b6", "#5a0eb3", "#5e1bb6", "#672dbd", "#662bbc", "#6427ba", "#601eb7", "#6f39c4", "#6324b9", "#6324b9", "#6a32c0", "#6e38c4", "#601fb8", "#6325ba", "#5600b1", "#6e38c4", "#5a0cb3", "#6c35c2", "#652abc", "#5c14b4", "#703bc5", "#5b12b4", "#5c14b4", "#5d16b5", "#713cc7", "#5c13b4", "#6020b8", "#662abc", "#5d18b6", "#6f3ac5", "#5c13b4", "#5803b2", "#6325ba", "#6e38c3", "#5600b1", "#6302c7", "#5d00c6", "#660fc9", "#6e25cf", "#5d00c6", "#6404c8", "#7330d3", "#7431d4", "#6d23ce", "#6e26cf", "#6a1ccc", "#6816ca", "#6f28d0", "#5d00c6", "#6610c9", "#6200c7", "#5d00c6", "#6715ca", "#6000c6", "#6816ca", "#702ad1", "#712cd2", "#7737d7", "#7431d4", "#5e00c6", "#650dc9", "#660fc9", "#6200c7", "#712dd2", "#5d00c6", "#5f00c6", "#6200c7", "#6817cb", "#6f29d0", "#6301c7", "#5d00c6", "#7432d5", "#7635d6", "#6d23ce", "#6b1fcd", "#7431d4", "#6a1ccc", "#6e27cf", "#7431d4", "#6e26cf", "#6301c7", "#722dd2", "#660ec9", "#7635d6", "#5f00c6", "#5d00c6", "#722ed3", "#5d00c6", "#6711c9", "#6b20cd", "#691acb", "#6000c6", "#7533d5", "#6816ca", "#6b20cd", "#7534d5", "#6d24ce", "#6919cb", "#6406c8", "#5d00c6", "#7635d6", "#5f00c6", "#702ad1", "#7533d5", "#650ac8", "#6711c9", "#660fc9", "#6303c8", "#5d00c6", "#6b1fcd", "#7533d5", "#6303c8", "#6508c8", "#6d25cf", "#7736d6", "#6e26cf", "#6100c6", "#6b1ecc", "#6301c7", "#5e00c6", "#5f00c6", "#691bcb", "#6712ca", "#7534d5", "#691acb", "#6404c8", "#650dc9", "#691bcc", "#712cd1", "#6b1fcd", "#5f00c6", "#6509c8", "#6714ca", "#7736d7", "#722ed2", "#6e00dd", "#7824e4", "#6e00dd", "#720fdf", "#6700dc", "#7925e4", "#7007de", "#6f02dd", "#6b00dc", "#7722e3", "#7211df", "#7212df", "#6500df", "#6800db", "#7b2be6", "#6a00db", "#7722e3", "#6a00db", "#6c00dc", "#7417e0", "#6600dd", "#761fe2", "#6f02dd", "#6800db", "#6500de", "#7008de", "#7a28e5", "#6800db", "#6b00dc", "#6900db", "#6b00dc", "#751ce1", "#7417e0", "#7a29e6", "#7a29e5", "#6f03de", "#6700dc", "#6600dd", "#751ce1", "#7823e3", "#6f01dd", "#6700dc", "#6500de", "#761ee2", "#7210df", "#6b00dc", "#6500de", "#6600dd", "#761fe2", "#6d00dc", "#6b00dc", "#6600dc", "#6900db", "#6a00db", "#7b2ae6", "#7722e3", "#710bde", "#710bde", "#6d00dc", "#6800db", "#6700dc", "#6500de", "#7721e3", "#7926e4", "#6500df", "#7b2ae6", "#6500df", "#7720e2", "#710ddf", "#6f03dd", "#6700dc", "#7211df", "#6800db", "#6c00dc", "#6b00dc", "#6500de", "#6700dc", "#6f01dd", "#6c00dc", "#6700dc", "#710bde", "#6c00dc", "#7825e4", "#6c00dc", "#6800db", "#7823e3", "#6700dc", "#751ae1", "#7210df", "#751ae1", "#6d00dd", "#6500dd", "#7721e3", "#6f02dd", "#7721e3", "#710cdf", "#7006de", "#7926e4", "#7925e4", "#7006de", "#7600f1", "#7900f2", "#7900f2", "#7e11f6", "#7000f5", "#7000f4", "#7800f2", "#7000f4", "#7d0df5", "#7300f1", "#6f00f5", "#7500f1", "#7600f1", "#7100f1", "#7900f2", "#7400f1", "#7300f1", "#7000fe", "#7400f1", "#8018f7", "#7e11f6", "#7c08f4", "#6f00f7", "#7e12f6", "#7700f2", "#7000f4", "#6f00f7", "#7d0bf5", "#7f16f6", "#7800f2", "#7000f5", "#7300f1", "#7000f5", "#7400f1", "#7300f1", "#7c07f4", "#7900f2", "#7d09f5", "#7500f1", "#7900f2", "#7000f2", "#7200f1", "#7800f2", "#7600f1", "#7500f1", "#7400f1", "#7200f1", "#7a00f3", "#7900f3", "#7600f1", "#801af7", "#7000f4", "#7900f3", "#7200f1", "#6f00fb", "#7a00f3", "#7400f1", "#7100f2", "#6f00f6", "#6f00fb", "#7e13f6", "#7800f2", "#7900f2", "#7000f3", "#7100f2", "#7100f2", "#7600f1", "#6f00fa", "#7100f2", "#7d0bf5", "#8019f7", "#7500f1", "#7b02f4", "#7000f4", "#7500f1", "#7c08f4", "#8017f7", "#7200f1", "#7d0ef5", "#6f00f8", "#7900f2", "#8019f7", "#7300f1", "#6f00f8", "#7500f1", "#7d09f5", "#7d0cf5", "#7c07f4", "#7100f2", "#7b02f4", "#7c04f4", "#7100f1", "#7a00f3", "#6f00fa", "#7000f4", "#7d0bf5", "#7000f2", "#7800f2", "#7800f2", "#7300f1" };

	protan_hex = { "#b4989c", "#c0a3a7", "#d1b1b6", "#c2a5a9", "#bb9fa3", "#c9abaf", "#b79b9f", "#bda0a5", "#b99da1", "#dbacb3", "#d1a4aa", "#c1979d", "#c3999f", "#d6a8af", "#be949b", "#dbacb3", "#ca9ea5", "#d4a6ad", "#d99ea7", "#c59098", "#e3a5af", "#d49ba3", "#dda1aa", "#c38e96", "#e7a8b2", "#ce969f", "#c8929a", "#e69da8", "#d8939e", "#d8939e", "#f0a3af", "#e49ba6", "#f1a4b0", "#d9949e", "#d9949e", "#df98a2", "#d48693", "#e994a2", "#e08e9b", "#f49caa", "#d88995", "#d48793", "#d58794", "#ed97a5", "#f69dab", "#e88a99", "#e68998", "#fe98a9", "#fc96a7", "#e88a9a", "#fe98a9", "#fe98a9", "#e48897", "#f995a5", "#fe8ea1", "#f6899c", "#fe8ea1", "#fb8c9f", "#ec8395", "#f4889a", "#fc8d9f", "#fe8ea1", "#fe8ea1", "#fe859a", "#fe859a", "#f47f94", "#ec7b8f", "#fe859a", "#f17e92", "#fe859a", "#f27f93", "#fe859a", "#fe7d94", "#fe7d94", "#fe7d94", "#fd7c93", "#fa7a91", "#fe7d94", "#f5788e", "#fe7d94", "#f2768c", "#fe748e", "#fe748e", "#fe748e", "#fe748e", "#fe748e", "#fe748e", "#fd738d", "#fe748e", "#fe748e" };
	deutan_hex = { "#bc9eae", "#ba9cad", "#bb9dad", "#be9fb0", "#c5a5b6", "#d0aec0", "#bfa0b1", "#c7a7b8", "#bc9dae", "#c79bb2", "#c99cb3", "#cfa1b9", "#daaac3", "#c89bb2", "#c599af", "#d6a7bf", "#b98fa5", "#ba90a6", "#d197b4", "#d398b6", "#e4a5c4", "#dd9fbe", "#dc9fbe", "#e5a5c5", "#d79bba", "#dda0bf", "#e3a4c3", "#ed9fc6", "#e69ac0", "#d891b5", "#e499bf", "#cf8bad", "#d790b4", "#cf8bad", "#ec9ec5", "#eea0c7", "#f99ccb", "#f89bca", "#d484ac", "#d484ad", "#d786af", "#f398c6", "#f197c4", "#f298c5", "#de8ab5", "#f08cbf", "#e987b9", "#f48ec2", "#f68fc4", "#db7fae", "#e484b5", "#fe94ca", "#df81b1", "#f890c5", "#fe8ac6", "#fe8ac6", "#fe89c5", "#fe8ac6", "#e279af", "#fe8ac6", "#e67cb2", "#fe8ac6", "#fe8ac6", "#f97cbd", "#f87cbd", "#fe7fc2", "#fe7fc2", "#fe7fc2", "#f97cbd", "#fe7fc2", "#fe7fc2", "#f87cbd", "#fe75be", "#fc74bc", "#fe75be", "#fe75be", "#fe75be", "#fe75be", "#fe75be", "#f06eb3", "#f16fb4", "#fe6bbb", "#fe6bbb", "#fe6bbb", "#fe6bbb", "#fa69b8", "#fe6bbb", "#fe6bbb", "#fe6bbb", "#fe6bbb" };
	tritan_hex = { "#a59fb7", "#b2abc5", "#aca5be", "#b8b0cb", "#a19bb2", "#a19bb2", "#b7b0ca", "#b7b0ca", "#aba4bc", "#a99fc2", "#b0a6cb", "#a49bbd", "#b1a7cc", "#b9aed5", "#a49abd", "#aca2c6", "#b5abd1", "#aca2c6", "#ab9ecd", "#beb0e4", "#a497c4", "#aea1d1", "#b6a9da", "#b5a7d9", "#b6a8da", "#aea1d0", "#b2a4d4", "#beaceb", "#c1aeee", "#b3a2dd", "#bfadec", "#a494cc", "#ac9cd5", "#ae9ed8", "#a797ce", "#b5a4e0", "#b09ce1", "#bda7f1", "#b19de3", "#b6a1e8", "#b29ee4", "#ae9adf", "#a491d3", "#bea8f3", "#ab97db", "#a992df", "#bda4fa", "#b199ea", "#c1a7fe", "#c0a6fd", "#af97e6", "#aa93e0", "#c1a7fe", "#b79ef2", "#b89bf9", "#ab90e9", "#a98ee6", "#bc9efe", "#bc9efe", "#bc9efe", "#ad91eb", "#bc9efe", "#b397f4", "#af8ff4", "#ad8df1", "#ae8ff3", "#b796fe", "#b796fe", "#b796fe", "#b695fe", "#b796fe", "#b796fe", "#af8bfa", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe" };
	background_hex = { "#889191", "#8a9bfa", "#5efa98", "#85f4f4", "#fe968f", "#fe9bf0", "#e5f79b", "#e3f2f2", "#c0cccb", "#c0cce6", "#b8e6cc", "#bce5e5", "#decccc", "#dfcce6", "#d6e6cd", "#d7e4e5" };
	//{ "#a7a7a7", "#b8b9b8", "#a7a7a7", "#a8a9a8", "#9d9d9c", "#b7b7b6", "#b5b6b5", "#aeaead", "#a4a4a4", "#b5b5b4", "#a6a7a6", "#a3a4a3", "#b5b5b4", "#a4a5a4", "#a2a2a1", "#b2b2b1", "#b6b6b5", "#ababab", "#b6b6b5", "#b3b3b2", "#b4b4b3", "#b3b3b3", "#aeaead", "#adadad", "#a1a1a0", "#acacac", "#b8b9b8", "#b5b5b5", "#b2b2b2", "#afb0af", "#a5a5a4", "#aaabaa", "#a4a4a3", "#aaaaa9", "#b4b4b3", "#adadad", "#b3b4b3", "#afafaf", "#a7a7a7", "#b0b0b0", "#adadad", "#a7a7a6", "#9f9f9f", "#aeaead", "#a7a8a7", "#b3b3b2", "#a2a3a2", "#a2a3a2", "#a0a09f", "#aeaead", "#b4b4b4", "#b6b6b5", "#a5a5a5", "#b0b0af", "#a2a2a1", "#b8b8b8", "#b3b4b3", "#a7a7a6", "#a8a8a7", "#a6a6a5", "#a8a8a8", "#acacac", "#abacab", "#afafae", "#b7b7b6", "#b8b8b7", "#a2a3a2", "#a1a1a1", "#b3b4b3", "#acadac", "#a1a1a0", "#a6a6a6", "#9c9c9b", "#9e9e9d", "#a0a09f", "#9c9c9c", "#a2a3a2", "#a9a9a8", "#acacac", "#b7b7b6", "#b4b5b4", "#adadac", "#b0b0b0", "#a7a8a7", "#9d9e9d", "#9f9f9f", "#a0a0a0", "#b6b6b6", "#a6a7a6", "#a9aaa9", "#a8a8a7", "#9e9e9d", "#aeaead", "#a3a3a2", "#a9a9a8", "#a4a4a4", "#a6a6a5", "#b5b6b5", "#a6a7a6" };
}

FLinearColor ALightController::serially() {
	if (serial < protan_hex.Num()) {
		hexColor = protan_hex[serial];
	}
	else if (serial >= protan_hex.Num() && serial < protan_hex.Num() + deutan_hex.Num()) {
		hexColor = deutan_hex[serial - protan_hex.Num()];
	}
	else if (serial >= protan_hex.Num() + deutan_hex.Num() && serial < protan_hex.Num() + deutan_hex.Num() + tritan_hex.Num()) {
		hexColor = tritan_hex[serial - protan_hex.Num() - deutan_hex.Num()];
	}
	else if (serial < protan_hex.Num() + deutan_hex.Num() + tritan_hex.Num() + background_hex.Num()) {
		hexColor = background_hex[serial - protan_hex.Num() - deutan_hex.Num() - tritan_hex.Num()];
	}
	else {
		serial = 0;
	}
	serial++;
	return convert_hex();
}



void ALightController::cube_colors(float start, float end, TArray<FLinearColor>& outColor) {
	TArray<FLinearColor> corners;
	float arr[2] = { start, end };
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				corners.Add(FLinearColor(arr[i], arr[j], arr[k]));
			}
		}
	}
	outColor = corners;
}