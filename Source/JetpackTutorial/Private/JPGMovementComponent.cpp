// Fill out your copyright notice in the Description page of Project Settings.


#include "JPGMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Classes/Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundCue.h"
#include "ConstructorHelpers.h"
#include "Engine/Classes/GameFramework/PlayerController.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/Classes/GameFramework/Controller.h"
#include "JetChar.h"
#include "UnrealNetwork.h"

UJPGMovementComponent::UJPGMovementComponent()
{
	fJetpackResource = 1.0;
	bWantsToGlide = false;
	fDesiredThrottle = 0.0;
	static ConstructorHelpers::FObjectFinder<USoundCue> ts(TEXT("'/Game/ThirdPersonBP/Blueprints/Enderman_teleport_Cue.Enderman_teleport_Cue'"));
	teleportSound = ts.Object;
	JetCharOwner = Cast<AJetChar>(GetOwner());
}

#pragma region Movement Mode Implementations

void UJPGMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("PhysCustom"), true, false, FLinearColor::Blue, 0.0);

	if (CustomMovementMode == ECustomMovementMode::CMOVE_JETPACK)
	{
		PhysJetpack(deltaTime, Iterations);
	}
	if (CustomMovementMode == ECustomMovementMode::CMOVE_GLIDE)
	{
		PhysGlide(deltaTime, Iterations);
	}
	if (CustomMovementMode == ECustomMovementMode::CMOVE_SPRINT)
	{
		PhysSprint(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

void UJPGMovementComponent::PhysSprint(float deltaTime, int32 Iterations)
{
	UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("PhysSprint"), true, false, FLinearColor::Red, 0.0);
	if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_SPRINT))
	{
		SetSprinting(false);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	if (!bWantsToSprint)
	{

		//SetSprinting(false);
		UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("SetMovementMode Walk"), true, true, FLinearColor::Red, 10);
		SetMovementMode(EMovementMode::MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	PhysWalking(deltaTime, Iterations);
}

void UJPGMovementComponent::PhysJetpack(float deltaTime, int32 Iterations)
{
	//UKismetSystemLibrary::PrintString(GetWorld(),GetOwner()->GetName() +   FString("PhysJetpack"), true, false, FLinearColor::Blue, 0.0);
	if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_JETPACK))
	{
		SetJetpacking(0.0);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	if (!bWantsToJetpack ||
		fDistanceFromGround <= 0 ||
		fJetpackResource <= (deltaTime / JetpackMaxTime))
	{
		SetMovementMode(EMovementMode::MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

#pragma region Print Data

	if (GetPawnOwner()->IsLocallyControlled())
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString("Speed: ") + FString::SanitizeFloat(Velocity.Size()), true, false, FLinearColor::Red, 0.0);
		UKismetSystemLibrary::PrintString(GetWorld(), FString("Distance from Ground: ") + FString::SanitizeFloat(fDistanceFromGround), true, false, FLinearColor::Red, 0.0);
		UKismetSystemLibrary::PrintString(GetWorld(), FString("Resource: ") + FString::SanitizeFloat(fJetpackResource), true, false, FLinearColor::Red, 0.0);
	}

#pragma endregion

	float jetpackAcceleration = JetpackBaseForce / Mass;
	float curveMultiplier = 1.0;
	if (JetpackHeightToForceMultiplier)
		curveMultiplier = JetpackHeightToForceMultiplier->GetFloatValue(fDistanceFromGround);
	if (JetpackVelocityToForceMultiplier)
		curveMultiplier = FMath::Max(curveMultiplier, JetpackVelocityToForceMultiplier->GetFloatValue(Velocity.Z));

	jetpackAcceleration *= curveMultiplier;

	float jetpackSurplusAccel = FMath::Max<float>(0, jetpackAcceleration + GetGravityZ());
	float desiredSurplusJetpackAccel = jetpackSurplusAccel * fDesiredThrottle;
	float desiredTotalJetpackAccel = (GetGravityZ() * -1) + desiredSurplusJetpackAccel;

	float totalDesiredVelocity = Velocity.Z + (desiredTotalJetpackAccel * deltaTime);
	float velLimitWGravCounteract = JetpackMaxVelocity + (GetGravityZ() * deltaTime * -1);
	float resultingAccel = 0.0;

	float deltaFromGround = JetpackMaxDistanceFromGround - fDistanceFromGround;

	if (Velocity.Z > velLimitWGravCounteract)
	{
		resultingAccel = 0.0f;
	}
	else
	{
		if (totalDesiredVelocity > velLimitWGravCounteract)
		{
			float velLimitClampAmount = totalDesiredVelocity - velLimitWGravCounteract;
			resultingAccel = FMath::Clamp<float>(desiredTotalJetpackAccel - (velLimitClampAmount / deltaTime), 0, desiredTotalJetpackAccel);
		}
		else
		{
			resultingAccel = desiredTotalJetpackAccel;
		}
	}

	float intermediaryVelocity = Velocity.Z + resultingAccel * deltaTime;
	float distanceWithIntermediareVelocity = intermediaryVelocity * deltaTime;

	if (deltaFromGround > 0)
	{
		if (intermediaryVelocity < 0)
		{

		}
		else
		{
			float stopTime = intermediaryVelocity / (GetGravityZ() * -1);
			float timeToCrossDelta = deltaFromGround / intermediaryVelocity;
			if (stopTime >= timeToCrossDelta)
			{
				float timeClampAmount = stopTime - timeToCrossDelta;
				resultingAccel -= FMath::Clamp<float>(deltaFromGround / (timeClampAmount* timeClampAmount), 0, resultingAccel);
			}
			else
			{

			}
		}
	}
	else
	{

		if (Velocity.Z > 0)
		{
			resultingAccel = 0.0f;
		}
		else
		{

			float stopTime = ((Velocity.Z + (GetGravityZ() * deltaTime))*-1) / desiredSurplusJetpackAccel;
			float timeToCrossDelta = deltaFromGround / (Velocity.Z + GetGravityZ() * deltaTime);
			if (stopTime > timeToCrossDelta)
			{
				float timeClampAmount = stopTime - timeToCrossDelta;
				resultingAccel += FMath::Clamp<float>(deltaFromGround / (timeClampAmount* timeClampAmount), 0, desiredSurplusJetpackAccel - resultingAccel);
			}
			else
			{
				resultingAccel = 0.0f;
			}
		}
	}

	fEffectiveThrottle = resultingAccel / (JetpackBaseForce / Mass);
	Velocity.Z += resultingAccel * deltaTime;

	if (DisableLateralFrictionForJetpack)
	{
		float oldFallingLateralFriction = FallingLateralFriction;
		FallingLateralFriction = 0;
		PhysFalling(deltaTime, Iterations);
		FallingLateralFriction = oldFallingLateralFriction;
	}
	else
	{
		PhysFalling(deltaTime, Iterations);
	}

	fJetpackResource = FMath::Clamp<float>(fJetpackResource - (deltaTime / JetpackMaxTime)*fEffectiveThrottle, 0, 1);
}

