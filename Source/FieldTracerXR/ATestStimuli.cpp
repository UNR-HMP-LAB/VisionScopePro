// TestStimuli.cpp

#include "TestStimuli.h"
#include "Engine/World.h"

ATestStimuli::ATestStimuli() {
    // Set default values
    StimuliRadius = 1000.0f;  // May need to adjust this
    TestType = ETestType::TEST_24_2;  // Default test type
}

void ATestStimuli::BeginPlay() {
    Super::BeginPlay();

    // Spawn the fixation point at the origin or a specified location
    if (FixationActorClass) {
        FVector FixationLocation = FVector(0.0f, 0.0f, StimuliRadius); // Directly in front of the player at the stimuli radius distance
        FRotator FixationRotation = FRotator::ZeroRotator; // No rotation needed
        AActor* FixationActor = GetWorld()->SpawnActor<AActor>(FixationActorClass, FixationLocation, FixationRotation);
        // Additional setup for the fixation actor can go here
    }

    GenerateStimuliPattern();
}

void ATestStimuli::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // Update fixation point location logic
    // For example, if the player moves around we want the fixation point to follow their gaze direction
}

void ATestStimuli::GeneratePattern_10_2() {
    const int32 PointsPerRow = 5; // Adjust according to the 10-2 test layout
    const float DegreeStep = 2.0f; // Assuming each point represents 2 degrees of visual angle

    GeneratePattern(PointsPerRow, DegreeStep);
}

void ATestStimuli::GeneratePattern_24_2() {
    const int32 PointsPerRow = 12; // Adjust according to the 24-2 test layout
    const float DegreeStep = 6.0f; // Assuming each point represents 6 degrees of visual angle

    GeneratePattern(PointsPerRow, DegreeStep);
}

void ATestStimuli::GeneratePattern(int32 PointsPerRow, float DegreeStep) {
    for (int32 Row = -PointsPerRow; Row <= PointsPerRow; ++Row) {
        for (int32 Col = -PointsPerRow; Col <= PointsPerRow; ++Col) {
            if (Row == 0 && Col == 0) continue; // Skip the central fixation point

            FVector SpawnLocation = PolarToCartesian(
                StimuliRadius,
                FMath::DegreesToRadians(Row * DegreeStep),
                FMath::DegreesToRadians(Col * DegreeStep)
            );

            GetWorld()->SpawnActor<AActor>(StimuliActorClass, SpawnLocation, FRotator::ZeroRotator);
        }
    }
}

FVector ATestStimuli::PolarToCartesian(float Radius, float VerticalAngle, float HorizontalAngle) {
    FVector Cartesian;
    // Assuming the user is facing towards the positive Y-axis 
    // Adjust the axis as needed
    Cartesian.X = Radius * FMath::Cos(VerticalAngle) * FMath::Sin(HorizontalAngle);
    Cartesian.Y = Radius * FMath::Sin(VerticalAngle);
    Cartesian.Z = Radius * FMath::Cos(VerticalAngle) * FMath::Cos(HorizontalAngle);
    return Cartesian;
}

void ATestStimuli::RecordStimulusResult(const FVector& Location, bool bSeen) {
    // Create a result struct and add it to the array
    FTestResults NewResult(Location, bSeen);
    TestResultsArray.Add(NewResult);
}