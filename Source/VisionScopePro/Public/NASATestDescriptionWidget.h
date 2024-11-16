// NASATestDescriptionWidget.h
// Copyright JoJo Petersky and University of Nevada, Reno. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NASATestDescriptionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContinueClicked);

UCLASS()
class VISIONSCOPEPRO_API UNASATestDescriptionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Set the test description text
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetTestDescription(const FString& Description);

    // Delegate for "Continue" button click
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnContinueClicked OnContinueClicked;

protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleContinueClicked();

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TestDescriptionText;

    UPROPERTY(meta = (BindWidget))
    class UButton* ContinueButton;
};
