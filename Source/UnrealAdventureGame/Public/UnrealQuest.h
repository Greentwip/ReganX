// UnrealQuest.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Questo.h"

#include "UnrealStage.h"

#include "UnrealQuest.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class UNREALADVENTUREGAME_API UUnrealQuest : public UObject
{
	GENERATED_BODY()
	
public:
	// Default constructor required by Unreal Engine
	UUnrealQuest();
	
	// Constructor that takes FObjectInitializer
	UUnrealQuest(const FObjectInitializer& ObjectInitializer);
	
	void Initialize(Quest& quest);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetOrCreateStage(const FString& StageName, UPARAM(ref) UUnrealStage*& OutUnrealStage);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetStage(const FString& StageName, UPARAM(ref) UUnrealStage*& OutUnrealStage);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetActiveStage(UPARAM(ref) UUnrealStage*& OutUnrealStage);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FString Name;
	
private:
	Quest* mQuest;  // Use a pointer to avoid issues with UObject constructors
};