void UJPGMovementComponent::PhysGlide(float deltaTime, int32 Iterations)
{
	if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_GLIDE))
	{
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	//airspeed
	float velSize = Velocity.Size();
	FRotator controlDirection = GetPawnOwner()->GetControlRotation();

	//if we do not want to glide anymore, or we are too low, cancel gliding mode
	if (!bWantsToGlide || fDistanceFromGround < GliderCancelDistanceFromGround)
	{
		//SetGliding(false);
		SetMovementMode(EMovementMode::MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		FHitResult out;
		SafeMoveUpdatedComponent(Velocity * deltaTime, GetOwner()->GetActorRotation(), true, out);
		return;
	}

	FRotator controlDirectionDelta = controlDirection - previousControlDirection;

	//normalize shifts angles to -180 to 180 range, pitch of control direction cannot exceed this range, it is locked in that range and cannot rollover
	controlDirectionDelta.Normalize();

	//at 1 we have full control, at 0 we have no control
	float velocityControlMultiplier = FMath::Clamp<float>((velSize - GliderControlLossVelocity) / GliderControlLossVelocityRange, 0, 1);
	//the slower we are, the more gravity forces us to turn downwards
	float pitchLoss = (deltaTime * 180 * (1 - FMath::InterpEaseOut<float>(0, 1, velocityControlMultiplier, 2)));

	controlDirectionDelta = FRotator(
		FMath::Clamp<float>(controlDirectionDelta.Pitch + pitchLoss, deltaTime * GliderMaxPitchRate * -1, deltaTime * GliderMaxPitchRate),
		FMath::Clamp<float>(controlDirectionDelta.Yaw, deltaTime * GliderMaxYawRate * -1 * velocityControlMultiplier, deltaTime * GliderMaxYawRate * velocityControlMultiplier),
		controlDirectionDelta.Roll
	);

	controlDirection = previousControlDirection + controlDirectionDelta;
	/*if (controlDirectionDelta != FRotator::ZeroRotator)
	{
		MulticastSetControlRotation(controlDirection);
	}*/

	//clamp shifts angle to 0-360 range, for some reason SetControlRotation prefers it this way
	if (GetPawnOwner()->Controller)
		GetPawnOwner()->Controller->SetControlRotation(controlDirection.Clamp());

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		//controlDirection = simulationControlRotation;
	}

	//pitch is flipped to invert vertical mouselook, and is rotated by further 90 degrees, because we are parallel to the ground when we glide
	FRotator pitchFlippedControlDirection = FRotator((controlDirection.Pitch * -1) - 90, controlDirection.Yaw, controlDirection.Roll).Clamp();

	// 1 is down, 0 is parallel, -1 is up
	angleOfAttack = (FMath::Abs(pitchFlippedControlDirection.GetNormalized().Pitch) / 90) - 1;

	float dragMultiplier = 1 - (GliderDragFactor * deltaTime);
	float gravityVelocity = angleOfAttack * GetGravityZ() * deltaTime;

	//shift velocity heading to upward vector of actor, ie directon of flight defined by control direction
	Velocity = FMath::Max<float>((velSize - gravityVelocity) * dragMultiplier, 0) * FRotator(pitchFlippedControlDirection).Add(90, 0, 0).Vector().GetSafeNormal();

	FHitResult out;
	SafeMoveUpdatedComponent(Velocity * deltaTime, pitchFlippedControlDirection.Quaternion(), true, out);

	previousControlDirection = controlDirection;

	if (out.bBlockingHit)
	{
		SetMovementMode(EMovementMode::MOVE_Falling);
	}
}

