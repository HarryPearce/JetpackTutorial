// Fill out your copyright notice in the Description page of Project Settings.


#include "JetChar.h"
#include "Engine/Classes/Engine/World.h"
#include "JPGMovementComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ConfigCacheIni.h"
#include "App.h"
#include "Classes/Components/SkeletalMeshComponent.h"

// Sets default values
AJetChar::AJetChar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UJPGMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{
		mc->distanceCheckOrigin = GetMesh()->RelativeLocation;

	}
}

// Called when the game starts or when spawned
void AJetChar::BeginPlay()
{
	Super::BeginPlay();
	/*FHitResult res;
	GetWorld()->LineTraceSingleByChannel(
		res,
		GetActorLocation(),
		GetActorForwardVector() * 500 + GetActorLocation(),ECollisionChannel::ECC_Visibility

		);*/

	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{
		mc->distanceCheckOrigin = GetMesh()->RelativeLocation;

	}

	if (IsLocallyControlled())
	{
		FApp::SetUnfocusedVolumeMultiplier(1.0);
	}
	
}

void AJetChar::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Custom && GetCharacterMovement()->MovementMode == ECustomMovementMode::CMOVE_GLIDE)
	{

	}
}

void AJetChar::PostInitializeComponents()
{
	
	cachedCMC = Cast<UJPGMovementComponent>(GetCharacterMovement());
	Super::PostInitializeComponents();
}

// Called every frame
void AJetChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AJetChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AJetChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AJetChar::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AJetChar::LookRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AJetChar::LookUp);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AJetChar::JumpPressed);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &AJetChar::JumpReleased);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AJetChar::SprintPressed);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AJetChar::SprintReleased);
	PlayerInputComponent->BindAction("ToggleGlide", EInputEvent::IE_Pressed, this, &AJetChar::GlidePressed);
	PlayerInputComponent->BindAction("ToggleGlide", EInputEvent::IE_Released, this, &AJetChar::GlideReleased);


}

void AJetChar::MoveForward(float Value)
{

	if (Value != 0.0f)
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AJetChar::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AJetChar::LookUp(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AJetChar::LookRight(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AJetChar::JumpPressed()
{
	Super::Jump();
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{
		mc->SetJetpacking(1.0);
	}
}

void AJetChar::JumpReleased()
{
	Super::StopJumping();
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{				
		mc->SetJetpacking(0.0);
	}
}

void AJetChar::SprintPressed()
{
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{		
		mc->SetSprinting(true);
	}
}

void AJetChar::SprintReleased()
{
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{		
		mc->SetSprinting(false);
	}
}

void AJetChar::GlidePressed()
{

	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{		
		mc->SetGliding(!mc->bWantsToGlide);
	}
}

void AJetChar::GlideReleased()
{


}

void AJetChar::ServerMoveExtended_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{	
	GetJPGMovementComponent()->ServerMoveExtended_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customState);
}

bool AJetChar::ServerMoveExtended_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	return GetJPGMovementComponent()->ServerMoveExtended_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customState);
}

// ServerMoveExtendedNoBase
void AJetChar::ServerMoveExtendedNoBase_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{	
	GetJPGMovementComponent()->ServerMoveExtended_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode, customState);
}

bool AJetChar::ServerMoveExtendedNoBase_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	return GetJPGMovementComponent()->ServerMoveExtended_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode, customState);
}

// ServerMoveExtendedDual
void AJetChar::ServerMoveExtendedDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	GetJPGMovementComponent()->ServerMoveExtendedDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customState);
}

bool AJetChar::ServerMoveExtendedDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	return GetJPGMovementComponent()->ServerMoveExtendedDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customState);
}

// ServerMoveExtendedDualNoBase
void AJetChar::ServerMoveExtendedDualNoBase_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	GetJPGMovementComponent()->ServerMoveExtendedDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode, customState);
}

bool AJetChar::ServerMoveExtendedDualNoBase_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	return GetJPGMovementComponent()->ServerMoveExtendedDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode, customState);
}

// ServerMoveExtendedDualHybridRootMotion
void AJetChar::ServerMoveExtendedDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	GetJPGMovementComponent()->ServerMoveExtendedDualHybridRootMotion_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customState);
}

bool AJetChar::ServerMoveExtendedDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, FSavedMove_CustomState customState)
{
	return GetJPGMovementComponent()->ServerMoveExtendedDualHybridRootMotion_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, customState);
}

// ServerMoveExtendedOld
void AJetChar::ServerMoveExtendedOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState customState)
{
	GetJPGMovementComponent()->ServerMoveExtendedOld_Implementation(OldTimeStamp, OldAccel, OldMoveFlags, customState);
}

bool AJetChar::ServerMoveExtendedOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomState customState)
{
	return GetJPGMovementComponent()->ServerMoveExtendedOld_Validate(OldTimeStamp, OldAccel, OldMoveFlags, customState);
}

