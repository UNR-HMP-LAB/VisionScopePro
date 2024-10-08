// AStimuli.cpp

#include "AStimuli.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

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
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Sphere mesh for stimuli assigned successfully."));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find mesh for stimuli."));
        }
    }

    // Find and assign the M_Stimuli material
    static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Game/Material/M_Stimuli.M_Stimuli"));
    if (MaterialAsset.Succeeded())
    {
        MeshComponent->SetMaterial(0, MaterialAsset.Object);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("M_Stimuli material assigned successfully."));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find M_Stimuli material."));
        }
    }
}

// Called when the game starts or when spawned
void AStimuli::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AStimuli::BeginPlay() called."));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AStimuli::BeginPlay() called."));
    }

    if (MeshComponent)
    {
        // Create a dynamic material instance based on the M_Stimuli material
        DynamicMaterial = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(0), this);
        MeshComponent->SetVisibility(true);
        if (DynamicMaterial)
        {
            // Apply the dynamic material instance to the mesh component
            MeshComponent->SetMaterial(0, DynamicMaterial);
            UE_LOG(LogTemp, Warning, TEXT("Dynamic material instance created successfully."));
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Dynamic material instance created successfully."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create dynamic material instance."));
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create dynamic material instance."));
            }
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
        UE_LOG(LogTemp, Warning, TEXT("SetScale called with Desired Diameter: %f"), DesiredDiameter);
        // Calculate the scale factor needed to match the desired diameter
        float OriginalDiameter = 100.0f; // Original diameter of UE sphere mesh is 100 units
        float ScaleFactor = DesiredDiameter / OriginalDiameter;
        FVector FinalScale = FVector(ScaleFactor);
        MeshComponent->SetWorldScale3D(FinalScale);

        FVector AppliedScale = MeshComponent->GetComponentScale();
        UE_LOG(LogTemp, Warning, TEXT("Applied Scale: %s"), *AppliedScale.ToString());

        if (GEngine)
        {
            FString DebugMessage = FString::Printf(TEXT("Stimuli scale set to: %s"), *FinalScale.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, DebugMessage);
        }
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

        UE_LOG(LogTemp, Warning, TEXT("Stimuli brightness set to dB value: %f (Luminance: %f cd/m\u00B2, Material brightness: %f)"), dBValue, Luminance, BrightnessValue);

        if (GEngine)
        {
            FString DebugMessage = FString::Printf(TEXT("Stimuli brightness set to: %f dB"), dBValue);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, DebugMessage);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DynamicMaterial is null in SetBrightnessFromDb."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DynamicMaterial is null in SetBrightnessFromDb."));
        }
    }
}

// Function to hide or show the stimulus
void AStimuli::SetVisibility(bool bVisible)
{
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(bVisible);
        UE_LOG(LogTemp, Warning, TEXT("Stimuli visibility set to: %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));

        if (GEngine)
        {
            FString DebugMessage = FString::Printf(TEXT("Stimuli is now: %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, DebugMessage);
        }
    }
}