#pragma endregion

#pragma region Overrides

void UJPGMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("bwantsToSprint ") + FString(bWantsToSprint ? "true" : "false"), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("TickComponent"), true, false, FLinearColor::Red, 0.0);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//recharge jetpack resource
	if (MovementMode != EMovementMode::MOVE_Custom || CustomMovementMode != ECustomMovementMode::CMOVE_JETPACK)
	{
		fJetpackResource = FMath::Clamp<float>(fJetpackResource + GetJetpackRechargeAmount(DeltaTime), 0, 1);
	}
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("fJetpackResource ") + FString::SanitizeFloat(fJetpackResource), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("fDesiredThrottle ") + FString::SanitizeFloat(fDesiredThrottle), true, false, FLinearColor::Red, 0.0);

#pragma region Debug Prints
	//FVector normVel = FVector(Velocity);
	//normVel.Normalize();
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Distance from ground: ") + FString::SanitizeFloat(fDistanceFromGround), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Acceleration: ") + Velocity.ToString(), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Up: ") + GetOwner()->GetActorUpVector().ToString(), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Fwd: ") + GetOwner()->GetActorForwardVector().ToString(), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("NormVel: ") + normVel.ToString(), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("VelLen: ") + FString::SanitizeFloat(Velocity.Size()), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Effective Throttle: ") + FString::SanitizeFloat(fEffectiveThrottle), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Desired Throttle ") + FString::SanitizeFloat(fDesiredThrottle), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Jetpack Resource: ") + FString::SanitizeFloat(fJetpackResource), true, false, FLinearColor::Red, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("Control Direction: ") + GetPawnOwner()->GetControlRotation().ToString(), true, false, FLinearColor::Green, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("RotatedUpVecotr: ") + GetOwner()->GetActorRotation().UnrotateVector(GetOwner()->GetActorUpVector()).ToString(), true, false, FLinearColor::Green, 0.0);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("AR: ") + FRotator(GetOwner()->GetActorRotation()).Clamp().ToString(), true, false, FLinearColor::Green, 0.0);

#pragma endregion
}

void UJPGMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("OnMovementUpdated"), true, false, FLinearColor::Red, 0.0);
	MeasureDistanceFromGround();

	if (bWantsToSprint)
	{
		if (CanSprint())
		{

			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_SPRINT);
		}
		/*else if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_SPRINT))
		{
			SetSprinting(false);
		}*/
	}

	if (bWantsToJetpack)
	{
		if (CanJetpack())
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_JETPACK);
		}
		/*else if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_JETPACK))
		{
			SetJetpacking(0);
		}*/
	}

	if (bWantsToGlide)
	{
		if (CanGlide())
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_GLIDE);
		}
		else if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_GLIDE))
		{
			SetGliding(false);
		}
	}

	if (bWantsToTeleport)
	{
		if (CanTeleport())
		{
			ProcessTeleport();
		}
		else
		{
			SetTeleport(false, FVector::ZeroVector);
		}

	}

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UJPGMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("OnMovementModeChanged "), true, true, FLinearColor::Red, 10);
	bool suppressSuperNotification = false;

	if (PreviousMovementMode == MovementMode && PreviousCustomMode == CustomMovementMode)
	{
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
		return;
	}

#pragma region Leaving State Handlers

	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == ECustomMovementMode::CMOVE_SPRINT)
	{

		SetSprinting(false);
		MaxWalkSpeed /= SprintSpeedMultiplier;
	}
	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == ECustomMovementMode::CMOVE_JETPACK)
	{
		SetJetpacking(0);
		fEffectiveThrottle = 0;
	}
	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == ECustomMovementMode::CMOVE_GLIDE)
	{
		SetGliding(false);

		if (GetPawnOwner() && GetPawnOwner()->Controller)
		{
			FRotator controlDirection = GetPawnOwner()->GetControlRotation();
			GetPawnOwner()->Controller->SetControlRotation(FRotator(controlDirection.GetNormalized().Pitch * -1, controlDirection.Yaw, controlDirection.Roll).Clamp());
		}
		GetOwner()->SetActorRotation(FRotator(0, GetOwner()->GetActorRotation().Yaw, 0));
	}

#pragma endregion

