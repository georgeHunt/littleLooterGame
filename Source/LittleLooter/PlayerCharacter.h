// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class LITTLELOOTER_API APlayerCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCharacter();

	void MoveForward(float MovementAmount);
	void MoveRight(float MovementAmount);
	void LookUp(float LookAmount);
	void LookRight(float LookAmount);

	void StartSprint();
	void StopSprint();

	void StartJump();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere) UCapsuleComponent* CapsuleComponent = nullptr;
	UPROPERTY(EditAnywhere, Category="Movement") float Speed = 5.0f;
	UPROPERTY(EditAnywhere, Category="Movement|Jump") float JumpSpeed = 400.f;          // initial upward velocity (cm/s)
	UPROPERTY(EditAnywhere, Category="Movement|Jump") float Gravity = -980.f;           // cm/s^2
	UPROPERTY(EditAnywhere, Category="Movement|Jump") float GroundCheckDistance = 60.f; // how far down we trace
	UPROPERTY(EditAnywhere, Category="Movement|Jump") float GroundStickTolerance = 4.f; // if this close, consider grounded

	bool bIsGrounded = false;
	float VerticalVelocity = 0.f;

	void UpdateGroundedState();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere);
	UCameraComponent* CameraComponent;	

};
