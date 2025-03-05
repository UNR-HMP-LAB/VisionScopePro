// AFixationPoint.cpp

#include "AFixationPoint.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h" // For GEngine

// Sets default values
AFixationPoint::AFixationPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Sphere mesh for fixation point assigned successfully."));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find mesh for fixation point."));
        }
    }

    // Find and assign the M_FixationPoint material
    static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Game/Material/M_FixationPoint.M_FixationPoint"));
    if (MaterialAsset.Succeeded())
    {
        MeshComponent->SetMaterial(0, MaterialAsset.Object);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("M_FixationPoint material assigned successfully."));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find M_FixationPoint material."));
        }
    }
}

// Called when the game starts or when spawned
void AFixationPoint::BeginPlay()
{
    Super::BeginPlay();

    // Initialize the position based on the player's view
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        FVector PlayerViewLocation;
        FRotator PlayerViewRotation;
        PlayerController->GetPlayerViewPoint(PlayerViewLocation, PlayerViewRotation);

        // Set the initial fixation point position in front of the player's view
        FVector FixationPointPosition = PlayerViewLocation + PlayerViewRotation.Vector() * DistanceFromPlayer;
        SetActorLocation(FixationPointPosition);
    }
}

// Called every frame
void AFixationPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        FVector PlayerViewLocation;
        FRotator PlayerViewRotation;
        PlayerController->GetPlayerViewPoint(PlayerViewLocation, PlayerViewRotation);

        // Set the fixation point position in front of the player's view
        FVector FixationPointPosition = PlayerViewLocation + PlayerViewRotation.Vector() * DistanceFromPlayer;
        SetActorLocation(FixationPointPosition);
    }
}

void AFixationPoint::SetScale(float DesiredDiameter)
{
    if (MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AFixationPoint.cpp=> SetScale called with Desired Diameter: %f"), DesiredDiameter);
        // Calculate the scale factor needed to match the desired diameter
        float OriginalDiameter = 100.0f; // Original diameter of UE sphere mesh is 100 units
        float ScaleFactor = DesiredDiameter / OriginalDiameter;
        FVector FinalScale = FVector(ScaleFactor);
        MeshComponent->SetWorldScale3D(FinalScale);

        FVector AppliedScale = MeshComponent->GetComponentScale();
        UE_LOG(LogTemp, Warning, TEXT("AFixationPoint.cpp=> Applied Scale: %s"), *AppliedScale.ToString());

        if (GEngine)
        {
            FString DebugMessage = FString::Printf(TEXT("AFixationPoint.cpp=> Fixation point scale set to: %s"), *FinalScale.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, DebugMessage);
        }
    }
}