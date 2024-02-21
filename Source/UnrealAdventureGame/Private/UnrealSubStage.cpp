// UnrealSubStage.cpp

#include "UnrealSubStage.h"

UUnrealSubStage::UUnrealSubStage()
{
}

UUnrealSubStage::UUnrealSubStage(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void UUnrealSubStage::Initialize(SubStage& subStage)
{
	mSubStage = &subStage;
}


void UUnrealSubStage::SetDialogForCharacter(const FString& Id, const FString& Content)
{
	std::string IdString(TCHAR_TO_UTF8(*Id));
	std::string ContentString(TCHAR_TO_UTF8(*Content));

	mSubStage->setDialogString(IdString, ContentString);
}

void UUnrealSubStage::ClearGibberishForCharacter(const FString& Id)
{
	std::string IdString(TCHAR_TO_UTF8(*Id));
		
	mSubStage->clearGibberishForCharacter(IdString);
}


void UUnrealSubStage::AppendGibberishForCharacter(const FString& character, const FString& id, const TArray<uint8>& gibberish)
{
	std::string CharacterString(TCHAR_TO_UTF8(*character));
	std::string IdString(TCHAR_TO_UTF8(*id));

	std::vector<unsigned char> stdVector;
	stdVector.assign(gibberish.GetData(), gibberish.GetData() + gibberish.Num());
	
	mSubStage->appendGibberishForCharacter(CharacterString, IdString, stdVector);
}


bool UUnrealSubStage::GetDialogForCharacter(const FString& Id, UPARAM(ref) UUnrealDialog*& OutUnrealDialog)
{
	std::string DialogIdString(TCHAR_TO_UTF8(*Id));

	Dialog* dialog = nullptr;
	
	if(mSubStage->getDialog(DialogIdString, &dialog)){
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