#pragma region Entering State Handlers

	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_SPRINT))
	{

		MaxWalkSpeed *= SprintSpeedMultiplier;
		suppressSuperNotification = true;
	}
	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_JETPACK))
	{

	}
	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_GLIDE))
	{
		if (GetOwner())
		{
			Velocity += GetOwner()->GetActorForwardVector() * GliderInitialImpulse;
		}

		if (GetPawnOwner() && GetPawnOwner()->Controller)
		{
			FRotator currentControlRotation = GetPawnOwner()->GetControlRotation();
			currentControlRotation.GetNormalized();
			GetPawnOwner()->Controller->SetControlRotation(FRotator(currentControlRotation.Pitch * -1, currentControlRotation.Yaw, currentControlRotation.Roll).Clamp());
		}
		previousControlDirection = GetPawnOwner()->GetControlRotation();
	}

#pragma endregion
	if (!suppressSuperNotification)
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	else
		CharacterOwner->OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

float UJPGMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_GLIDE)
	{
		return GliderMaxSpeed;
	}

	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_SPRINT))
	{
		return Super::GetMaxSpeed() * SprintSpeedMultiplier;
	}

	return Super::GetMaxSpeed();
}

float UJPGMovementComponent::GetMaxAcceleration() const
{
	return Super::GetMaxAcceleration();
}

bool UJPGMovementComponent::IsFalling() const
{
	return Super::IsFalling() || IsCustomMovementMode(ECustomMovementMode::CMOVE_JETPACK);
}

bool UJPGMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || (IsCustomMovementMode((uint8)ECustomMovementMode::CMOVE_SPRINT) && UpdatedComponent);
}

#pragma endregion

#pragma region Helpers

bool UJPGMovementComponent::IsCustomMovementMode(uint8 cm) const
{
	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == cm)
		return true;
	return false;
}

void UJPGMovementComponent::MeasureDistanceFromGround()
{
	//Don't measure distance for actors, that are not controlled by you if you are a client
	/*if (!GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
		return;*/

	FHitResult res;
	FVector start = GetOwner()->GetActorLocation() + distanceCheckOrigin;

	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(),
		start,
		(FVector(0, 0, -1) * DistanceCheckRange) + start,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::ForOneFrame,
		res,
		true

	) && res.bBlockingHit)
	{
		fDistanceFromGround = res.Distance;
	}
	else
	{
		fDistanceFromGround = DistanceCheckRange;
	}
}

void UJPGMovementComponent::ProcessTeleport()
{
	UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ProcessTeleport"), true, true, FLinearColor::Red, 10);
	FHitResult res;

	SafeMoveUpdatedComponent(teleportDestination - GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), false, res, ETeleportType::TeleportPhysics);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), teleportSound, teleportDestination);

	execSetTeleport(false, FVector::ZeroVector);
}

float UJPGMovementComponent::GetJetpackRechargeAmount(float time)
{

	if (MovementMode != EMovementMode::MOVE_Custom || CustomMovementMode != ECustomMovementMode::CMOVE_JETPACK)
	{
		return time / JetpackFullRechargeSeconds;
	}
	return 0.0;
}

void UJPGMovementComponent::ServerApplyCustomState(float TimeStamp, FSavedMove_CustomState &customState)
{
	if (!HasValidData() || !IsActive())
	{
		return;
	}

	FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
	check(ServerData);

	if (!VerifyClientTimeStamp(TimeStamp, *ServerData))
	{
		UE_LOG(LogNetPlayerMovement, Log, TEXT("ServerMove: TimeStamp expired. %f, CurrentTimeStamp: %f"), TimeStamp, ServerData->CurrentClientTimeStamp);
		return;
	}
	ApplyCustomState(customState);
	lastServerCustomState = customState;	
}

void UJPGMovementComponent::ApplyCustomState(FSavedMove_CustomState & customState)
{
	fJetpackResource = customState.savedJetpackResource;
	fDistanceFromGround = customState.savedDistanceFromGround;
	execSetJetpacking(customState.savedDesiredThrottle);
	bWantsToSprint = customState.savedWantsToSprint;
	bWantsToGlide = customState.savedWantsToGlide;
	execSetTeleport(customState.savedWantsToTeleport, customState.savedTeleportDestination);
}

void UJPGMovementComponent::MulticastPlayTeleportSound_Implementation(FVector location)
{
	UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("MulticastPlayTeleportSound_Implementation"), true, true, FLinearColor::Red, 10);
	//this should only execute on proxies
	if (!GetPawnOwner()->IsLocallyControlled())
	{
		UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("MC_PlaySoundAtLocation"), true, true, FLinearColor::Red, 10);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), teleportSound, location);
	}
}
#pragma endregion

#pragma region State Setters

void UJPGMovementComponent::SetJetpacking(float throttle)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("SetJetpacking"), true, true, FLinearColor::Red, 10);
	if (throttle != fDesiredThrottle)
	{
		execSetJetpacking(throttle);
		return;
#pragma region Networking

		if (!GetOwner() || !GetPawnOwner())
			return;

		if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
		{
			//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ServerSetJetpackingRPC"), true, true, FLinearColor::Green, 10);
			ServerSetJetpackingRPC(throttle);
		}
		else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
		{
			//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ClientSetJetpackingRPC"), true, true, FLinearColor::Black, 10);
			ClientSetJetpackingRPC(throttle);
		}

#pragma endregion

	}
}

