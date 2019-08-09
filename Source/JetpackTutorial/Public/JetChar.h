// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CoreUObject/Public/UObject/UObjectGlobals.h"
#include "JetChar.generated.h"

UCLASS()
class JETPACKTUTORIAL_API AJetChar : public ACharacter
{
	GENERATED_BODY()


public:
	// Sets default values for this character's properties
	AJetChar(const FObjectInitializer& ObjectInitializer);
	//AJetChar();

	
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

public:	
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


	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Rate);
	void LookRight(float Rate);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
		float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
		float BaseLookUpRate = 45.f;
};
