// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "Questo.h"

#include "UnrealQuest.h"

#include "QuestoSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALADVENTUREGAME_API UQuestoSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	// Sets default values for this subsystem's properties
	UQuestoSubsystem();
	~UQuestoSubsystem();

	// Called when the game starts
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// Called when the game ends
	virtual void Deinitialize() override;
	
	// Blueprint callable function for branching in Blueprints
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetOrCreateQuest(const FString& QuestName, UPARAM(ref) UUnrealQuest*& OutUnrealQuest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetQuest(const FString& QuestName, UPARAM(ref) UUnrealQuest*& OutUnrealQuest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetActiveQuest(UPARAM(ref) UUnrealQuest*& OutUnrealQuest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetActiveQuest(const FString& QuestName);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearGibberishForCharacter(const FString& Id);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetDialogForCharacter(const FString& Id, UPARAM(ref) UUnrealDialog*& OutUnrealDialog);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetDialogForCharacter(const FString& Id, const FString& Content);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AppendGibberishForCharacter(const FString& character, const FString& id, const TArray<uint8>& gibberish);


private:
	QuestSystem mQuestSystem;
};
