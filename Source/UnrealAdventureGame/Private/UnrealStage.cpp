// UUnrealStage.cpp

#include "UnrealStage.h"

UUnrealStage::UUnrealStage()
{
	// Default constructor implementation
	// Make sure to initialize any variables here if needed
}

UUnrealStage::UUnrealStage(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Constructor with FObjectInitializer implementation
	// Make sure to initialize any variables here if needed
}

void UUnrealStage::Initialize(Stage& stage)
{
	mStage = &stage;
}

bool UUnrealStage::GetOrCreateSubStage(const FString& SubStageName, UPARAM(ref) UUnrealSubStage*& OutUnrealSubStage)
{
	// Convert FString to std::string
	std::string SubStageNameString(TCHAR_TO_UTF8(*SubStageName));
	
	
	// Check if the quest was created successfully
	if (mStage->createEmptySubStage(SubStageNameString))
	{
		UUnrealSubStage* NewSubStage = NewObject<UUnrealSubStage>(this);
		
		NewSubStage->Initialize(*mStage->getSubStage(SubStageNameString));
		NewSubStage->Name = SubStageName;
		OutUnrealSubStage = NewSubStage;

		return true;
	}
	else
	{
		UUnrealSubStage* NewSubStage = NewObject<UUnrealSubStage>(this);
		
		NewSubStage->Initialize(*mStage->getSubStage(SubStageNameString));
		NewSubStage->Name = SubStageName;
		OutUnrealSubStage = NewSubStage;
	}
	
	return false;

}

bool UUnrealStage::GetSubStage(const FString& SubStageName, UPARAM(ref) UUnrealSubStage*& OutUnrealSubStage)
{
	// Convert FString to std::string
	std::string SubStageNameString(TCHAR_TO_UTF8(*SubStageName));
	
	
	// Check if the quest was created successfully
	if (mStage->findSubStage(SubStageNameString))
	{
		UUnrealSubStage* NewSubStage = NewObject<UUnrealSubStage>(this);
		
		NewSubStage->Initialize(*mStage->getSubStage(SubStageNameString));
		NewSubStage->Name = SubStageName;
		OutUnrealSubStage = NewSubStage;
		
		return true;
	}
	else
	{
		// Log an error or handle the failure in some way
		UE_LOG(LogTemp, Error, TEXT("Failed to create an empty quest for %s."), *SubStageName);
		
		// Returning false to indicate failure
		return false;
	}
}


bool UUnrealStage::GetActiveSubStage(UPARAM(ref) UUnrealSubStage*& OutUnrealSubStage)
{
	// Check if the quest was created successfully
	if (mStage->getActiveSubStage() != std::nullopt)
	{
		UUnrealSubStage* NewSubStage = NewObject<UUnrealSubStage>(this);
		
		NewSubStage->Initialize(*mStage->getActiveSubStage());
		OutUnrealSubStage = NewSubStage;
		
		return true;
	}
	else
	{
		// Log an error or handle the failure in some way
		UE_LOG(LogTemp, Error, TEXT("SubStage does not exist."));
		
		// Returning false to indicate failure
		return false;
	}
}


void UUnrealStage::CompleteSubStage(const FString& SubStageName)
{
	// Convert FString to std::string
	std::string SubStageNameString(TCHAR_TO_UTF8(*SubStageName));
	
	mStage->completeSubStage(SubStageNameString);
	
}