void UJPGMovementComponent::SetGliding(bool wantsToGlide)
{
	if (bWantsToGlide != wantsToGlide)
	{
		execSetGliding(wantsToGlide);
		return;
#pragma region Networking

		if (!GetOwner() || !GetPawnOwner())
			return;

		if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
		{
			ServerSetGlidingRPC(wantsToGlide);
		}
		else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
		{
			ClientSetGlidingRPC(wantsToGlide);
		}

#pragma endregion

	}
}

void UJPGMovementComponent::SetTeleport(bool wantsToTeleport, FVector destination)
{
	UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("SetTeleport"), true, true, FLinearColor::Red, 10);
	if (bWantsToTeleport != wantsToTeleport || teleportDestination != destination)
	{
		execSetTeleport(wantsToTeleport, destination);
		return;
#pragma region Networking

		if (!GetOwner() || !GetPawnOwner())
			return;

		if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
		{
			UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ServerSetTeleportRPC"), true, true, FLinearColor::Green, 10);
			ServerSetTeleportRPC(wantsToTeleport, destination);
		}
		else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
		{
			UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ClientSetTeleportRPC"), true, true, FLinearColor::Black, 10);
			ClientSetTeleportRPC(wantsToTeleport, destination);
		}

#pragma endregion

	}
}

void UJPGMovementComponent::SetSprinting(bool wantsToSprint)
{

	if (bWantsToSprint != wantsToSprint)
	{
		execSetSprinting(wantsToSprint);
		return;
#pragma region Networking

		if (!GetOwner() || !GetPawnOwner())
			return;

		if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
		{
			//ServerSetSprintingRPC(wantsToSprint);
		}
		else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
		{
			//ClientSetSprintingRPC(wantsToSprint);
		}

#pragma endregion

	}
}

#pragma region Implementations

void UJPGMovementComponent::execSetSprinting(bool wantsToSprint)
{
	bWantsToSprint = wantsToSprint;
}

void UJPGMovementComponent::execSetJetpacking(float throttle)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("execSetJetpacking"), true, true, FLinearColor::Red, 10);
	float oldThrottle = fDesiredThrottle;
	throttle = FMath::Clamp<float>(throttle, 0.0, 1.0);
	bWantsToJetpack = throttle > 0;
	fDesiredThrottle = throttle;

	if (OnJetpackingChanged.IsBound())
		OnJetpackingChanged.Broadcast(oldThrottle, fDesiredThrottle);
}

void UJPGMovementComponent::execSetGliding(bool wantsToGlide)
{
	bWantsToGlide = wantsToGlide;
}

void UJPGMovementComponent::execSetTeleport(bool wantsToTeleport, FVector destination)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), FString("execSetTeleport"), true, true, FLinearColor::Red, 10);
	bWantsToTeleport = wantsToTeleport;
	teleportDestination = destination;
}

#pragma endregion

#pragma region Jetpacking Replication

void UJPGMovementComponent::ClientSetJetpackingRPC_Implementation(float throttle)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ClientSetJetpackingRPC_Implementation"), true, true, FLinearColor::Green, 10);
	execSetJetpacking(throttle);
}

bool UJPGMovementComponent::ServerSetJetpackingRPC_Validate(float throttle)
{
	return true;
}

void UJPGMovementComponent::ServerSetJetpackingRPC_Implementation(float throttle)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("ServerSetJetpackingRPC_Implementation"), true, true, FLinearColor::Black, 10);
	MulticastSetJetpackingRPC(throttle);
}

void UJPGMovementComponent::MulticastSetJetpackingRPC_Implementation(float throttle)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("MulticastSetJetpackingRPC_Implementation"), true, true, FLinearColor::Black, 10);
	execSetJetpacking(throttle);
}

#pragma endregion

#pragma region Gliding Replication

void UJPGMovementComponent::ClientSetGlidingRPC_Implementation(bool wantsToGlide)
{
	execSetGliding(wantsToGlide);
}

bool UJPGMovementComponent::ServerSetGlidingRPC_Validate(bool wantsToGlide)
{
	return true;
}

void UJPGMovementComponent::ServerSetGlidingRPC_Implementation(bool wantsToGlide)
{
	MulticastSetGlidingRPC(wantsToGlide);
}

void UJPGMovementComponent::MulticastSetGlidingRPC_Implementation(bool wantsToGlide)
{
	execSetGliding(wantsToGlide);
}

void UJPGMovementComponent::MulticastSetControlRotation_Implementation(FRotator cr)
{
	simulationControlRotation = cr;
}

#pragma endregion

#pragma region Sprinting Replication

void UJPGMovementComponent::ClientSetSprintingRPC_Implementation(bool wantsToSprint)
{
	//execSetSprinting(wantsToSprint);
}

bool UJPGMovementComponent::ServerSetSprintingRPC_Validate(bool wantsToSprint)
{
	return true;
}

void UJPGMovementComponent::ServerSetSprintingRPC_Implementation(bool wantsToSprint)
{
	//MulticastSetSprintingRPC(wantsToSprint);
}

