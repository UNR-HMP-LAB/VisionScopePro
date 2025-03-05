// ABackgroundSphere.cpp

#include "ABackgroundSphere.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h" // For GEngine

// Sets default values
ABackgroundSphere::ABackgroundSphere()
{
    // Set actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create and attach the static mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Find and assign the SM_SphereShell mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Game/Mesh/SM_SphereShell.SM_SphereShell"));
    if (SphereMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Sphere mesh for background assigned successfully."));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find background sphere mesh."));
        }
    }

    // Find and assign the M_BackgroundSphere material
    static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Game/Material/M_BackgroundSphere.M_BackgroundSphere"));
    if (MaterialAsset.Succeeded())
    {
        MeshComponent->SetMaterial(0, MaterialAsset.Object);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("M_BackgroundSphere material assigned successfully."));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find M_BackgroundSphere material."));
        }
    }
}

// Called when the game starts or when spawned
void ABackgroundSphere::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ABackgroundSphere::BeginPlay() called."));
    }
}

// Called every frame
void ABackgroundSphere::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABackgroundSphere::SetScale(float Diameter)
{
    if (MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABackgroundSphere.cpp=> SetScale called with Diameter: %f"), Diameter);
        // Calculate the scale factor needed to match the desired diameter
        float OriginalDiameter = 320.0f; // Original diameter of shell sphere mesh is 320 units
        float ScaleFactor = Diameter / OriginalDiameter;
        FVector FinalScale = FVector(ScaleFactor);
        MeshComponent->SetWorldScale3D(FinalScale);

        FVector AppliedScale = MeshComponent->GetComponentScale();
        UE_LOG(LogTemp, Warning, TEXT("ABackgroundSphere.cpp=> Applied Scale: %s"), *AppliedScale.ToString());

        if (GEngine)
        {
            FString DebugMessage = FString::Printf(TEXT("ABackgroundSphere.cpp=> Background sphere scale set to: %s"), *FinalScale.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, DebugMessage);
        }
    }
}