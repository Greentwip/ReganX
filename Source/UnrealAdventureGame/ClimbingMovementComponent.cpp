#include "ClimbingMovementComponent.h"

#include "ECustomMovement.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


UClimbingMovementComponent::UClimbingMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UClimbingMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ClimbQueryParams.AddIgnoredActor(GetOwner());
}

void UClimbingMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SweepAndStoreWallHits();
}

void UClimbingMovementComponent::SweepAndStoreWallHits()
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeBox(FVector(CollisionCapsuleRadius, CollisionCapsuleRadius, CollisionCapsuleHalfHeight));
	
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 20;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	
	const FVector End = Start + UpdatedComponent->GetForwardVector() * WallSweepDistance; // Adjusted to use a specified WallSweepDistance
	
	TArray<FHitResult> Hits;
	const bool HitWall = GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
														 ECC_WorldStatic, CollisionShape, ClimbQueryParams);
	
	if(HitWall){
		CurrentWallHits = Hits;
	} else {
		CurrentWallHits.Reset();
	}
}


bool UClimbingMovementComponent::CanStartClimbing()
{
	for (FHitResult& Hit : CurrentWallHits)
	{
		const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D();

		const float HorizontalDot = FVector::DotProduct(UpdatedComponent->GetForwardVector(), -HorizontalNormal);
		const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal);

		const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(HorizontalDot));

		const bool bIsCeiling = FMath::IsNearlyZero(VerticalDot);
		
		if (HorizontalDegrees <= MinHorizontalDegreesToStartClimbing &&
			!bIsCeiling && IsFacingSurface(VerticalDot))
		{
			return true;
		}
	}

	return false;
}

bool UClimbingMovementComponent::IsFacingSurface(const float Steepness) const
{
	constexpr float BaseLength = 80;
	const float SteepnessMultiplier = 1 + (1 - Steepness) * 5;
	
	return EyeHeightTrace(BaseLength * SteepnessMultiplier);
}

bool UClimbingMovementComponent::EyeHeightTrace(const float TraceDistance) const
{
	FHitResult UpperEdgeHit;

	const float BaseEyeHeight = GetCharacterOwner()->BaseEyeHeight;
	const float EyeHeightOffset = IsClimbing() ? BaseEyeHeight + ClimbingCollisionShrinkAmount : BaseEyeHeight;
	
	const FVector Start = UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetUpVector() * EyeHeightOffset;
	const FVector End = Start + (UpdatedComponent->GetForwardVector() * TraceDistance);

	return GetWorld()->LineTraceSingleByChannel(UpperEdgeHit, Start, End, ECC_WorldStatic, ClimbQueryParams);
}

void UClimbingMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (bWantsToClimb)
	{
		SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Climbing);
		bOrientRotationToMovement = false;  // Add this line to disable rotation to movement
	}

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UClimbingMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
	
//		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
//		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight());
	}

	const bool bWasClimbing = PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Climbing;
	if (bWasClimbing)
	{
		bOrientRotationToMovement = true;

		SetRotationToStand();

//		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
//		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight());

		StopMovementImmediately();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UClimbingMovementComponent::SetRotationToStand() const
{
	const FRotator StandRotation = FRotator(0, UpdatedComponent->GetComponentRotation().Yaw, 0);
	UpdatedComponent->SetRelativeRotation(StandRotation);
}

void UClimbingMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == ECustomMovementMode::CMOVE_Climbing)
	{
		if(bIsLedgeClimbing){
			PhysLedgeClimbing(deltaTime, Iterations);
		} else {
			PhysClimbing(deltaTime, Iterations);
		}
	}
	
	Super::PhysCustom(deltaTime, Iterations);
}

void UClimbingMovementComponent::UpdateClimbDashState(float deltaTime)
{
	if (!bIsClimbDashing)
	{
		return;
	}

	CurrentClimbDashTime += deltaTime;

	// Better to cache it when dash starts
	float MinTime, MaxTime;
	ClimbDashCurve->GetTimeRange(MinTime, MaxTime);
	
	if (CurrentClimbDashTime >= MaxTime)
	{
		StopClimbDashing();
	}
}


void UClimbingMovementComponent::PhysLedgeClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	
	// Interpolate between the current location and the target location
	FVector NewLocation = FMath::VInterpTo(LedgeClimbLocation, TargetLedgeLocation, deltaTime, LedgeClimbSpeed);
	
	
	LedgeClimbLocation = NewLocation;
	
	// Check if we've reached the destination
	if ((LedgeClimbLocation - TargetLedgeLocation).IsNearlyZero(0.1f))
	{
		// Set the updated location
		UpdatedComponent->SetWorldLocation(NewLocation);

		// Set bIsLedgeClimbing to false when the ledge climbing is finished
		bIsLedgeClimbing = false;
		
		StopClimbing(deltaTime, Iterations);
	}

	ApplyRootMotionToVelocity(deltaTime);


}

