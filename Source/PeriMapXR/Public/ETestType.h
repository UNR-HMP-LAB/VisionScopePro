// ETestType.h

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETestType : uint8 {
    TEST_10_2 UMETA(DisplayName = "10-2 Test"),
    TEST_24_2 UMETA(DisplayName = "24-2 Test")
};