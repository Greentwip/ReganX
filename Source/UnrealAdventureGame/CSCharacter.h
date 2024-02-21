// Copyright Epic Games, Inc. All Rights Reserved.

// Base Character from UE5 Third Person Template

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CSCharacter.generated.h"

class UClimbingMovementComponent;
UCLASS(config=Game)
class ACSCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="-360.0", ClampMax="360.0"))
	float WallInclinationAdjustmentPitch = 1.0f;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="-360.0", ClampMax="360.0"))
	float WallInclinationAdjustmentYaw = 0.0f;

	
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="-360.0", ClampMax="360.0"))
	float WallInclinationAdjustmentRoll = 0.0f;

	UFUNCTION(BlueprintPure)
	FORCEINLINE UClimbingMovementComponent* GetCustomCharacterMovement() const { return MovementComponent; }
	
public:
	ACSCharacter(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing")
	float RotationBlend;

private:
	UPROPERTY()
	UClimbingMovementComponent* MovementComponent;

	// Add a member variable to store the current climbing surface normal
	FVector CurrentClimbSurfaceNormal = FVector::ZeroVector;

	// Variables
	float PreviousRoll;
	float ClimbUpValue = 0.0f;

protected:
	void Climb();

	void CancelClimb();
	
	void MoveForward(float Value);
	
	void MoveRight(float Value);
	
	FRotationMatrix GetControlOrientationMatrix() const;

	virtual void Jump() override;
	
	void TurnAtRate(float Rate);
	
	void LookUpAtRate(float Rate);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Climbing")
	class USkeletalMeshComponent* CharacterMesh;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