void UClimbingMovementComponent::PhysClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	ComputeSurfaceInfo();
	
	if(ShouldStopClimbing()){
		bIsOnCeiling = true;
	} else {
		bIsOnCeiling = false;
	}
	
	if (ClimbDownToFloor() || bIsClimbCancelled)
	{
		StopClimbing(deltaTime, Iterations);
		return;
	}

	UpdateClimbDashState(deltaTime);

	ComputeClimbingVelocity(deltaTime);

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	
	MoveAlongClimbingSurface(deltaTime);

	TryClimbUpLedge();
	
	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;

	SnapToClimbingSurface(deltaTime);
}

void UClimbingMovementComponent::ComputeSurfaceInfo()
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
		
		// Check if the climbing normal is zero (hitting the ceiling) and set a default downward direction
		if (CurrentClimbingNormal.IsZero())
		{
			CurrentClimbingNormal = PreviousValidClimbingNormal;
		} else {
			PreviousValidClimbingNormal = CurrentClimbingNormal;
		}
	}
	
	if (CurrentClimbingNormal.IsZero())
	{
		CurrentClimbingNormal = PreviousValidClimbingNormal;
	} else {
		PreviousValidClimbingNormal = CurrentClimbingNormal;
	}

}

bool UClimbingMovementComponent::ShouldStopClimbing()
{
	bIsOnCeiling = FVector::Parallel(CurrentClimbingNormal, FVector::UpVector);
	
	return !bWantsToClimb || bIsOnCeiling || CurrentClimbingNormal.IsZero();
}

void UClimbingMovementComponent::StopClimbing(float deltaTime, int32 Iterations)
{
	StopClimbDashing();

	bWantsToClimb = false;
	bIsClimbCancelled = false;
	bIsOnCeiling = false;
	CurrentWallHits.Reset();
	CurrentClimbingNormal = FVector();
	PreviousValidClimbingNormal = FVector();
	TargetLedgeLocation = FVector();

	SetMovementMode(EMovementMode::MOVE_Falling);
	StartNewPhysics(deltaTime, Iterations);
}

bool UClimbingMovementComponent::ClimbDownToFloor() const
{
	FHitResult FloorHit;
	if (!CheckFloor(FloorHit))
	{
		return false;
	}

	const bool bOnWalkableFloor = FloorHit.Normal.Z > GetWalkableFloorZ();
	
	const float DownSpeed = FVector::DotProduct(Velocity, -FloorHit.Normal);
	const bool bIsMovingTowardsFloor = DownSpeed >= MaxClimbingSpeed / 3 && bOnWalkableFloor;
	
	const bool bIsClimbingFloor = CurrentClimbingNormal.Z > GetWalkableFloorZ();
	
	return bIsMovingTowardsFloor || (bIsClimbingFloor && bOnWalkableFloor);
}

bool UClimbingMovementComponent::CheckFloor(FHitResult& FloorHit) const
{
	const FVector Start = UpdatedComponent->GetComponentLocation() + (UpdatedComponent->GetUpVector() * - 20);
	const FVector End = Start + FVector::DownVector * FloorCheckDistance;

	return GetWorld()->LineTraceSingleByChannel(FloorHit, Start, End, ECC_WorldStatic, ClimbQueryParams);
}
bool UClimbingMovementComponent::HasReachedEdge() const
{
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float TraceDistance = Capsule->GetUnscaledCapsuleRadius() * EdgeDetectionOffset;
	
	return !EyeHeightTrace(TraceDistance);
}


bool UClimbingMovementComponent::CanMoveToLedgeClimbLocation() const
{
	if(TargetLedgeLocation.IsZero()){
		return false;
	}

	// Determine the target location
	FVector CheckLocation = TargetLedgeLocation;

	if (!IsLocationWalkable(CheckLocation))
	{
		return false;
	}
	
	return true;
}

bool UClimbingMovementComponent::IsLocationWalkable(const FVector& CheckLocation) const
{
	const float MaxPullUpDistance = PullUpDistance * 2;
	
	FHitResult LedgeHit;
	
	check(PullUpDistance != 0);
	
	for (float Distance = PullUpDistance / 16.0f; Distance <= MaxPullUpDistance; Distance += PullUpDistance / 16.0f)
	{
		const FVector CurrentCheckEnd = CheckLocation + (FVector::DownVector * Distance);
		
		if (GetWorld()->LineTraceSingleByChannel(LedgeHit, CheckLocation, CurrentCheckEnd,
												 ECC_WorldStatic, ClimbQueryParams))
		{
			return LedgeHit.Normal.Z > GetWalkableFloorZ();
		}
	}
	
	return false; // No suitable surface found within the specified range
}

