// Fill out your copyright notice in the Description page of Project Settings.


#include "MetamorphopsiaController.h"

#define MAX_LAYERS 3

// Sets default values
AMetamorphopsiaController::AMetamorphopsiaController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AMetamorphopsiaController::Initiate(TArray<UMaterialInstanceDynamic*> distortion_mats) {
	//get scotoma values from past_meta into meta
	simulated_distortions = distortion_mats;

	//create material instance from current meta

	//adjust chart based on past chart info

	//set the material to the chart
}

void AMetamorphopsiaController::Simulate(int which_distortion, UStaticMeshComponent* distortion_plane) {
	simulated_distortions[which_distortion]->SetScalarParameterValue(FName("monocular_l"), 1.0f);
	simulated_distortions[which_distortion]->SetScalarParameterValue(FName("monocular_r"), 0.0f);
	distortion_plane->SetMaterial(0, simulated_distortions[which_distortion]);
}

void AMetamorphopsiaController::fromMaterialtoScotoma_C(Eye eye, UMaterialInstanceDynamic* mat, FScotoma_C& scotomas) {
	float j;
	FScotoma_C sc;
	TMap<Eye, FString> prefix_mat = { {Eye::Left, "Left_"}, {Eye::Right, "Right_"} };

	for (int8 layer = 0; layer < MAX_LAYERS; layer++) {
		mat->GetScalarParameterValue(FName(prefix_mat[eye] + "Sigma" + FString::FromInt(layer)), j);
		FScotomata_C s;
		if (j > 0) {
			sc.layers_active[layer] = true;
			s.Sigma = j;
			FLinearColor mean;
			mat->GetVectorParameterValue(FName(prefix_mat[eye] + "Mean" + FString::FromInt(layer)), mean);
			s.MeanColor = mean;
			mat->GetScalarParameterValue(FName(prefix_mat[eye] + "Rotation" + FString::FromInt(layer)), j);
			s.Rotation = j;
			mat->GetScalarParameterValue(FName(prefix_mat[eye] + "Distortion" + FString::FromInt(layer)), j);
			s.Distortion = j;
			mat->GetScalarParameterValue(FName(prefix_mat[eye] + "Weight" + FString::FromInt(layer)), j);
			s.Weight = j;
			mat->GetVectorParameterValue(FName(prefix_mat[eye] + "Boundary" + FString::FromInt(layer)), mean);
			s.Boundary = mean;
		}
		sc.layers.Add(s);

	}
	scotomas = sc;
}

void AMetamorphopsiaController::printScotomata(FScotomata_C s, FString& print_s) {
	FString ret = "Mean : ";
	ret += FString::SanitizeFloat(s.MeanColor.R) + " " + FString::SanitizeFloat(s.MeanColor.G);
	ret += "\nSigma: ";
	ret += FString::SanitizeFloat(s.Sigma);
	ret += "\nRotation: ";
	ret += FString::SanitizeFloat(s.Rotation);
	ret += "\nDistortion: ";
	ret += FString::SanitizeFloat(s.Distortion);
	ret += "\nWeight: ";
	ret += FString::SanitizeFloat(s.Weight);
	print_s = ret;
}

void AMetamorphopsiaController::fromScotoma_CtoMaterial(Eye eye, bool post, FScotoma_C scotomas, UStaticMeshComponent* distortion_plane, UMaterialInstanceDynamic*& mat) {
	mat = UMaterialInstanceDynamic::Create(bin_mat, this);
	TMap<Eye, FString> prefix_mat = { {Eye::Left, "Left_"}, {Eye::Right, "Right_"} };
	TMap<Eye, FString> monocular_prefix = { {Eye::Left, "monocular_r"}, {Eye::Right, "monocular_l"} };

	mat->SetScalarParameterValue(FName("monocular_l"), 1.0f);
	mat->SetScalarParameterValue(FName("monocular_r"), 1.0f);
	if (!post) {
		if (mono) mat->SetScalarParameterValue(FName(monocular_prefix[eye]), 0.0f);
		for (int8 i = 0; i < scotomas.layers_active.Num(); i++) {
			if (scotomas.layers_active[i]) {
				mat->SetVectorParameterValue(FName(prefix_mat[eye] + "Mean" + FString::FromInt(i)), scotomas.layers[i].MeanColor);
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Sigma" + FString::FromInt(i)), scotomas.layers[i].Sigma);
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Rotation" + FString::FromInt(i)), scotomas.layers[i].Rotation);
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Distortion" + FString::FromInt(i)), scotomas.layers[i].Distortion);
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Weight" + FString::FromInt(i)), scotomas.layers[i].Weight);
				mat->SetVectorParameterValue(FName(prefix_mat[eye] + "Boundary" + FString::FromInt(i)), scotomas.layers[i].Boundary);
			}
		}
		if (distortion_plane) distortion_plane->SetMaterial(0, mat);
	}
	else {
		FLinearColor temp_mean;
		float temp_float;
		if (mono) mat->SetScalarParameterValue(FName(monocular_prefix[eye]), 0.0f);
		for (int8 i = 0; i < scotomas.layers_active.Num(); i++) {
			if (scotomas.layers_active[i]) {
				temp_mean = scotomas.layers[i].MeanColor;
				mat->SetVectorParameterValue(FName(prefix_mat[eye] + "Mean" + FString::FromInt(i)), temp_mean);
				temp_float = scotomas.layers[i].Sigma;
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Sigma" + FString::FromInt(i)), temp_float);
				temp_float = scotomas.layers[i].Rotation;
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Rotation" + FString::FromInt(i)), temp_float);
				temp_float = scotomas.layers[i].Distortion;
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Distortion" + FString::FromInt(i)), temp_float);
				mat->SetScalarParameterValue(FName(prefix_mat[eye] + "Weight" + FString::FromInt(i)), scotomas.layers[i].Weight);
				mat->SetVectorParameterValue(FName(prefix_mat[eye] + "Boundary" + FString::FromInt(i)), scotomas.layers[i].Boundary);
			}
		}
		TArray<FWeightedBlendable> postArray;
		postArray.Add(FWeightedBlendable(1.0, mat));
		Corrected_setting.WeightedBlendables = postArray;
		camera->PostProcessSettings = Corrected_setting;
	}
}

