// ABackgroundSphere.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABackgroundSphere.generated.h"

UCLASS()
class PERIMAPXR_API ABackgroundSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABackgroundSphere();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function to set the scale of the background sphere
	void SetScale(float Diameter);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicMaterial;
};
