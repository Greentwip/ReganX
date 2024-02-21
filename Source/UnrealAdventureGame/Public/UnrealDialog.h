// UnrealQuest.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Questo.h"

#include "UnrealDialog.generated.h"

UCLASS(BlueprintType)
class UNREALADVENTUREGAME_API UUnrealDialog : public UObject
{
	GENERATED_BODY()
	
public:
	// Default constructor required by Unreal Engine
	UUnrealDialog();
	
	// Constructor that takes FObjectInitializer
	UUnrealDialog(const FObjectInitializer& ObjectInitializer);
	
	// Function to initialize the quest with QuestSystem
	void Initialize(Dialog& dialog);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FString GetString();
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<uint8> GetGibberish();


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FString CharacterId;

private:
	Dialog* mDialog;  // Use a pointer to avoid issues with UObject constructors
};

