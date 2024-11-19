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
    // Set the test description text to type out
    UFUNCTION(BlueprintCallable, Category = "UI")
    void StartTestDescription(const FString& Description, float TypingSpeed = 0.05f);

    // Delegate for "Continue" button click
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnContinueClicked OnContinueClicked;

protected:
    virtual void NativeConstruct() override;

private:
    // Function to handle typing the next character
    void TypeNextCharacter();

    // We need a way to toggle caret visibility
    void ToggleCaretVisibility();

    // Handles "Continue 
    UFUNCTION()
    void HandleContinueClicked();

    // Full text to type out
    FString DescriptionText;

    // Current text being typed
    FString CurrentTypedText;

    // Caret to aid visual effects
    FString Caret;

    // Time between displaying each new character
    float TypingSpeed;

    // Timer handles
    FTimerHandle TypingTimerHandle;
    FTimerHandle CaretBlinkTimerHandle;

    // Toggle caret visibility
    bool bCaretVisible;

    // Text block to hold the test description
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TestDescriptionText;

    // Continue button for UI
    UPROPERTY(meta = (BindWidget))
    class UButton* ContinueButton;
};