void UJPGMovementComponent::MulticastSetSprintingRPC_Implementation(bool wantsToSprint)
{
	//execSetSprinting(wantsToSprint);
}

#pragma endregion

#pragma region Teleport Replication

void UJPGMovementComponent::ClientSetTeleportRPC_Implementation(bool wantsToTeleport, FVector destination)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString("ClientSetTeleportRPC_Implementation"), true, true, FLinearColor::Green, 10);
	execSetTeleport(wantsToTeleport, destination);
}

bool UJPGMovementComponent::ServerSetTeleportRPC_Validate(bool wantsToTeleport, FVector destination)
{
	return true;
}

void UJPGMovementComponent::ServerSetTeleportRPC_Implementation(bool wantsToTeleport, FVector destination)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString("ServerSetTeleportRPC_Implementation"), true, true, FLinearColor::Black, 10);
	MulticastSetTeleportRPC(wantsToTeleport, destination);
}

void UJPGMovementComponent::MulticastSetTeleportRPC_Implementation(bool wantsToTeleport, FVector destination)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString("MulticastSetTeleportRPC_Implementation"), true, true, FLinearColor::Black, 10);
	execSetTeleport(wantsToTeleport, destination);
}

#pragma endregion

#pragma endregion

#pragma region State Queries

bool UJPGMovementComponent::IsSprinting()
{
	return IsCustomMovementMode(ECustomMovementMode::CMOVE_SPRINT);
}

bool UJPGMovementComponent::IsJetpacking()
{
	return IsCustomMovementMode(ECustomMovementMode::CMOVE_JETPACK);
}

bool UJPGMovementComponent::IsGliding()
{
	return IsCustomMovementMode(ECustomMovementMode::CMOVE_GLIDE);
}

bool UJPGMovementComponent::CanTeleport()
{
	return IsMovingOnGround();
}

#pragma endregion

#pragma region State Conditions

bool UJPGMovementComponent::CanJetpack()
{
	if (fDistanceFromGround < JetpackMinDistanceFromGround)
	{
		return false;
	}

	if (MovementMode != EMovementMode::MOVE_Falling)
	{
		return false;
	}

	if (fJetpackResource <= 0)
	{
		return false;
	}

	return true;
}

bool UJPGMovementComponent::CanGlide()
{
	if (IsFalling() && fDistanceFromGround > GliderMinDistanceFromGround)
		return true;
	return false;
}

bool UJPGMovementComponent::CanSprint()
{
	return Super::IsMovingOnGround();
}

#pragma endregion

#pragma region Network

#pragma region FSavedMove_CustomState

void FSavedMove_CustomState::Clear()
{
	savedJetpackResource = 1.0;
	savedDistanceFromGround = 0;
	savedDesiredThrottle = 0;
	savedWantsToGlide = false;
	savedWantsToSprint = false;
}

bool FSavedMove_CustomState::IsImportant(const FSavedMove_CustomState & previous) const
{
	if (savedDesiredThrottle != previous.savedDesiredThrottle)
		return true;
	if (savedWantsToSprint != previous.savedWantsToSprint)
		return true;
	if (savedWantsToGlide != previous.savedWantsToGlide)
		return true;
	if (savedWantsToTeleport != previous.savedWantsToTeleport)
		return true;
	if (savedTeleportDestination != previous.savedTeleportDestination)
		return true;
	return false;
}

bool FSavedMove_CustomState::IsDefault() const
{
	return IsImportant(FSavedMove_CustomState());
}

#pragma endregion

#pragma region FNetworkPredictionData_Client

FNetworkPredictionData_Client*
UJPGMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);
	//Bug here I think on listen server, not sure if client or lsiten server yet Commenting out seams to be ok, testing on dedi and listen and issue is fixed when commenting out
	//check(PawnOwner->Role < ROLE_Authority);

	if (!ClientPredictionData)
	{
		UJPGMovementComponent* MutableThis = const_cast<UJPGMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_JPGMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

FNetworkPredictionData_Client_JPGMovement::FNetworkPredictionData_Client_JPGMovement(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_JPGMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_JPGMovement());
}

#pragma endregion

#pragma region FSavedMove_JPGMovement

void FSavedMove_JPGMovement::Clear()
{
	Super::Clear();
	customState.Clear();
}

uint8 FSavedMove_JPGMovement::GetCompressedFlags() const
{
	return Super::GetCompressedFlags();
}

