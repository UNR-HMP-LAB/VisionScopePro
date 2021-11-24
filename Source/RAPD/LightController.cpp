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
		GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Red, FString::Printf(TEXT("Output: %d"), position_in_sequence));
		GetWorldTimerManager().ClearTimer(LightTimerHandle);
		if(dark_duration==0.0f) Darkness(lights);
		return;
	}

	UMaterialInstanceDynamic* mat;
	float intensity = initial_light_intensity *construct_full_presentation_sequence[position_in_sequence];
	
	if (intensity > 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Light Left"));
		mat = D_left_and_right[0];
		current_intensity = { intensity, 0 };
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Light Right"));
		mat = D_left_and_right[1];
		intensity = -intensity;
		current_intensity = { 0, intensity };
	}

	mat->SetScalarParameterValue(TEXT("intensity"), intensity);
	mat->SetVectorParameterValue(TEXT("Color"), color);
	for (int32 i = 0; i < lights.Num(); i++)
	{
		lights[i]->GetStaticMeshComponent()->SetMaterial(0, mat);
	}

	position_in_sequence++;
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
		SaveArrayText(SavingLocation, ID+"_"+tempstring+".csv", CSV_file, true);
		return;
	}
	current_intensity = { 0, 0 };
}

void ALightController::Start_calibration() {
	eye_core = SRanipalEye_Core::Instance();
	if(do_calibration) eye_core->LaunchEyeCalibration_(nullptr);
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
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate;

	D_left_and_right.Empty();

	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[0], this));
	D_left_and_right[0]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);
	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[1], this));
	D_left_and_right[1]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);

	construct_full_presentation_sequence.Empty();

	for (int32 i = 0; i < dropoff_left.Num(); i++) {
		for (int32 j = 0; j < repititions; j++) {
			construct_full_presentation_sequence.Add(-dropoff_right[i]);
			construct_full_presentation_sequence.Add(dropoff_left[i]);
		}
	}

	position_in_sequence = 0;

	LightTimerDelegate.BindUFunction(this, FName("IncreaseLuminance"), lights);
	DarkTimerDelegate.BindUFunction(this, FName("Darkness"), lights);
	
	if (after_accommodation) {
		FTimerDelegate EyeDelegate;
		EyeDelegate.BindUFunction(this, FName("eyeTick"));
		current_intensity = { 0 , 0 };
		GetWorldTimerManager().SetTimer(EyeTimerHandle, EyeDelegate, .008, true, 0.0f);
	}
	if(light_duration>0.0f) GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + dark_duration, true, start_time);
	if(dark_duration>0.0f) GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + dark_duration, true, start_time + light_duration);
}

bool ALightController::LoadTextFromFile(FString FileName, TArray<FString>& TextArray, FString& TextString)
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
	Super::BeginPlay();	
}

void ALightController::eyeTick() {
	Elapsed_time += .008;
	float l = -1.0f, r = -1.0f;

	FString TimeStamp = FString::SanitizeFloat(Elapsed_time), Intensity_Left = FString::SanitizeFloat(current_intensity[0]), \
		Intensity_Right = FString::SanitizeFloat(current_intensity[1]);

	FString Pupil_Diameter_Left = "", Pupil_Diameter_Right = "";

	if (eye_tracking_ready) {
		//hmd->GetPupilRadius(EFoveEye::Left, l);
		//hmd->GetPupilRadius(EFoveEye::Right, r);
		ViveSR::anipal::Eye::EyeData_v2 data;
		int error = eye_core->GetEyeData_v2(&data);
		//TimeStamp = FString::SanitizeFloat(data.timestamp);
		Pupil_Diameter_Left = FString::SanitizeFloat(data.verbose_data.left.pupil_diameter_mm);
		Pupil_Diameter_Right = FString::SanitizeFloat(data.verbose_data.right.pupil_diameter_mm);
		//GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::White, FString::Printf(TEXT("Output: %f"), data.verbose_data.left.pupil_diameter_mm));
	}

	CSV_file.Add(TimeStamp + "," + Intensity_Left + "," + Pupil_Diameter_Left + "," + Intensity_Right + "," + Pupil_Diameter_Right);
	//SaveArrayText(SavingLocation, ID + "_" + tempstring + ".csv", CSV_file, true);
}

// Called every frame
void ALightController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

