#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FoveVRSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Fove VR"))
class UFoveVRSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Force the game to be started in VR mode, even if the headset is disconnected
	 * Makes FFoveHMD::IsHMDConnected() function always return true, independent of actual connection status
	 */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category = "Fove VR", DisplayName = "Force start in VR")
		bool bForceStartInVR = true;
};