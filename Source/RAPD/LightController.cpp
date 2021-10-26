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
	if (repititions % 2 == 1) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Right");
	else GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Left");

	if (repititions <= 0)
	{
		GetWorldTimerManager().ClearTimer(LightTimerHandle);
		return;
	}

	if(eye_tracking_ready) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Ready");

	UMaterialInstanceDynamic* mat = D_left_and_right[repititions%2];
	mat->GetScalarParameterValue(TEXT("intensity"), current_intensity[repititions%2]);
	current_intensity[repititions%2] *= dropoff;
	mat->SetScalarParameterValue(TEXT("intensity"), current_intensity[repititions % 2]);
	for (int32 i = 0; i < lights.Num(); i++)
	{
		lights[i]->GetStaticMeshComponent()->SetMaterial(0, mat);
	}
	repititions--;
}

void ALightController::Darkness(TArray<AStaticMeshActor*> lights)
{
	UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(Dark_Material, this);
	for (int32 i = 0; i < lights.Num(); i++)
	{
		lights[i]->GetStaticMeshComponent()->SetMaterial(0, mat);
	}
	if (repititions <= 0)
	{
		GetWorldTimerManager().ClearTimer(DarkTimerHandle);
		FString tempstring = FDateTime().Now().ToString();
		SaveArrayText(SavingLocation, ID+"_"+tempstring+".csv", CSV_file, true);
		return;
	}
}

void ALightController::TestProtocol(TArray<AStaticMeshActor*> lights)
{
	FTimerDelegate LightTimerDelegate, DarkTimerDelegate;

	while (FFoveHMD::Get()) {
		hmd = FFoveHMD::Get();
		hmd->IsEyeTrackingReady(eye_tracking_ready);
	}

	D_left_and_right.Empty();
	repititions *= 2;

	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[0], this));
	D_left_and_right[0]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);
	D_left_and_right.Add(UMaterialInstanceDynamic::Create(Left_and_right[1], this));
	D_left_and_right[1]->SetScalarParameterValue(TEXT("intensity"), initial_light_intensity);

	current_intensity = { initial_light_intensity , initial_light_intensity };

	LightTimerDelegate.BindUFunction(this, FName("IncreaseLuminance"), lights);
	DarkTimerDelegate.BindUFunction(this, FName("Darkness"), lights);

	GetWorldTimerManager().SetTimer(LightTimerHandle, LightTimerDelegate, light_duration + dark_duration, true, 4.0f);
	GetWorldTimerManager().SetTimer(DarkTimerHandle, DarkTimerDelegate, light_duration + dark_duration, true, 4.0f + light_duration);
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
	Super::BeginPlay();
	
}

// Called every frame
void ALightController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Elapsed_time += DeltaTime;
	float l = -1.0f, r = -1.0f;

	FString TimeStamp = FString::SanitizeFloat(Elapsed_time), Intensity_Left = FString::SanitizeFloat(current_intensity[0]), \
		Intensity_Right = FString::SanitizeFloat(current_intensity[1]);

	FString Pupil_Diameter_Left = "", Pupil_Diameter_Right = "";
	
	if (hmd && eye_tracking_ready) {
		hmd->GetPupilRadius(EFoveEye::Left, l);
		hmd->GetPupilRadius(EFoveEye::Right, r);
		Pupil_Diameter_Left = FString::SanitizeFloat(l);
		Pupil_Diameter_Right = FString::SanitizeFloat(r);		
	}

	CSV_file.Add(TimeStamp + "," + Intensity_Left + "," + Pupil_Diameter_Left + "," + Intensity_Right + "," + Pupil_Diameter_Right);
}

