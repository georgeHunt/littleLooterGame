// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->InitCapsuleSize(34.f, 88.f); // radius, half-height (cm)
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->SetSimulatePhysics(false);
	CapsuleComponent->SetEnableGravity(false);
	RootComponent = CapsuleComponent;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->FieldOfView = 100.0f;
	CameraComponent->SetRelativeLocation(FVector(-100.0f, 0.0f, 50.0f));
}


void APlayerCharacter::MoveForward(float movementAmount)
{
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector NewLocation = GetActorLocation() + (FVector(ForwardVector.X, ForwardVector.Y, 0.0f) * movementAmount * Speed);
	SetActorLocation(NewLocation);
}
void APlayerCharacter::MoveRight(float movementAmount)
{
	FVector RightVector = CameraComponent->GetRightVector();
	FVector NewLocation = GetActorLocation() + (RightVector * movementAmount * Speed);
	SetActorLocation(NewLocation);
}
void APlayerCharacter::LookUp(float lookAmount)
{
	FRotator NewRotation = CameraComponent->GetRelativeRotation();
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + lookAmount, -89.0f, 89.0f);
	CameraComponent->SetRelativeRotation(NewRotation);
}
void APlayerCharacter::LookRight(float lookAmount)
{
	FRotator NewRotation = CameraComponent->GetRelativeRotation();
	NewRotation.Yaw += lookAmount;
	CameraComponent->SetRelativeRotation(NewRotation);
}
void APlayerCharacter::StartSprint()
{
	Speed *= 2.0f;
}
void APlayerCharacter::StopSprint()
{
	Speed /= 2.0f;
}
void APlayerCharacter::StartJump()
{
	if (bIsGrounded)
	{
		bIsGrounded = false;
		VerticalVelocity = JumpSpeed;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Initialize grounded state at spawn
	UpdateGroundedState();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGroundedState();

	if (!bIsGrounded)
	{
		VerticalVelocity += Gravity * DeltaTime;

		FVector Loc = GetActorLocation();
		Loc.Z += VerticalVelocity * DeltaTime;

		// Sweep to avoid tunneling through ceilings/floors
		SetActorLocation(Loc, true);
	}
	else
	{
		// Clamp downwards velocity when grounded
		if (VerticalVelocity < 0.f)
		{
			VerticalVelocity = 0.f;
		}

		// Tiny stick-to-ground to prevent micro-bounces
		FVector Loc = GetActorLocation();
		Loc.Z -= GroundStickTolerance * 0.5f;
		SetActorLocation(Loc, true);
	}
}

void APlayerCharacter::UpdateGroundedState()
{
	if (!GetWorld()) return;

	// Get capsule half height
	float CapsuleHalfHeight = 0.f;
	if (UCapsuleComponent* Capsule = FindComponentByClass<UCapsuleComponent>())
	{
		CapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	}

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, GroundCheckDistance + CapsuleHalfHeight);

	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("GroundTrace"), false, this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		// If we're moving down or stationary and are very close to the floor, treat as grounded
		const float Dist = Start.Z - Hit.ImpactPoint.Z;
		bIsGrounded = (VerticalVelocity <= 0.f) && (Dist <= GroundStickTolerance + CapsuleHalfHeight);
		if (bIsGrounded)
		{
			// Snap Z to hit + capsule half height
			FVector Loc = GetActorLocation();
			Loc.Z = Hit.ImpactPoint.Z + CapsuleHalfHeight;
			SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	else
	{
		bIsGrounded = false;
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &APlayerCharacter::LookRight);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::StopSprint);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::StartJump);
}
