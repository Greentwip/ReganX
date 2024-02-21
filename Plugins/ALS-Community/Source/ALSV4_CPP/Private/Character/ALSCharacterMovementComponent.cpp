// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"

#include "ECustomMantleMovement.h"

#include "Curves/CurveVector.h"

UALSCharacterMovementComponent::UALSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UALSCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	ZippyCharacterOwner = Cast<AALSBaseCharacter>(GetOwner());
}

void UALSCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                       const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	if (bRequestMovementSettingsChange)
	{
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;

		bRequestMovementSettingsChange = false;
	}
}


void UALSCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
	
	switch (CustomMovementMode)
	{
		case CMOVE_MantleClimb:
			PhysClimb(deltaTime, Iterations);
			break;
		default:
			UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}
void UALSCharacterMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	ComputeSurfaceInfo();

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}
	
	// Perform the move
	bJustTeleported = false;
	Iterations++;
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	
	
	// Transform Acceleration
//	Acceleration.Z = 0.f;
//	Acceleration = Acceleration.RotateAngleAxis(90.f, -UpdatedComponent->GetRightVector());
	
	// Apply acceleration
//	CalcVelocity(deltaTime, 0.f, false, GetMaxBrakingDeceleration());
//	Velocity = FVector::VectorPlaneProject(Velocity, SurfHit.Normal);
	
//	// Compute move parameters
//	const FVector Delta = deltaTime * Velocity; // dx = v * dt
//	if (!Delta.IsNearlyZero())
//	{
//		FHitResult Hit;
//		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
//		FVector WallAttractionDelta = -SurfHit.Normal * WallAttractionForce * deltaTime;
//		SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
//	}
	
	ComputeClimbingVelocity(deltaTime);
	
	MoveAlongClimbingSurface(deltaTime);

	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;

	SnapToClimbingSurface(deltaTime);
	
	
//	FHitResult SurfHit;
//	GetWorld()->LineTraceSingleByProfile(SurfHit, OldLocation, OldLocation + UpdatedComponent->GetForwardVector() * ClimbReachDistance, "BlockAll", ZippyCharacterOwner->GetIgnoreCharacterParams());
	if (CurrentClimbingNormal.IsZero())
	{
		ZippyCharacterOwner->SetMovementState(EALSMovementState::InAir);
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	FHitResult FloorHit;

	const bool bHit = GetWorld()->LineTraceSingleByProfile(FloorHit, OldLocation, OldLocation + FVector::DownVector * CapHH() * 1.02f, "BlockAll", ZippyCharacterOwner->GetIgnoreCharacterParams());

	if(bHit){
		ZippyCharacterOwner->SetMovementState(EALSMovementState::Grounded);
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
	}
}

void UALSCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if(ZippyCharacterOwner->GetMovementState() == EALSMovementState::Climbing){
		
		SetMovementMode(MOVE_Custom, CMOVE_MantleClimb);

		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UALSCharacterMovementComponent::GetMaxSpeed() const
{
	if(ZippyCharacterOwner->GetMovementState() == EALSMovementState::Climbing){
		return MaxClimbSpeed;
	} else {
		return Super::GetMaxSpeed();
	}
}

float UALSCharacterMovementComponent::GetMaxAcceleration() const
{
	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		if(ZippyCharacterOwner->GetMovementState() == EALSMovementState::Climbing){
			return MaxClimbingAcceleration;
		} else {
			return Super::GetMaxAcceleration();
		}
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

float UALSCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		if(ZippyCharacterOwner->GetMovementState() == EALSMovementState::Climbing){
			return BrakingDecelerationClimbing;
		} else {
			return Super::GetMaxBrakingDeceleration();
		}
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

void UALSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

class FNetworkPredictionData_Client* UALSCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UALSCharacterMovementComponent* MutableThis = const_cast<UALSCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UALSCharacterMovementComponent::FSavedMove_My::Clear()
{
	Super::Clear();

	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EALSGait::Walking;
}

uint8 UALSCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UALSCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                               FVector const& NewAccel,
                                                               class FNetworkPredictionData_Client_Character&
                                                               ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;
	}
}

void UALSCharacterMovementComponent::FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;
	}
}

UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return MakeShared<FSavedMove_My>();
}

void UALSCharacterMovementComponent::Server_SetAllowedGait_Implementation(const EALSGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}

