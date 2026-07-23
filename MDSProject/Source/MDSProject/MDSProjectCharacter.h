// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MDSProjectCharacter.generated.h"

/**
 *  A controllable top-down perspective character
 */
UCLASS(abstract)
class AMDSProjectCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:

	/** Constructor */
	AMDSProjectCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	void RequestLocalAttackPresentation(FName PresentationSource);
	void ApplyMovementInput(const FVector2D& MovementInput);
	void BeginTemporaryFireFacing(const FVector& AimDirection, float DurationSeconds);
	void PlayShotTracerPresentation(const FVector& TraceEnd);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayRemoteAttackPresentation(
		FName PresentationSource,
		FVector_NetQuantizeNormal AimDirection,
		FVector_NetQuantize TraceEnd,
		float DurationSeconds);

	/** Returns the camera component **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns the Camera Boom component **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "MDS|Combat Presentation")
	void BP_OnLocalAttackPresentationRequested(FName PresentationSource);

private:
	UPROPERTY(EditDefaultsOnly, Category = "MDS|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	void OnMoveInput(const struct FInputActionValue& Value);
	void PlayAttackMontagePresentation(FName PresentationSource);
	void RestoreMovementFacing();

	FVector MovementVerificationStartLocation = FVector::ZeroVector;
	FVector LockedFireFacingDirection = FVector::ForwardVector;
	double LastMovementVerificationLogTimeSeconds = -1000000.0;
	bool bMovementVerificationInitialized = false;
	bool bFireFacingLocked = false;
	FTimerHandle FireFacingTimerHandle;
};

