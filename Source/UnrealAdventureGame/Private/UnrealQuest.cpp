// UnrealQuest.cpp

#include "UnrealQuest.h"

UUnrealQuest::UUnrealQuest()
{
	// Default constructor implementation
	// Make sure to initialize any variables here if needed
}

UUnrealQuest::UUnrealQuest(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Constructor with FObjectInitializer implementation
	// Make sure to initialize any variables here if needed
}

void UUnrealQuest::Initialize(Quest& quest)
{
	mQuest = &quest;
}


bool UUnrealQuest::GetOrCreateStage(const FString& StageName, UPARAM(ref) UUnrealStage*& OutUnrealStage)
{
	// Convert FString to std::string
	std::string StageNameString(TCHAR_TO_UTF8(*StageName));
	
	
	// Check if the quest was created successfully
	if (mQuest->createEmptyStage(StageNameString))
	{
		UUnrealStage* NewStage = NewObject<UUnrealStage>(this);
		
		NewStage->Initialize(*mQuest->getStage(StageNameString));
		NewStage->Name = StageName;
		OutUnrealStage = NewStage;
	}
	else
	{
		UUnrealStage* NewStage = NewObject<UUnrealStage>(this);
		
		NewStage->Initialize(*mQuest->getStage(StageNameString));
		NewStage->Name = StageName;
		OutUnrealStage = NewStage;
	}
	return true;

}

bool UUnrealQuest::GetStage(const FString& StageName, UPARAM(ref) UUnrealStage*& OutUnrealStage)
{
	// Convert FString to std::string
	std::string StageNameString(TCHAR_TO_UTF8(*StageName));
	
	
	// Check if the quest was created successfully
	if (mQuest->findStage(StageNameString))
	{
		UUnrealStage* NewStage = NewObject<UUnrealStage>(this);
		
		NewStage->Initialize(*mQuest->getStage(StageNameString));
		NewStage->Name = StageName;
		OutUnrealStage = NewStage;
		
		return true;
	}
	else
	{
		// Log an error or handle the failure in some way
		UE_LOG(LogTemp, Error, TEXT("Failed to create an empty quest for %s."), *StageName);
		
		// Returning false to indicate failure
		return false;
	}
}

bool UUnrealQuest::GetActiveStage(UPARAM(ref) UUnrealStage*& OutUnrealStage)
{
	// Check if the quest was created successfully
	if (mQuest->getActiveStage() != std::nullopt)
	{
		UUnrealStage* NewStage = NewObject<UUnrealStage>(this);
		
		NewStage->Initialize(*mQuest->getActiveStage());
		OutUnrealStage = NewStage;
		
		return true;
	}
	else
	{
		// Log an error or handle the failure in some way
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve active stage."));
		
		// Returning false to indicate failure
		return false;
	}
}
