// UnrealQuest.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Questo.h"
#include "UnrealDialog.h"

#include "UnrealSubStage.generated.h"

UCLASS(BlueprintType)
class UNREALADVENTUREGAME_API UUnrealSubStage : public UObject
{
	GENERATED_BODY()
	
public:
	// Default constructor required by Unreal Engine
	UUnrealSubStage();
	
	// Constructor that takes FObjectInitializer
	UUnrealSubStage(const FObjectInitializer& ObjectInitializer);
	
	// Function to initialize the quest with QuestSystem
	void Initialize(SubStage& stage);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearGibberishForCharacter(const FString& Id);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetDialogForCharacter(const FString& Id, const FString& Content);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AppendGibberishForCharacter(const FString& character, const FString& id, const TArray<uint8>& gibberish);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetDialogForCharacter(const FString& Id, UPARAM(ref) UUnrealDialog*& OutUnrealDialog);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FString Name;
	
private:
	SubStage* mSubStage;  // Use a pointer to avoid issues with UObject constructors
};

