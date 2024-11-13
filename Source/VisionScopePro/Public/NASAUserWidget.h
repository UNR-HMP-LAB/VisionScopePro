// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NASAUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class VISIONSCOPEPRO_API UNASAUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Selection")
    FString SelectedAstronaut;

    UPROPERTY(BlueprintReadOnly, Category = "Tests")
    TArray<FString> TestSequence;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SelectAstronaut(int32 AstronautNumber);

    UFUNCTION(BlueprintCallable, Category = "Menu")
    FString GetTestOverview() const;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void StartTestSequence();

protected:
    virtual void NativeConstruct() override;

private:
    FString GetAstronautDescription(int32 AstronautNumber) const;
    void SetupTestSequence();
};
