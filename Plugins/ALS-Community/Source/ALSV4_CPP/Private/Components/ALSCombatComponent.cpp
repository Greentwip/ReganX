// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Components/ALSCombatComponent.h"

#include "Character/ALSCharacter.h"
#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "Components/ALSDebugComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Library/ALSMathLibrary.h"


const FName NAME_CombatEnd(TEXT("CombatEnd"));
const FName NAME_CombatUpdate(TEXT("CombatUpdate"));
const FName NAME_CombatTimeline(TEXT("CombatTimeline"));

FName UALSCombatComponent::NAME_IgnoreOnlyPawn(TEXT("IgnoreOnlyPawn"));


UALSCombatComponent::UALSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	CombatTimeline = CreateDefaultSubobject<UTimelineComponent>(NAME_CombatTimeline);
}

void UALSCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		OwnerCharacter = Cast<AALSBaseCharacter>(GetOwner());
		
		OwnerMovement = Cast<UALSCharacterMovementComponent>(OwnerCharacter->GetMovementComponent());
		
		if (OwnerCharacter)
		{
			ALSDebugComponent = OwnerCharacter->FindComponentByClass<UALSDebugComponent>();

			AddTickPrerequisiteActor(OwnerCharacter); // Always tick after owner, so we'll use updated values

			// Bindings
			FOnTimelineFloat TimelineUpdated;
			FOnTimelineEvent TimelineFinished;
			TimelineUpdated.BindUFunction(this, NAME_CombatUpdate);
			TimelineFinished.BindUFunction(this, NAME_CombatEnd);
			CombatTimeline->SetTimelineFinishedFunc(TimelineFinished);
			CombatTimeline->SetLooping(false);
			CombatTimeline->SetTimelineLengthMode(TL_TimelineLength);
			CombatTimeline->AddInterpFloat(MantleTimelineCurve, TimelineUpdated);
			
			OwnerCharacter->CombatPressedDelegate.AddUniqueDynamic(this, &UALSCombatComponent::OnOwnerCombatInput);
			
			OwnerCharacter->RagdollStateChangedDelegate.AddUniqueDynamic(
				this, &UALSCombatComponent::OnOwnerRagdollStateChanged);
		}
	}
}


void UALSCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (OwnerCharacter)
	{
		if (OwnerCharacter->GetMovementState() == EALSMovementState::Grounded)
		{
			if (TryCombat())
			{
				
			}
		}
		else if(OwnerCharacter->GetMovementState() == EALSMovementState::Fighting){
			
			// Try disengage
		}
	}
}

bool UALSCombatComponent::TryCombat()
{
	return false;
}


void UALSCombatComponent::CombatStart(EALSCombatType CombatType)
{
	if (OwnerCharacter == nullptr || !IsValid(CombatTimeline))
	{
		return;
	}

	// Step 1: Get the Mantle Asset and use it to set the new Mantle Params.
	const FALSCombatAsset CombatAsset = GetCombatAsset(CombatType, OwnerCharacter->GetOverlayState());
//	check(CombatAsset.PositionCorrectionCurve)

	CombatParams.AnimMontage = CombatAsset.AnimMontage;
//	CombatParams.PositionCorrectionCurve = CombatAsset.PositionCorrectionCurve;
	CombatParams.StartingOffset = CombatAsset.StartingOffset;

	CombatParams.PlayRate = CombatAsset.PlayRate;

	// Step 3: Set the Mantle Target and calculate the Starting Offset
	// (offset amount between the actor and target transform).
//	CombatTarget = MantleLedgeWS.Transform;
//	CombatActualStartOffset = UALSMathLibrary::TransformSub(OwnerCharacter->GetActorTransform(), MantleTarget);

	// Step 4: Calculate the Animated Start Offset from the Target Location.
	// This would be the location the actual animation starts at relative to the Target Transform.
//	FVector RotatedVector = CombatTarget.GetRotation().Vector() * CombatParams.StartingOffset.Y;
//	RotatedVector.Z = MantleParams.StartingOffset.Z;
//	const FTransform StartOffset(MantleTarget.Rotator(), CombatTarget.GetLocation() - RotatedVector,
//	                             FVector::OneVector);
//	MantleAnimatedStartOffset = UALSMathLibrary::TransformSub(StartOffset, CombatTarget);

	// Step 5: Clear the Character Movement Mode and set the Movement State to Mantling
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);	OwnerCharacter->SetMovementState(EALSMovementState::Fighting);

	// Step 6: Configure the Mantle Timeline so that it is the same length as the
	// Lerp/Correction curve minus the starting position, and plays at the same speed as the animation.
	// Then start the timeline.
	float MinTime = 0.0f;
	float MaxTime = 0.0f;
//	CombatParams.PositionCorrectionCurve->GetTimeRange(MinTime, MaxTime);
	CombatTimeline->SetTimelineLength(CombatAsset.AnimMontage->GetPlayLength());
	
	CombatTimeline->SetPlayRate(CombatParams.PlayRate);
	CombatTimeline->PlayFromStart();

	// Step 7: Play the Anim Montage if valid.
	if (CombatParams.AnimMontage && OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(CombatParams.AnimMontage, CombatParams.PlayRate,
			 EMontagePlayReturnType::MontageLength,
			 CombatParams.StartingPosition, true);
	}
}

bool UALSCombatComponent::MantleCheck(const FALSMantleTraceSettings& TraceSettings, EDrawDebugTrace::Type DebugType)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	return true;
}

void UALSCombatComponent::Server_CombatStart_Implementation(EALSCombatType CombatType)
{
	Multicast_CombatStart(CombatType);
}

void UALSCombatComponent::Multicast_CombatStart_Implementation(EALSCombatType CombatType)
{
	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		CombatStart(CombatType);
	}
}

void UALSCombatComponent::CombatUpdate(float BlendIn)
{
	if (!OwnerCharacter)
	{
		return;
	}

}

void UALSCombatComponent::CombatEnd()
{
	// Set the Character Movement Mode to Walking
	if (OwnerCharacter)
	{
		CombatTimeline->Stop();
		
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Stop(0.1f, CombatParams.AnimMontage);
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		OwnerCharacter->SetMovementState(EALSMovementState::Grounded);
		if (OwnerCharacter->IsA(AALSCharacter::StaticClass()))
		{
			Cast<AALSCharacter>(OwnerCharacter)->UpdateHeldObject();
		}
	}
}

void UALSCombatComponent::OnOwnerCombatInput(bool Value)
{
	if(Value){
		CombatStart(EALSCombatType::Melee1);
	} else {
		if (CombatParams.AnimMontage && OwnerCharacter->GetMesh()->GetAnimInstance()) {
			
			CombatEnd();
		}

	}
}

void UALSCombatComponent::OnOwnerRagdollStateChanged(bool bRagdollState)
{
	// If owner is going into ragdoll state, stop combat immediately
	if (bRagdollState)
	{
		CombatTimeline->Stop();
	}
}


void UALSCombatComponent::CancelSpecial()
{
	
	if (OwnerCharacter)
	{
		CombatTimeline->Stop();
		
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Stop(0.1f, CombatParams.AnimMontage);
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		OwnerCharacter->SetMovementState(EALSMovementState::Grounded);
		if (OwnerCharacter->IsA(AALSCharacter::StaticClass()))
		{
			Cast<AALSCharacter>(OwnerCharacter)->UpdateHeldObject();
		}
	}

}