float UALSCharacterMovementComponent::GetMappedSpeed() const
{
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows us to vary the movement speeds but still use the mapped range in calculations for consistent results

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UALSCharacterMovementComponent::SetMovementSettings(FALSMovementSettings NewMovementSettings)
{
	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UALSCharacterMovementComponent::SetAllowedGait(EALSGait NewAllowedGait)
{
	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}

float UALSCharacterMovementComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UALSCharacterMovementComponent::CapHH() const
{
	return ZippyCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UALSCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ClimbQueryParams.AddIgnoredActor(GetOwner());
}

void UALSCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
											   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SweepAndStoreWallHits();
}

void UALSCharacterMovementComponent::SweepAndStoreWallHits()
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapR(), CapHH());
	
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 20;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	
	const FVector End = Start + UpdatedComponent->GetForwardVector();
	
	FHitResult Hits;
	const bool HitWall = GetWorld()->SweepSingleByChannel(Hits, Start, End, FQuat::Identity,
														 ECC_WorldStatic, CollisionShape, ClimbQueryParams);
	
	if(HitWall){
		CurrentWallHits = {Hits};
	} else {
		CurrentWallHits.Reset();
	}
}

void UALSCharacterMovementComponent::ComputeSurfaceInfo()
{
	CurrentClimbingNormal = FVector::ZeroVector;
	CurrentClimbingPosition = FVector::ZeroVector;
	
	if (CurrentWallHits.Num() > 0)
	{
		for (const FHitResult& WallHit : CurrentWallHits)
		{
			const FVector HorizontalNormal = WallHit.Normal.GetSafeNormal2D();
			CurrentClimbingNormal += HorizontalNormal;
			CurrentClimbingPosition += WallHit.ImpactPoint;
		}
		
		const float InvNumHits = 1.0f / CurrentWallHits.Num();
		CurrentClimbingNormal *= InvNumHits;
		CurrentClimbingPosition *= InvNumHits;
	}
}

FVector UALSCharacterMovementComponent::GetClimbSurfaceNormal() const
{
	return CurrentClimbingNormal;
}


void UALSCharacterMovementComponent::ComputeClimbingVelocity(float deltaTime)
{
	RestorePreAdditiveRootMotionVelocity();
	
	constexpr float Friction = 0.0f;
	constexpr bool bFluid = false;
	CalcVelocity(deltaTime, Friction, bFluid, BrakingDecelerationClimbing);

	ApplyRootMotionToVelocity(deltaTime);
}


void UALSCharacterMovementComponent::MoveAlongClimbingSurface(float deltaTime)
{
	const FVector Adjusted = Velocity * deltaTime;
	
	FHitResult Hit(1.f);
	
	SafeMoveUpdatedComponent(Adjusted, GetClimbingRotation(deltaTime), true, Hit);
	
	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
}

void UALSCharacterMovementComponent::SnapToClimbingSurface(float deltaTime)
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();
	
	const FVector ForwardDifference = (CurrentClimbingPosition - Location).ProjectOnTo(Forward);
	
	const FVector LocationDelta = -CurrentClimbingNormal * (ForwardDifference.Length() - DistanceFromSurface);
	
	constexpr bool bSweep = true;
	constexpr float SnapResetThresholdSquared = 600.0f;  // Adjust the value as needed
	
	// Move to the interpolated location
	UpdatedComponent->MoveComponent(LocationDelta, Rotation, bSweep);
}


FQuat UALSCharacterMovementComponent::GetClimbingRotation(float deltaTime) const
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();
	
	const FQuat Target = FRotationMatrix::MakeFromX(-CurrentClimbingNormal).ToQuat();
	
	const float RotationSpeed = ClimbingRotationSpeed;
	
	// Calculate the angle between current and target rotations
	const float Angle = Target.AngularDistance(Current);
	const float Tolerance = 0.7f; // Adjust the tolerance as needed
	
	// If the angle is small enough, skip interpolation
	if (FMath::IsNearlyZero(Angle, Tolerance))
	{
		return Current;
	}
	
	// Calculate the target rotation using Slerp for a smoother interpolation
	FQuat LerpedRotation = FQuat::Slerp(Current, Target, FMath::Clamp(RotationSpeed * deltaTime, 0.0f, 1.0f));
	
	return LerpedRotation;
}
