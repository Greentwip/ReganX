// UnrealQuest.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Questo.h"

#include "UnrealSubStage.h"

#include "UnrealStage.generated.h"

UCLASS(BlueprintType)
class UNREALADVENTUREGAME_API UUnrealStage : public UObject
{
	GENERATED_BODY()
	
public:
	// Default constructor required by Unreal Engine
	UUnrealStage();
	
	// Constructor that takes FObjectInitializer
	UUnrealStage(const FObjectInitializer& ObjectInitializer);
	
	// Function to initialize the quest with QuestSystem
	void Initialize(Stage& stage);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetOrCreateSubStage(const FString& SubStageName, UPARAM(ref) UUnrealSubStage*& OutUnrealSubStage);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetSubStage(const FString& SubStageName, UPARAM(ref) UUnrealSubStage*& OutUnrealSubStage);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetActiveSubStage(UPARAM(ref) UUnrealSubStage*& OutUnrealSubStage);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteSubStage(const FString& SubStageName);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FString Name;
	
private:
	Stage* mStage;  // Use a pointer to avoid issues with UObject constructors
};
