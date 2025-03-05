// AFixationPoint.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AFixationPoint.generated.h"

UCLASS()
class PERIMAPXR_API AFixationPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFixationPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function to set the scale of the fixation point
	void SetScale(float Radius);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
	float DistanceFromPlayer = 100.0f; // Distance in front of the player

	UMaterialInstanceDynamic* DynamicMaterial;
};