void AMetamorphopsiaController::alter_camera_setting(bool is_normal_setting) {
	if (is_normal_setting) {
		camera->PostProcessSettings = Normal_setting;
	}
	else {
		camera->PostProcessSettings = Corrected_setting;
	}
}

void AMetamorphopsiaController::manipulationLayer(FScotoma_C replica, int32& which_layer, FScotoma_C& out) {
	int8 i = 0;

	which_layer = -1;

	FScotoma_C sc;

	for (i = 0; i < replica.layers_active.Num(); i++) {
		if (replica.layers_active[i]) {
			sc.layers_active[i] = true;
			sc.layers.Add(replica.layers[i]);
		}
		else {
			sc.layers_active[i] = true;

			FScotomata_C new_sc;
			new_sc.MeanColor = FLinearColor(0.5f, 0.5f, 0.0f, 1.0f);
			new_sc.Sigma = 0.01f;
			new_sc.Rotation = 0.0f;
			new_sc.Distortion = 0.0f;
			new_sc.Weight = 1.0;
			which_layer = i;

			sc.layers.Add(new_sc);
			break;
		}
	}

	out = sc;
}

void AMetamorphopsiaController::move_location(FScotoma_C sc, TArray<int32> which_layers, float x_val, float y_val, FScotoma_C& out) {
	for (int8 i = 0; i < which_layers.Num(); i++)
	{
		sc.layers[which_layers[i]].MeanColor.G -= x_val / 500;
		sc.layers[which_layers[i]].MeanColor.R += y_val / 500;
	}
	out = sc;
}

void AMetamorphopsiaController::alter_scotoma_property(FScotoma_C sc, int32 which_layer, int32 which_property, float val, FScotoma_C& out) {
	float temp;
	if (sc.layers_active[which_layer]) {
		switch (which_property)
		{
		case 1:
			temp = sc.layers[which_layer].Sigma + val * 0.001f;
			if (min_limit[1] < temp && max_limit[1] > temp) sc.layers[which_layer].Sigma = temp;
			sc.layers[which_layer].Weight = 1.0;
			sc.layers[which_layer].Boundary = FLinearColor::Red;
			break;
		case 2:
			sc.layers[which_layer].Weight = 0.0;
			temp = sc.layers[which_layer].Rotation + val * 1.0f;
			if (min_limit[2] < temp && max_limit[2] > temp) sc.layers[which_layer].Rotation = temp;
			break;
		case 3:
			sc.layers[which_layer].Weight = 0.0;
			temp = sc.layers[which_layer].Distortion + val * 0.01f;
			if (min_limit[3] < temp && max_limit[3] > temp) sc.layers[which_layer].Distortion = temp;
			break;
		case 4:
			sc.layers[which_layer].Boundary = FLinearColor::Black;
			temp = sc.layers[which_layer].Weight + val * 0.015f;
			if (min_limit[4] < temp && max_limit[4] > temp) sc.layers[which_layer].Weight = temp;
			break;
		default:
			break;
		}
	}
	else {
		manipulationLayer(sc, which_layer, sc);
	}
	out = sc;
}