bool FSavedMove_JPGMovement::CanCombineWith(const FSavedMovePtr & NewMove, ACharacter * Character, float MaxDelta) const
{
	if (!customState.IsImportant(((FSavedMove_JPGMovement*)&NewMove)->customState))
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_JPGMovement::CombineWith(const FSavedMove_Character * OldMove, ACharacter * InCharacter, APlayerController * PC, const FVector & OldStartLocation)
{
	//before it is combine, the DeltaTime in the current move is measured from the old move
	UJPGMovementComponent *mc = Cast< UJPGMovementComponent>(InCharacter->GetMovementComponent());
	if (mc)
	{

	}
	UKismetSystemLibrary::PrintString(InCharacter->GetWorld(), "Combined Move", true, true, FLinearColor::Red, 10);

	Super::CombineWith(OldMove, InCharacter, PC, OldStartLocation);
}

bool FSavedMove_JPGMovement::IsImportantMove(const FSavedMovePtr & LastAckedMove) const
{	
	return Super::IsImportantMove(LastAckedMove) || customState.IsImportant(((FSavedMove_JPGMovement*)&LastAckedMove)->customState);
}

void FSavedMove_JPGMovement::SetMoveFor(ACharacter * Character, float InDeltaTime, FVector const & NewAccel, FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	UJPGMovementComponent* CharMov = Cast<UJPGMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		customState.savedJetpackResource = CharMov->fJetpackResource;
		customState.savedDistanceFromGround = CharMov->fDistanceFromGround;
		customState.savedDesiredThrottle = CharMov->fDesiredThrottle;
		customState.savedWantsToSprint = CharMov->bWantsToSprint;
		customState.savedWantsToGlide = CharMov->bWantsToGlide;
		customState.savedWantsToTeleport = CharMov->bWantsToTeleport;
		customState.savedTeleportDestination = CharMov->teleportDestination;
	}
}

void FSavedMove_JPGMovement::PrepMoveFor(ACharacter * Character)
{

	Super::PrepMoveFor(Character);
	UJPGMovementComponent* CharMov = Cast<UJPGMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		CharMov->fJetpackResource = customState.savedJetpackResource;
		CharMov->fDistanceFromGround = customState.savedDistanceFromGround;
		CharMov->execSetJetpacking(customState.savedDesiredThrottle);
		CharMov->bWantsToSprint = customState.savedWantsToSprint;
		CharMov->bWantsToGlide = customState.savedWantsToGlide;
		CharMov->execSetTeleport(customState.savedWantsToTeleport, customState.savedTeleportDestination);
	}
}

#pragma endregion

#pragma region ServerMoveExtended

void UJPGMovementComponent::CallServerMove(const FSavedMove_Character * NewMove, const FSavedMove_Character * OldMove)
{
	check(NewMove != nullptr);
	const FSavedMove_JPGMovement* jpgNewMove = ((FSavedMove_JPGMovement*)NewMove);
	FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
	const FSavedMove_JPGMovement * lastAckJPGSavedMove = nullptr;

	bool isCustomStateImportant = true;
	if (ClientData)
	{
		lastAckJPGSavedMove = (FSavedMove_JPGMovement*)ClientData->LastAckedMove.Get();
	}
	if ((lastAckJPGSavedMove && !jpgNewMove->customState.IsImportant(lastAckJPGSavedMove->customState)) )
	{
		isCustomStateImportant = false;
	}

	// Compress rotation down to 5 bytes
	const uint32 ClientYawPitchINT = PackYawAndPitchTo32(NewMove->SavedControlRotation.Yaw, NewMove->SavedControlRotation.Pitch);
	const uint8 ClientRollBYTE = FRotator::CompressAxisToByte(NewMove->SavedControlRotation.Roll);

	// Determine if we send absolute or relative location
	UPrimitiveComponent* ClientMovementBase = NewMove->EndBase.Get();
	const FName ClientBaseBone = NewMove->EndBoneName;
	const FVector SendLocation = MovementBaseUtility::UseRelativeLocation(ClientMovementBase) ? NewMove->SavedRelativeLocation : NewMove->SavedLocation;

	// send old move if it exists
	if (OldMove)
	{
		const FSavedMove_JPGMovement* jpgOldMove = ((FSavedMove_JPGMovement*)OldMove);
		if ( (lastAckJPGSavedMove && !jpgOldMove->customState.IsImportant(lastAckJPGSavedMove->customState)) )
		{
			ServerMoveExtendedOld(OldMove->TimeStamp, OldMove->Acceleration, OldMove->GetCompressedFlags(), nullptr);
		}
		else
		{
			ServerMoveExtendedOld(OldMove->TimeStamp, OldMove->Acceleration, OldMove->GetCompressedFlags(), jpgOldMove);
		}
		
	}
	
	if (const FSavedMove_Character* const PendingMove = ClientData->PendingMove.Get())
	{
		const FSavedMove_JPGMovement* jpgPendingMove = ((FSavedMove_JPGMovement*)PendingMove);
		const uint32 OldClientYawPitchINT = PackYawAndPitchTo32(ClientData->PendingMove->SavedControlRotation.Yaw, ClientData->PendingMove->SavedControlRotation.Pitch);

		// If we delayed a move without root motion, and our new move has root motion, send these through a special function, so the server knows how to process them.
		if ((PendingMove->RootMotionMontage == NULL) && (NewMove->RootMotionMontage != NULL))
		{

			// send two moves simultaneously
			ServerMoveExtendedDualHybridRootMotion(
				PendingMove->TimeStamp,
				PendingMove->Acceleration,
				PendingMove->GetCompressedFlags(),
				OldClientYawPitchINT,
				NewMove->TimeStamp,
				NewMove->Acceleration,
				SendLocation,
				NewMove->GetCompressedFlags(),
				ClientRollBYTE,
				ClientYawPitchINT,
				ClientMovementBase,
				ClientBaseBone,
				NewMove->EndPackedMovementMode,
				isCustomStateImportant ? jpgNewMove : nullptr
			);
		}
		else
		{

			// send two moves simultaneously
			ServerMoveExtendedDual(
				PendingMove->TimeStamp,
				PendingMove->Acceleration,
				PendingMove->GetCompressedFlags(),
				OldClientYawPitchINT,
				NewMove->TimeStamp,
				NewMove->Acceleration,
				SendLocation,
				NewMove->GetCompressedFlags(),
				ClientRollBYTE,
				ClientYawPitchINT,
				ClientMovementBase,
				ClientBaseBone,
				NewMove->EndPackedMovementMode,
				isCustomStateImportant ? jpgNewMove : nullptr
			);
		}
	}
	else
	{

		ServerMoveExtended(
			NewMove->TimeStamp,
			NewMove->Acceleration,
			SendLocation,
			NewMove->GetCompressedFlags(),
			ClientRollBYTE,
			ClientYawPitchINT,
			ClientMovementBase,
			ClientBaseBone,
			NewMove->EndPackedMovementMode,
			isCustomStateImportant ? jpgNewMove : nullptr
		);
	}

	MarkForClientCameraUpdate();
}

