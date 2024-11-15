// NASAUserWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AstronautProfile.h"
#include "NASAUserWidget.generated.h"

UCLASS()
class VISIONSCOPEPRO_API UNASAUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SelectAstronaut(int32 AstronautNumber);

    UFUNCTION(BlueprintCallable, Category = "Menu")
    FString GetTestOverview() const;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void StartTestSequence();

protected:
    virtual void NativeConstruct() override;

private:
    void UpdateProfileWithTestResult();
    FString GetAstronautDescription(int32 AstronautNumber) const;
};
