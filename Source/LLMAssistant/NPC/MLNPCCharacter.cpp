// Fill out your copyright notice in the Description page of Project Settings.


#include "MLNPCCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AMLNPCCharacter::AMLNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->NavAgentProps.bCanCrouch = true;
	MoveComp->GetNavAgentPropertiesRef().bCanJump = true;

	CrouchHalfHight = 50.0f;

	TraceStartForCrouch = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStartForCrouch"));
	TraceStartForJump = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStartForJump"));

	TraceStartForCrouch->SetupAttachment(RootComponent);
	TraceStartForJump->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMLNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	if (World)
	{
		StartLocation = GetActorLocation();
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		if (Capsule)
		{
			DefaultCapsuleHalfHight = Capsule->GetScaledCapsuleHalfHeight();
			DefaultCapsuleRelativeLocation = Capsule->GetRelativeLocation();
		}
	}
}

void AMLNPCCharacter::ExecuteAction(ENPCAction Action, const FVector& MoveDirection)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (MoveComp)
	{
		switch (Action)
		{
		case ENPCAction::Walk:
			if (MoveComp->IsCrouching())
			{
				MoveComp->UnCrouch();
			}
			AddMovementInput(MoveDirection);
			break;
		case ENPCAction::Jump:
			if (MoveComp->IsCrouching())
			{
				MoveComp->UnCrouch();
			}
			Jump();
			AddMovementInput(MoveDirection);
			break;
		case ENPCAction::ClouchWalk:
			Crouch();
			AddMovementInput(MoveDirection);
			break;
		default:
			break;
		}
	}
}

void AMLNPCCharacter::ResetToStart()
{
	SetActorLocation(StartLocation);
	GetCharacterMovement()->StopMovementImmediately();

	if (GetCharacterMovement()->IsCrouching())
		UnCrouch();
}

void AMLNPCCharacter::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);

	GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchHalfHight);
}

void AMLNPCCharacter::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);

	GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHight);
}


