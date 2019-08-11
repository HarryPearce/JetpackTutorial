// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ObjectMacros.h"
#include "Sound/SoundCue.h"
#include "Runtime/Engine/Classes/Engine/NetSerialization.h"
#include "JPGMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_JETPACK = 1,
	CMOVE_GLIDE = 2,
	CMOVE_SPRINT = 3
};

USTRUCT()
struct FSavedMove_CustomState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float savedJetpackResource;
	UPROPERTY()
	float savedDistanceFromGround;
	UPROPERTY()
	float savedDesiredThrottle;
	UPROPERTY()
	bool savedWantsToGlide;
	UPROPERTY()
	bool savedWantsToSprint;
	UPROPERTY()
	bool savedWantsToTeleport;
	UPROPERTY()
	FVector savedTeleportDestination;

	FSavedMove_CustomState() : savedJetpackResource(0.0), savedDistanceFromGround(0.0), savedDesiredThrottle(0.0), savedWantsToGlide(false), savedWantsToSprint(false), savedWantsToTeleport(false), savedTeleportDestination(FVector::ZeroVector) {}

	FSavedMove_CustomState(float jr, float dfg, float dt, bool wtg, bool wts, bool wtt, FVector td) : savedJetpackResource(jr), savedDistanceFromGround(dfg), savedDesiredThrottle(dt), savedWantsToGlide(wtg), savedWantsToSprint(wts), savedWantsToTeleport(wtt), savedTeleportDestination(dt) {}

	void Clear();
	bool IsImportant(const FSavedMove_CustomState &previous) const;
	bool IsDefault() const;


	//bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

//template<>
//struct TStructOpsTypeTraits<FSavedMove_CustomState> : public TStructOpsTypeTraitsBase2<FSavedMove_CustomState>
//{
//	enum
//	{
//		WithNetSerializer = true
//	};
//};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDesiredThrottleDelegate, float, oldThrottle,float,newThrottle);

UCLASS()
class JETPACKTUTORIAL_API UJPGMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

		FRotator previousControlDirection;
	USoundCue* teleportSound;

	UJPGMovementComponent();

	friend class FSavedMove_JPGMovement;

	AJetChar* JetCharOwner;

#pragma region Overrides

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void CallServerMove(const class FSavedMove_Character* NewMove, const class FSavedMove_Character* OldMove);
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const;
	
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual bool IsFalling() const override;
	virtual bool IsMovingOnGround() const override;

	

#pragma region Networking
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
#pragma endregion

#pragma endregion

#pragma region Movement Mode Implementations

	void PhysSprint(float deltaTime, int32 Iterations);
	void PhysJetpack(float deltaTime, int32 Iterations);
	void PhysGlide(float deltaTime, int32 Iterations);

#pragma endregion	

#pragma region Helpers

	void MeasureDistanceFromGround();
	bool IsCustomMovementMode(uint8 cm) const;
	void ProcessTeleport();
	float GetJetpackRechargeAmount(float time);
	void ServerApplyCustomState(float TimeStamp, FSavedMove_CustomState &customState);
	void ApplyCustomState(FSavedMove_CustomState &customState);

	UFUNCTION(NetMulticast, unreliable)
		void MulticastPlayTeleportSound(FVector location);

	FRotator simulationControlRotation;


#pragma endregion

protected:

#pragma region Local State Setters

	void execSetSprinting(bool wantsToSprint);
	void execSetJetpacking(float throttle);
	void execSetGliding(bool wantsToGlide);
	void execSetTeleport(bool wantsToTeleport, FVector destination);