void UClimbingMovementComponent::ComputeClimbingVelocity(float deltaTime)
{
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		if (bIsClimbDashing)
		{
			AlignClimbDashDirection();

			const float CurrentCurveSpeed = ClimbDashCurve->GetFloatValue(CurrentClimbDashTime);
			Velocity = ClimbDashDirection * CurrentCurveSpeed;
		}
		else
		{
			constexpr float Friction = 0.0f;
			constexpr bool bFluid = false;
			CalcVelocity(deltaTime, Friction, bFluid, BrakingDecelerationClimbing);
		}
	}

	ApplyRootMotionToVelocity(deltaTime);
}


void UClimbingMovementComponent::AlignClimbDashDirection()
{
	const FVector HorizontalSurfaceNormal = GetClimbSurfaceNormal();
	
	ClimbDashDirection = FVector::VectorPlaneProject(ClimbDashDirection, HorizontalSurfaceNormal);
}

void UClimbingMovementComponent::StopClimbDashing()
{
	bIsClimbDashing = false;
	CurrentClimbDashTime = 0.f;
	ClimbDashDirection = FVector::ZeroVector;
}

float UClimbingMovementComponent::GetMaxSpeed() const
{
	return IsClimbing() ? MaxClimbingSpeed : Super::GetMaxSpeed();
}

float UClimbingMovementComponent::GetMaxAcceleration() const
{
	return IsClimbing() ? MaxClimbingAcceleration : Super::GetMaxAcceleration();
}

void UClimbingMovementComponent::MoveAlongClimbingSurface(float deltaTime)
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

FQuat UClimbingMovementComponent::GetClimbingRotation(float deltaTime) const
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return Current;
	}
	
	const FQuat Target = FRotationMatrix::MakeFromX(-CurrentClimbingNormal).ToQuat();
	
	const float RotationSpeed = ClimbingRotationSpeed * FMath::Max(1, Velocity.Length() / MaxClimbingSpeed);
	
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

bool UClimbingMovementComponent::TryClimbUpLedge()
{
	const float UpSpeed = FVector::DotProduct(Velocity, UpdatedComponent->GetUpVector());
	const bool bIsMovingUp = UpSpeed >= MaxClimbingSpeed / 3;
	
	LedgeClimbLocation = UpdatedComponent->GetComponentLocation();

	if (bIsMovingUp && HasReachedEdge() && !bIsLedgeTargeted)
	{
		bIsLedgeTargeted = true;
//		SetRotationToStand();
					
		
		// Determine the target location
		TargetLedgeLocation = UpdatedComponent->GetComponentLocation();
		TargetLedgeLocation += UpdatedComponent->GetUpVector() * PullUpDistance;  // Adjust the distance as needed
		TargetLedgeLocation += UpdatedComponent->GetForwardVector() * PullForwardDistance;  // Adjust the distance as needed
	}
	
	if(bIsLedgeTargeted){
		
		if(CanMoveToLedgeClimbLocation()){
			bIsLedgeClimbing = true;
			
			return true;
		}

	}

	return false;
}

void UClimbingMovementComponent::SnapToClimbingSurface(float deltaTime) const
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();
	
	const FVector ForwardDifference = (CurrentClimbingPosition - Location).ProjectOnTo(Forward);
	
	const FVector Offset = -CurrentClimbingNormal * (ForwardDifference.Length() - DistanceFromSurface);

	constexpr bool bSweep = true;

	const float SnapSpeed = ClimbingSnapSpeed * ((Velocity.Length() / MaxClimbingSpeed) + 1);
	UpdatedComponent->MoveComponent(Offset * SnapSpeed * deltaTime, Rotation, bSweep);
}

void UClimbingMovementComponent::TryClimbing()
{
	if (CanStartClimbing())
	{
		bWantsToClimb = true;
	}
}

void UClimbingMovementComponent::TryClimbDashing()
{
	if (ClimbDashCurve && bIsClimbDashing == false)
	{
		bIsClimbDashing = true;
		CurrentClimbDashTime = 0.f;
		
		StoreClimbDashDirection();
	}
}

void UClimbingMovementComponent::StoreClimbDashDirection()
{
	ClimbDashDirection = UpdatedComponent->GetUpVector();

	const float AccelerationThreshold = MaxClimbingAcceleration / 10;
	if (Acceleration.Length() > AccelerationThreshold)
	{
		ClimbDashDirection = Acceleration.GetSafeNormal();
	}
}

void UClimbingMovementComponent::CancelClimbing()
{
	bWantsToClimb = false;
	bIsClimbCancelled = true;
}

FVector UClimbingMovementComponent::GetClimbSurfaceNormal() const
{
	return CurrentClimbingNormal;
}

FVector UClimbingMovementComponent::GetClimbDashDirection() const
{
	return ClimbDashDirection;
}

bool UClimbingMovementComponent::IsClimbing() const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Climbing;
}

bool UClimbingMovementComponent::IsLedgeClimbing() const
{
	return bIsLedgeClimbing;
}

bool UClimbingMovementComponent::IsClimbDashing() const
{
	return IsClimbing() && bIsClimbDashing;
}
