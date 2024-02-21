// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"

#include "Character/ALSBaseCharacter.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ECustomMantleMovement.h"

#include "ALSCombatComponent.generated.h"

// forward declarations
class UALSDebugComponent;

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UALSCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UALSCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	bool MantleCheck(const FALSMantleTraceSettings& TraceSettings,
	                 EDrawDebugTrace::Type DebugType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	void CombatStart(EALSCombatType MantleType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	void CombatUpdate(float BlendIn);
	
	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	void CombatEnd();

	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	void OnOwnerCombatInput(bool Value);

	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	void OnOwnerRagdollStateChanged(bool bRagdollState);

	/** Implement on BP to get correct mantle parameter set according to character state */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|Combat System")
	FALSCombatAsset GetCombatAsset(EALSCombatType CombatType, EALSOverlayState CurrentOverlayState);
	
	UFUNCTION(BlueprintCallable, Category = "ALS|Combat System")
	void CancelSpecial();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// Called when the game starts
	virtual void BeginPlay() override;

	/** Mantling*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "ALS|Combat System")
	void Server_CombatStart(EALSCombatType MantleType);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "ALS|Combat System")
	void Multicast_CombatStart(EALSCombatType MantleType);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Combat System")
	TObjectPtr<UTimelineComponent> CombatTimeline = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	FALSMantleTraceSettings GroundedTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	FALSMantleTraceSettings AutomaticTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	FALSMantleTraceSettings FallingTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	TObjectPtr<UCurveFloat> MantleTimelineCurve;

	static FName NAME_IgnoreOnlyPawn;
	/** Profile to use to detect objects we allow mantling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	FName MantleObjectDetectionProfile = NAME_IgnoreOnlyPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	TEnumAsByte<ECollisionChannel> WalkableSurfaceDetectionChannel = ECC_Visibility;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Combat System")
	FALSCombatParams CombatParams;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Combat System")
	FALSComponentAndTransform MantleLedgeLS;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Combat System")
	FTransform MantleTarget = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Combat System")
	FTransform MantleActualStartOffset = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Combat System")
	FTransform MantleAnimatedStartOffset = FTransform::Identity;

	/** If a dynamic object has a velocity bigger than this value, do not start mantle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	float AcceptableVelocityWhileMantling = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Combat System")
	float TimeToClimb = 1.0f;

private:
	bool TryCombat();
	
private:
	UPROPERTY()
	TObjectPtr<AALSBaseCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UALSCharacterMovementComponent> OwnerMovement;

	UPROPERTY()
	TObjectPtr<UALSDebugComponent> ALSDebugComponent = nullptr;
};
