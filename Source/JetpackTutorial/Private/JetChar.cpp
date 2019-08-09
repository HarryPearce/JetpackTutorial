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
		if (!HasAuthority())
			mc->ServerSetJetpackingRPC(0.0);
		//else
		mc->SetJetpacking(0.0);
	}
}

void AJetChar::SprintPressed()
{
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{
		if (!HasAuthority())
			mc->ServerSetSprintingRPC(true);
		//else
		mc->SetSprinting(true);
	}
}

void AJetChar::SprintReleased()
{
	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{
		if (!HasAuthority())
			mc->ServerSetSprintingRPC(false);
		//else
		mc->SetSprinting(false);
	}
}

void AJetChar::GlidePressed()
{

	UJPGMovementComponent* mc = Cast<UJPGMovementComponent>(GetCharacterMovement());
	if (mc)
	{
		if (!HasAuthority())
			mc->ServerSetGlidingRPC(!mc->bWantsToGlide);
		//else
		mc->SetGliding(!mc->bWantsToGlide);
	}
}

void AJetChar::GlideReleased()
{


}


