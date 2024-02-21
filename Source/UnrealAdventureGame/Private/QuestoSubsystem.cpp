#include "QuestoSubsystem.h"

UQuestoSubsystem::UQuestoSubsystem()
: UEngineSubsystem()
{
	// Constructor logic, if needed
}

UQuestoSubsystem::~UQuestoSubsystem()
{
	// Destructor logic, if needed
}

void UQuestoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UQuestoSubsystem::Deinitialize()
{	
	Super::Deinitialize();
}

bool UQuestoSubsystem::GetOrCreateQuest(const FString& QuestName, UPARAM(ref) UUnrealQuest*& OutUnrealQuest)
{
	// Convert FString to std::string
	std::string QuestNameString(TCHAR_TO_UTF8(*QuestName));
	
	
	// Check if the quest was created successfully
	if (mQuestSystem.createEmptyQuest(QuestNameString))
	{
		UUnrealQuest* NewQuest = NewObject<UUnrealQuest>(this);
		
		NewQuest->Initialize(mQuestSystem.getQuest(QuestNameString).value());

		NewQuest->Name = QuestName;
		OutUnrealQuest = NewQuest;
	}
	else
	{
		UUnrealQuest* NewQuest = NewObject<UUnrealQuest>(this);
		
		NewQuest->Initialize(mQuestSystem.getQuest(QuestNameString).value());
		
		NewQuest->Name = QuestName;
		OutUnrealQuest = NewQuest;

	}
	
	// Returning false to indicate failure
	return true;

}

bool UQuestoSubsystem::GetQuest(const FString& QuestName, UPARAM(ref) UUnrealQuest*& OutUnrealQuest)
{
	// Convert FString to std::string
	std::string QuestNameString(TCHAR_TO_UTF8(*QuestName));
	
	// Check if the quest was created successfully
	if (mQuestSystem.findQuest(QuestNameString))
	{
		UUnrealQuest* NewQuest = NewObject<UUnrealQuest>(this);
		
		NewQuest->Initialize(mQuestSystem.getQuest(QuestNameString).value());
		
		NewQuest->Name = QuestName;
		OutUnrealQuest = NewQuest;
		
		return true;
	}
	else
	{
		// Log an error or handle the failure in some way
		UE_LOG(LogTemp, Error, TEXT("quest does not exist for %s."), *QuestName);
		
		// Returning false to indicate failure
		return false;
	}
}

bool UQuestoSubsystem::GetActiveQuest(UPARAM(ref) UUnrealQuest*& OutUnrealQuest)
{
	// Check if the quest was created successfully
	if (mQuestSystem.getActiveQuest() != std::nullopt)
	{
		UUnrealQuest* NewQuest = NewObject<UUnrealQuest>(this);
		
		NewQuest->Initialize(*mQuestSystem.getActiveQuest());
		
		OutUnrealQuest = NewQuest;
		
		return true;
	}
	else
	{
		// Log an error or handle the failure in some way
		UE_LOG(LogTemp, Error, TEXT("quest does not exist."));
		
		// Returning false to indicate failure
		return false;
	}
}

void UQuestoSubsystem::SetActiveQuest(const FString& QuestName)
{
	// Convert FString to std::string
	std::string QuestNameString(TCHAR_TO_UTF8(*QuestName));
	
	mQuestSystem.setActiveQuest(QuestNameString);
}


void UQuestoSubsystem::SetDialogForCharacter(const FString& Id, const FString& Content)
{
	std::string IdString(TCHAR_TO_UTF8(*Id));
	std::string ContentString(TCHAR_TO_UTF8(*Content));
	
	
	Dialog* dialog = nullptr;
	
	mQuestSystem.getActiveQuest()->get().getActiveStage()->get().getActiveSubStage()->get().getDialog(IdString, &dialog);
	
	dialog->setString(ContentString);
}

void UQuestoSubsystem::ClearGibberishForCharacter(const FString& Id)
{
	std::string IdString(TCHAR_TO_UTF8(*Id));
	
	mQuestSystem.getActiveQuest()->get().getActiveStage()->get().getActiveSubStage()->get().clearGibberishForCharacter(IdString);
}


void UQuestoSubsystem::AppendGibberishForCharacter(const FString& character, const FString& id, const TArray<uint8>& gibberish)
{
	std::string CharacterString(TCHAR_TO_UTF8(*character));
	std::string IdString(TCHAR_TO_UTF8(*id));
	
	std::vector<unsigned char> stdVector;
	stdVector.assign(gibberish.GetData(), gibberish.GetData() + gibberish.Num());
	
	
	mQuestSystem.getActiveQuest()->get().getActiveStage()->get().getActiveSubStage()->get().appendGibberishForCharacter(CharacterString, IdString, stdVector);
	
}


bool UQuestoSubsystem::GetDialogForCharacter(const FString& Id, UPARAM(ref) UUnrealDialog*& OutUnrealDialog)
{
	std::string DialogIdString(TCHAR_TO_UTF8(*Id));
	
	Dialog* dialog = nullptr;
	
	if(mQuestSystem.getActiveQuest()->get().getActiveStage()->get().getActiveSubStage()->get().getDialog(DialogIdString, &dialog)){
		UUnrealDialog* NewDialog = NewObject<UUnrealDialog>(this);
		
		NewDialog->Initialize(*dialog);
		NewDialog->CharacterId = Id;
		OutUnrealDialog = NewDialog;
		
		return true;
		
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve dialog for character %s."), *Id);
		
		return false;
	}
}
