#include "UnrealDialog.h"

UUnrealDialog::UUnrealDialog()
{
	// Default constructor implementation
	// Make sure to initialize any variables here if needed
}

UUnrealDialog::UUnrealDialog(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Constructor with FObjectInitializer implementation
	// Make sure to initialize any variables here if needed
}

void UUnrealDialog::Initialize(Dialog& dialog)
{
	mDialog = &dialog;
}

FString UUnrealDialog::GetString(){
	return FString(UTF8_TO_TCHAR(mDialog->getString().c_str()));
}

TArray<uint8> UUnrealDialog::GetGibberish(){
	const std::vector<unsigned char>& gibberishVector = mDialog->getGibberish();
	
	// Convert the std::vector to TArray<uint8>
	TArray<uint8> gibberishArray;
	gibberishArray.SetNumUninitialized(gibberishVector.size());
	
	FMemory::Memcpy(gibberishArray.GetData(), gibberishVector.data(), gibberishVector.size());
	
	return gibberishArray;
}
