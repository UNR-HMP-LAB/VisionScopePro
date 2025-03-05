// ETestState.h

#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETestState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Running UMETA(DisplayName = "Running"),
    WaitingForInput UMETA(DisplayName = "WaitingForInput"),
    Paused UMETA(DisplayName = "Paused"),
    Completed UMETA(DisplayName = "Completed")
};