// AStimuli.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FLogManager.h"
#include "AStimuli.generated.h"

UCLASS()
class PERIMAPXR_API AStimuli : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStimuli();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Function to set the scale of the visual stimulus
    void SetScale(float DesiredDiameter);

    // Function to set the brightness of the visual stimulus based on decibel value
    void SetBrightnessFromDb(float dBValue);

    // Function to hide or show the stimulus
    void SetVisibility(bool bVisible);

    // Settings for message toggling
    bool bEnableConsoleMessages;
    bool bEnableOnScreenMessages;
    bool bEnableSaveToLog;

private:
    // Static mesh component to represent the visual stimulus
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComponent;

    // Dynamic material instance for the visual stimulus
    UPROPERTY(VisibleAnywhere)
    UMaterialInstanceDynamic* DynamicMaterial;

    // Max output of the stimulus in nits
    UPROPERTY(EditDefaultsOnly, Category = "Stimulus Settings")
    float MaxLuminanceNits;  // 60 nits for Pico 4

    // Max brightness for Unreal Engine material (0-1)
    UPROPERTY(EditDefaultsOnly, Category = "Stimulus Settings")
    float MaxBrightness;

    // Instance of FLogManager to call in this class, ensures initiation of GEngine
    FLogManager LogManager;
    FString LogMessage;
};
