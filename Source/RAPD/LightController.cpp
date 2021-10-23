// Fill out your copyright notice in the Description page of Project Settings.


#include "LightController.h"

// Sets default values
ALightController::ALightController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ALightController::IncreaseLuminance(TArray<AStaticMeshActor*> lights, bool do_inc)
{
	UMaterialInstanceDynamic* mat;
	for (int32 i = 0; i < lights.Num(); i++) 
	{
		mat = lights[i]->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
		float prev_intensity;
		mat->GetScalarParameterValue(TEXT("intensity"), prev_intensity);
		if(do_inc) mat->SetScalarParameterValue(TEXT("intensity"), prev_intensity*10);
		else mat->SetScalarParameterValue(TEXT("intensity"), prev_intensity / 10);
	}
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

}

