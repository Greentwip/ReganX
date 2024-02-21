#pragma once

#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum ECustomMantleMovementMode
{
	CMOVE_MantleClimb      	UMETA(DisplayName = "Climbing"),
	CMOVE_MAX				UMETA(Hidden),
};
