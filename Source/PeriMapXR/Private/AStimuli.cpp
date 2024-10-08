// AStimuli.cpp

#include "AStimuli.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "FLogManager.h"

// Sets default values
AStimuli::AStimuli()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Initialize max luminance and brightness values
    MaxLuminanceNits = 60.0f; // Max luminance of the Pico 4 in nits
    MaxBrightness = 1.0f; // Max brightness for Unreal Engine material (0-1 range)

    // Create and set up the static mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Find and assign the Sphere mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Game/Mesh/Sphere.Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
        LogMessage = "AStimuli::Sphere mesh for stimuli assigned successfully.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
    else
    {
        LogMessage = "AStimuli::Failed to find mesh for stimuli.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }

    // Find and assign the M_Stimuli material
    static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Game/Material/M_Stimuli.M_Stimuli"));
    if (MaterialAsset.Succeeded())
    {
        MeshComponent->SetMaterial(0, MaterialAsset.Object);
        LogMessage = "AStimuli::M_Stimuli material assigned successfully.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
    else
    {
        LogMessage = "AStimuli::Failed to find M_Stimuli material.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
}

// Called when the game starts or when spawned
void AStimuli::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        // Create a dynamic material instance based on the M_Stimuli material
        DynamicMaterial = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(0), this);
        MeshComponent->SetVisibility(true);
        if (DynamicMaterial)
        {
            // Apply the dynamic material instance to the mesh component
            MeshComponent->SetMaterial(0, DynamicMaterial);
            LogMessage = "AStimuli::Dynamic material instance created successfully.";
            LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        }
        else
        {
            LogMessage = "AStimuli::Failed to create dynamic material instance.";
            LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
        }
    }
}

// Called every frame
void AStimuli::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Function to set the scale of the visual stimulus
void AStimuli::SetScale(float DesiredDiameter)
{
    if (MeshComponent)
    {
        // Calculate the scale factor needed to match the desired diameter
        float OriginalDiameter = 100.0f; // Original diameter of UE sphere mesh is 100 units
        float ScaleFactor = DesiredDiameter / OriginalDiameter;
        FVector FinalScale = FVector(ScaleFactor);
        MeshComponent->SetWorldScale3D(FinalScale);

        FVector AppliedScale = MeshComponent->GetComponentScale();
        LogMessage = FString::Printf(TEXT("AStimuli::Stimuli scale set to : % s"), *FinalScale.ToString());
        LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
}

// Function to set the brightness of the visual stimulus based on decibel value
void AStimuli::SetBrightnessFromDb(float dBValue)
{
    if (DynamicMaterial)
    {
        // Calculate luminance from dB value using the formula: Luminance = L_max * 10^(-dB/10)
        float Luminance = MaxLuminanceNits * FMath::Pow(10.0f, -dBValue / 10.0f);

        // Normalize luminance to the 0-1 range for material brightness (0 = black, MaxLuminanceNits = full brightness)
        float BrightnessValue = FMath::Clamp(Luminance / MaxLuminanceNits, 0.0f, MaxBrightness);

        // Apply the brightness to the material's scalar parameter
        DynamicMaterial->SetScalarParameterValue(TEXT("Brightness"), BrightnessValue);

        LogMessage = FString::Printf(TEXT("AStimuli::Stimuli brightness set to: %f dB"), dBValue);
        LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
    else
    {
        LogMessage = "AStimuli::DynamicMaterial is null in SetBrightnessFromDb.";
        LogManager.LogMessage(LogMessage, ELogVerbosity::Error, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
}

// Function to hide or show the stimulus
void AStimuli::SetVisibility(bool bVisible)
{
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(bVisible);
        LogMessage = FString::Printf(TEXT("AStimuli::Stimuli is now: %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
        LogManager.LogMessage(LogMessage, ELogVerbosity::Log, 5.0f, bEnableConsoleMessages, bEnableOnScreenMessages, bEnableSaveToLog);
    }
}