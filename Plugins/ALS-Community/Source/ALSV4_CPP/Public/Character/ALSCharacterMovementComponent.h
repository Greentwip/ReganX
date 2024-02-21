// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Library/ALSCharacterStructLibrary.h"

#include "Character/ALSBaseCharacter.h"

#include "ALSCharacterMovementComponent.generated.h"

/**
 * Authoritative networked Character Movement
 */
UCLASS()
class ALSV4_CPP_API UALSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()


	class ALSV4_CPP_API FSavedMove_My : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
		                        class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Walk Speed Update
		uint8 bSavedRequestMovementSettingsChange : 1;
		EALSGait SavedAllowedGait = EALSGait::Walking;
	};

	class ALSV4_CPP_API FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Movement Settings Override
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	void PhysClimb(float deltaTime, int32 Iterations);

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// Movement Settings Variables
	UPROPERTY()
	uint8 bRequestMovementSettingsChange = 1;

	UPROPERTY()
	EALSGait AllowedGait = EALSGait::Walking;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Movement System")
	FALSMovementSettings CurrentMovementSettings;

	// Set Movement Curve (Called in every instance)
	float GetMappedSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FALSMovementSettings NewMovementSettings);

	// Set Max Walking Speed (Called from the owning client)
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetAllowedGait(EALSGait NewAllowedGait);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetAllowedGait(EALSGait NewAllowedGait);

	// Climb
	UPROPERTY(EditDefaultsOnly) float MaxClimbSpeed = 300.f;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="10.0", ClampMax="2000.0"))
	float MaxClimbingAcceleration = 10;


	UPROPERTY(EditDefaultsOnly) float BrakingDecelerationClimbing = 1000.f;
	UPROPERTY(EditDefaultsOnly) float ClimbReachDistance = 200.f;
	
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="80.0"))
	float DistanceFromSurface = 45.f;
	
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.1", ClampMax="2.0"))
	float ClimbingSnapSpeed = 1.f;
	
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="1.0", ClampMax="60.0"))
	int ClimbingRotationSpeed = 5;


	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="1.0", ClampMax="1000.0"))
	float WallSweepDistance = 40;

	UFUNCTION(BlueprintPure)
	FVector GetClimbSurfaceNormal() const;
	
	float CapR() const;
	float CapHH() const;
	
	UPROPERTY(Transient) AALSBaseCharacter* ZippyCharacterOwner;
protected:
	virtual void InitializeComponent() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	virtual void BeginPlay() override;

	void SweepAndStoreWallHits();
	
	void ComputeSurfaceInfo();
	void ComputeClimbingVelocity(float deltaTime);
	void MoveAlongClimbingSurface(float deltaTime);
	void SnapToClimbingSurface(float deltaTime);
	FQuat GetClimbingRotation(float deltaTime) const;

	
	FVector CurrentClimbingNormal;
	
	FVector CurrentClimbingPosition;
	TArray<FHitResult> CurrentWallHits;
	
	FVector PreviousLocationDelta;

	FCollisionQueryParams ClimbQueryParams;
};