#pragma endregion

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
		FORCEINLINE AJetChar* GetJetCharacterOwner() { return JetCharOwner; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
		TEnumAsByte<ECustomMovementMode> ECustomMovementMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom")
		float DistanceCheckRange = 10000;

#pragma region Jetpack Settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		float JetpackMinDistanceFromGround = 90;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		float JetpackMaxVelocity = 500;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		float JetpackBaseForce = 107800;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		float JetpackMaxDistanceFromGround = 2000;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		float JetpackMaxTime = 10;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		float JetpackFullRechargeSeconds = 10;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		bool DisableLateralFrictionForJetpack = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		UCurveFloat* JetpackHeightToForceMultiplier;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Jetpack")
		UCurveFloat* JetpackVelocityToForceMultiplier;
#pragma endregion

#pragma region Glider Settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderMaxPitchRate = 45;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderMaxYawRate = 45;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderMinDistanceFromGround = 200;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderControlLossVelocity = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderControlLossVelocityRange = 150;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderCancelDistanceFromGround = 25;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderDragFactor = 0.10;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderMaxSpeed = 3000;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Glider")
		float GliderInitialImpulse = 200;
#pragma endregion

#pragma region Sprint Settings	   
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Custom|Sprint")
		float SprintSpeedMultiplier = 1.25;
#pragma endregion

#pragma region State Setters

	UFUNCTION(BlueprintCallable)
		void SetSprinting(bool wantsToSprint);
	UFUNCTION(BlueprintCallable)
		void SetJetpacking(float throttle);
	UFUNCTION(BlueprintCallable)
		void SetGliding(bool wantsToGlide);
	UFUNCTION(BlueprintCallable)
		void SetTeleport(bool wantsToTeleport, FVector destination);

	UPROPERTY(BlueprintAssignable, Category = "Test")
		FDesiredThrottleDelegate OnJetpackingChanged;


#pragma region RPCs

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetSprintingRPC(bool wantsToSprint);
	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetSprintingRPC(bool wantsToSprint);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void MulticastSetSprintingRPC(bool wantsToSprint);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetJetpackingRPC(float throttle);
	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetJetpackingRPC(float throttle);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void MulticastSetJetpackingRPC(float throttle);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetGlidingRPC(bool wantsToGlide);
	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetGlidingRPC(bool wantsToGlide);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void MulticastSetGlidingRPC(bool wantsToGlide);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void MulticastSetControlRotation(FRotator cr);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetTeleportRPC(bool wantsToTeleport, FVector destination);
	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetTeleportRPC(bool wantsToTeleport, FVector destination);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void MulticastSetTeleportRPC(bool wantsToTeleport, FVector destination);

#pragma endregion

#pragma endregion

#pragma region State Queries
	UFUNCTION(BlueprintCallable)
		bool IsSprinting();
	UFUNCTION(BlueprintCallable)
		bool IsJetpacking();
	UFUNCTION(BlueprintCallable)
		bool IsGliding();

#pragma endregion

#pragma region State Conditions

	bool CanJetpack();
	bool CanGlide();
	bool CanSprint();
	bool CanTeleport();

#pragma endregion

#pragma region State Variables

	UFUNCTION()
	void OnRep_ServerCustomState();
	UPROPERTY(ReplicatedUsing=OnRep_ServerCustomState)
	FSavedMove_CustomState lastServerCustomState;	

	UPROPERTY(BlueprintReadOnly, Category = "Custom|State")
		float fEffectiveThrottle;
	UPROPERTY(BlueprintReadOnly, Category = "Custom|State")
		float fJetpackResource;
	UPROPERTY(BlueprintReadOnly, Category = "Custom|State")
		float fDistanceFromGround;
	UPROPERTY(BlueprintReadOnly, Category = "Custom|State")
		float fDesiredThrottle;

	bool bWantsToGlide : 1;
	bool bWantsToSprint : 1;
	bool bWantsToJetpack : 1;
	bool bWantsToTeleport : 1;

	UPROPERTY(BlueprintReadOnly, Category = "Custom|State")
		FVector teleportDestination;

	UPROPERTY(BlueprintReadOnly, Category = "Custom|State")
		float angleOfAttack;

#pragma endregion

	FVector distanceCheckOrigin;

#pragma region ServerMoveExtended

	virtual void ServerMoveExtended(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, const FSavedMove_JPGMovement* customMove);
	virtual void ServerMoveExtended_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState);
	virtual bool ServerMoveExtended_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState);

	/**
	 * Replicated function sent by client to server - contains client movement and view info for two moves.
	 * Calls either CharacterOwner->ServerMoveExtendedDual() or CharacterOwner->ServerMoveExtendedDualNoBase() depending on whehter ClientMovementBase is null.
	 */
	virtual void ServerMoveExtendedDual(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, const FSavedMove_JPGMovement* customMove);
	virtual void ServerMoveExtendedDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState);
	virtual bool ServerMoveExtendedDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. First move is non root motion, second is root motion. */
	virtual void ServerMoveExtendedDualHybridRootMotion(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, const FSavedMove_JPGMovement* customMove);
	virtual void ServerMoveExtendedDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState);
	virtual bool ServerMoveExtendedDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState);

	/* Resending an (important) old move. Process it if not already processed. */
	virtual void ServerMoveExtendedOld(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, const FSavedMove_JPGMovement* customMove);
	virtual void ServerMoveExtendedOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState &customState);
	virtual bool ServerMoveExtendedOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState &customState);

#pragma endregion

};

#pragma region Networking



/** FSavedMove_Character represents a saved move on the client that has been sent to the server and might need to be played back. */
class FSavedMove_JPGMovement : public FSavedMove_Character
{
	friend class UJPGMovementComponent;

public:
	typedef FSavedMove_Character Super;
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;
	virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) override;
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
	FSavedMove_CustomState customState;
	
};

/** Get prediction data for a client game. Should not be used if not running as a client. Allocates the data on demand and can be overridden to allocate a custom override if desired. Result must be a FNetworkPredictionData_Client_Character. */
class FNetworkPredictionData_Client_JPGMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_JPGMovement(const UCharacterMovementComponent& ClientMovement);
	typedef FNetworkPredictionData_Client_Character Super;
	virtual FSavedMovePtr AllocateNewMove() override;
};

#pragma endregion