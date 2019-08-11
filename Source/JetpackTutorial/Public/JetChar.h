// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CoreUObject/Public/UObject/UObjectGlobals.h"
#include "JPGMovementComponent.h"
#include "JetChar.generated.h"

UCLASS()
class JETPACKTUTORIAL_API AJetChar : public ACharacter
{
	GENERATED_BODY()


		UJPGMovementComponent* cachedCMC;
	
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PostInitializeComponents() override;

public:	
	// Sets default values for this character's properties
	AJetChar(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void JumpPressed();
	void JumpReleased();

	void SprintPressed();
	void SprintReleased();

	void GlidePressed();

	void GlideReleased();

	UFUNCTION(BlueprintCallable, BlueprintPure,Category = "Custom")
		FORCEINLINE UJPGMovementComponent* GetJPGMovementComponent() { return cachedCMC; };


	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Rate);
	void LookRight(float Rate);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
		float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
		float BaseLookUpRate = 45.f;


	/** Replicated function sent by client to server - contains client movement and view info. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveExtended(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	void ServerMoveExtended_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	bool ServerMoveExtended_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);

	/**
	 * Replicated function sent by client to server. Saves bandwidth over ServerMoveExtended() by implying that ClientMovementBase and ClientBaseBoneName are null.
	 * Passes through to CharacterMovement->ServerMoveExtended_Implementation() with null base params.
	 */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveExtendedNoBase(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	void ServerMoveExtendedNoBase_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	bool ServerMoveExtendedNoBase_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveExtendedDual(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	void ServerMoveExtendedDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	bool ServerMoveExtendedDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveExtendedDualNoBase(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	void ServerMoveExtendedDualNoBase_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	bool ServerMoveExtendedDualNoBase_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. First move is non root motion, second is root motion. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveExtendedDualHybridRootMotion(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode,FSavedMove_CustomState customState);
	void ServerMoveExtendedDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);
	bool ServerMoveExtendedDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState);

	/* Resending an (important) old move. Process it if not already processed. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveExtendedOld(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState customState);
	void ServerMoveExtendedOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState customState);
	bool ServerMoveExtendedOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState customState);
};