void AMetamorphopsiaController::save_patientData(TArray<FScotomata_C> arr_sc, bool simulation) {
	TArray<FString> modeling_responses = { "Example Scotoma, Measure Mean.X, Measured Mean.Y, Measured Size, Measured Rotational Distortion, Spatial Distortion" };

	for (int8 i = 0; i < arr_sc.Num(); i++)
	{
		FString temp = FString::FromInt(i) + ",";
		temp += FString::SanitizeFloat(arr_sc[i].MeanColor.R) + "," + FString::SanitizeFloat(arr_sc[i].MeanColor.G) + ",";
		temp += FString::SanitizeFloat(arr_sc[i].Sigma) + ",";
		temp += FString::SanitizeFloat(arr_sc[i].Rotation) + ",";
		temp += FString::SanitizeFloat(arr_sc[i].Distortion) + ",";
		modeling_responses.Add(temp);
	}
	FString savfile = FString::FromInt(Subject_ID) + "_meta_" + FDateTime::Now().ToString() + ".csv";
	UAssessmentMetrics_C::SaveArrayText(FPaths::ProjectSavedDir(), savfile, modeling_responses, true);
}

void AMetamorphopsiaController::new_simulation(UStaticMeshComponent* distortion_plane)
{
	Simulate(distortion, distortion_plane);
	if (current_scotoma.layers.Num() != active_layer) {
		responses.Add(current_scotoma.layers[active_layer]);
	}
	distortion = (distortion + 1) % 6;
	reload_scotoma(distortion_plane);
}

void AMetamorphopsiaController::change_eye(Eye eye, UStaticMeshComponent* distortion_plane)
{
	current_eye = eye;
	UMaterialInstanceDynamic* mat;
	fromScotoma_CtoMaterial(current_eye, post_process_enabled, current_scotoma, distortion_plane, mat);
}

void AMetamorphopsiaController::change_property(bool inc)
{
	if (inc) {
		selected_property = (selected_property + 1) % 5;
	}
	else {
		selected_property = (selected_property + 4) % 5;
	}
}

void AMetamorphopsiaController::activate_see_through_mode(bool change_level, UStaticMeshComponent* distortion_plane, UMaterial* post_material) {
	if (!change_level) {
		post_process_enabled = true;
		bin_mat = post_material;
		selected_property = 4;
		UMaterialInstanceDynamic* mat;
		fromScotoma_CtoMaterial(current_eye, post_process_enabled, current_scotoma, distortion_plane, mat);
	}
}

void AMetamorphopsiaController::preload_scotoma(UStaticMeshComponent* distortion_plane)
{
	manipulationLayer(current_scotoma, active_layer, current_scotoma);
	UMaterialInstanceDynamic* mat;
	fromScotoma_CtoMaterial(current_eye, post_process_enabled, current_scotoma, distortion_plane, mat);
}

void AMetamorphopsiaController::get_started(bool post, UStaticMeshComponent* distortion_plane, UMaterial* post_material, UMaterial* plane_material)
{
	post_process_enabled = post;
	current_eye = Eye::Left;
	selected_property = 0;
	active_layer = 1;
	if (post) {
		bin_mat = post_material;
	}
	else {
		bin_mat = plane_material;
	}
	mono = false;
	responses.Empty();
	reload_scotoma(distortion_plane);
	//initialize now
}


void AMetamorphopsiaController::update_scotoma(float x_val, float y_val, UStaticMeshComponent* distortion_plane)
{
	if (active_layer!=0) {
		if (selected_property == 0) {
			TArray<int> tempa = {active_layer};
			move_location(current_scotoma, tempa, x_val, y_val, current_scotoma);
			UMaterialInstanceDynamic* mat;
			fromScotoma_CtoMaterial(current_eye, post_process_enabled, current_scotoma, distortion_plane, mat);
		}
		else if (selected_property < 5) {
			alter_scotoma_property(current_scotoma, active_layer, selected_property, x_val, current_scotoma);
			UMaterialInstanceDynamic* mat;
			fromScotoma_CtoMaterial(current_eye, post_process_enabled, current_scotoma, distortion_plane, mat);
		}
	}
}


void AMetamorphopsiaController::reload_scotoma(UStaticMeshComponent* distortion_plane)
{
	FScotoma_C temp_scotoma;
	fromMaterialtoScotoma_C(current_eye, UMaterialInstanceDynamic::Create(distortion_plane->GetMaterial(0), this), temp_scotoma);
	manipulationLayer(temp_scotoma, active_layer, current_scotoma);
	UMaterialInstanceDynamic* temp_mat;
	fromScotoma_CtoMaterial(current_eye, post_process_enabled, current_scotoma, distortion_plane, temp_mat);
}

// Called when the game starts or when spawned
void AMetamorphopsiaController::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMetamorphopsiaController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