void UJPGMovementComponent::ServerMoveExtended(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, const FSavedMove_JPGMovement* customMove)
{
	if (MovementBaseUtility::IsDynamicBase(ClientMovementBase))
	{
		if (customMove == nullptr)
			JetCharOwner->ServerMove(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
		else
		{
			JetCharOwner->ServerMoveExtended(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customMove->customState);
		}
	}
	else
	{
		if (customMove == nullptr)
			JetCharOwner->ServerMoveNoBase(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementMode);
		else
		{
			JetCharOwner->ServerMoveExtendedNoBase(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementMode, customMove->customState);
		}
	}
}

void UJPGMovementComponent::ServerMoveExtended_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState)
{
	ServerApplyCustomState(TimeStamp,customState);
	ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool UJPGMovementComponent::ServerMoveExtended_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState)
{
	return ServerMove_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

void UJPGMovementComponent::ServerMoveExtendedDual(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, const FSavedMove_JPGMovement* customMove)
{
	if (MovementBaseUtility::IsDynamicBase(ClientMovementBase))
	{
		if (customMove == nullptr)
			JetCharOwner->ServerMoveDual(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
		else
			JetCharOwner->ServerMoveExtendedDual(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customMove->customState);
	}
	else
	{
		if (customMove == nullptr)
			JetCharOwner->ServerMoveDualNoBase(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementMode);
		else
			JetCharOwner->ServerMoveExtendedDualNoBase(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementMode, customMove->customState);
	}
}

void UJPGMovementComponent::ServerMoveExtendedDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState)
{
	ServerApplyCustomState(TimeStamp, customState);
	ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool UJPGMovementComponent::ServerMoveExtendedDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState)
{
	return ServerMoveDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);;
}

void UJPGMovementComponent::ServerMoveExtendedDualHybridRootMotion(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, const FSavedMove_JPGMovement* customMove)
{
	if (customMove == nullptr)
		JetCharOwner->ServerMoveDualHybridRootMotion(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
	else
		JetCharOwner->ServerMoveExtendedDualHybridRootMotion(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customMove->customState);
}

void UJPGMovementComponent::ServerMoveExtendedDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState)
{
	ServerApplyCustomState(TimeStamp, customState);
	ServerMoveDualHybridRootMotion_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool UJPGMovementComponent::ServerMoveExtendedDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState &customState)
{
	return ServerMoveDualHybridRootMotion_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

void UJPGMovementComponent::ServerMoveExtendedOld(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, const FSavedMove_JPGMovement* customMove)
{
	if (customMove == nullptr)
		JetCharOwner->ServerMoveOld(OldTimeStamp, OldAccel, OldMoveFlags);
	else
		JetCharOwner->ServerMoveExtendedOld(OldTimeStamp, OldAccel, OldMoveFlags, customMove->customState);
}

void UJPGMovementComponent::ServerMoveExtendedOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState &customState)
{
	ServerApplyCustomState(OldTimeStamp, customState);
	ServerMoveOld_Implementation(OldTimeStamp, OldAccel, OldMoveFlags);
}

bool UJPGMovementComponent::ServerMoveExtendedOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState &customState)
{
	return ServerMoveOld_Validate(OldTimeStamp, OldAccel, OldMoveFlags);;
}

#pragma endregion

void UJPGMovementComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UJPGMovementComponent, lastServerCustomState, COND_SimulatedOnly);
}

void UJPGMovementComponent::OnRep_ServerCustomState()
{
	//UKismetSystemLibrary::PrintString(GetWorld(), GetOwner()->GetName() + FString("OnRep_ServerCustomState"), true, true, FLinearColor::Red, 10);
	ApplyCustomState(lastServerCustomState);
}

#pragma endregion
