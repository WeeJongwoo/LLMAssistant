// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCInteractor.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsActions.h"
#include "GameFramework/CharacterMovementComponent.h"


void UNPCInteractor::SpecifyAgentObservation_Implementation(FLearningAgentsObservationSchemaElement& OutObservationSchemaElement, ULearningAgentsObservationSchema* InObservationSchema)
{
	TMap<FName, FLearningAgentsObservationSchemaElement> Elements;

	Elements.Add(TEXT("GoalDist"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("GoalDist")));
	Elements.Add(TEXT("GoalAngle"), ULearningAgentsObservations::SpecifyAngleObservation(InObservationSchema, TEXT("GoalAngle")));
	Elements.Add(TEXT("Speed"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("Speed")));
	Elements.Add(TEXT("IsCrouching"), ULearningAgentsObservations::SpecifyBoolObservation(InObservationSchema, TEXT("IsCrouching")));
	Elements.Add(TEXT("IsOnGround"), ULearningAgentsObservations::SpecifyBoolObservation(InObservationSchema, TEXT("IsOnGround")));
	Elements.Add(TEXT("FrontWallDist"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("FrontWallDist")));
	Elements.Add(TEXT("FrontWallHeight"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("FrontWallHeight")));
	Elements.Add(TEXT("FrontWallHeight"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("FrontCrouchHeight")));

	OutObservationSchemaElement = ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, Elements, TEXT("NPCObservation"));
}

void UNPCInteractor::GatherAgentObservation_Implementation(FLearningAgentsObservationObjectElement& OutObservationObjectElement, ULearningAgentsObservationObject* InObservationObject, const int32 AgentId)
{
	AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
	if (!NPC || IsValid(GoalActor))
	{
		return;
	}

	const FVector Loc = NPC->GetActorLocation();
	const FVector GoalLoc = GoalActor->GetActorLocation();
	const FVector ToGoal = GoalLoc - Loc;

	// ── 목표 방향 각도 계산 ──
	const FVector Forward = NPC->GetActorForwardVector();
	const FVector ToGoalDir = ToGoal.GetSafeNormal();
	const float Dot = FVector::DotProduct(Forward, ToGoalDir);
	const float Cross = FVector::CrossProduct(Forward, ToGoalDir).Z;
	const float Angle = FMath::Atan2(Cross, Dot);

	// ── 전방 장애물 레이캐스트 ──
	const FVector ToGoalFlat = FVector(ToGoal.X, ToGoal.Y, 0.f).GetSafeNormal();

	const FVector RayStart = Loc;

	const FVector RayEnd = RayStart + Forward * 500.f;

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic);

	float WallHeight = 0.f;
	float CrouchHight = 0.0f;
	if (bHit)
	{
		FHitResult TopHit;
		const FVector TopStart = Hit.ImpactPoint + FVector(0.f, 0.f, NPC->GetRelativeTraceStartForJump().X);
		const FVector TopEnd = Hit.ImpactPoint;
		bool bISTopHit = GetWorld()->LineTraceSingleByChannel(TopHit, TopStart, TopEnd, ECC_WorldStatic);
		if (bISTopHit)
		{
			WallHeight = TopHit.ImpactPoint.Z;
		}
		else
		{
			FHitResult DownHit;
			const FVector DownStart = Hit.ImpactPoint + FVector(0.f, 0.f, NPC->GetRelativeTraceStartForCrouch().X);
			const FVector DownEnd = Hit.ImpactPoint;
			bool bIsDownHit = GetWorld()->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ECC_WorldStatic);
			if (bIsDownHit)
			{
				CrouchHight = DownHit.ImpactPoint.Z;
			}
		}
	}

	TMap<FName, FLearningAgentsObservationObjectElement> Elements;

	Elements.Add(TEXT("GoalDist"),ULearningAgentsObservations::MakeFloatObservation(InObservationObject, ToGoal.Size(), 2000.f, TEXT("GoalDist")));
	Elements.Add(TEXT("GoalAngle"), ULearningAgentsObservations::MakeAngleObservation(InObservationObject, Angle, 0.0f, TEXT("GoalAngle")));
	Elements.Add(TEXT("Speed"), ULearningAgentsObservations::MakeFloatObservation(InObservationObject, ToGoal.Size(), 2000.f, TEXT("Speed")));
	Elements.Add(TEXT("IsCrouching"), ULearningAgentsObservations::MakeBoolObservation(InObservationObject, NPC->GetCharacterMovement()->IsCrouching(), TEXT("IsCrouching")));
	Elements.Add(TEXT("IsOnGround"), ULearningAgentsObservations::MakeBoolObservation(InObservationObject, NPC->GetCharacterMovement()->IsMovingOnGround(), TEXT("IsOnGround")));
	Elements.Add(TEXT("FrontWallDist"), ULearningAgentsObservations::MakeFloatObservation(InObservationObject, bHit ? Hit.Distance : 500.f, 500.f, TEXT("FrontWallDist")));
	Elements.Add(TEXT("FrontWallHeight"), ULearningAgentsObservations::MakeFloatObservation(InObservationObject, WallHeight, 300, TEXT("FrontWallHeight")));
	Elements.Add(TEXT("FrontWallHeight"), ULearningAgentsObservations::MakeFloatObservation(InObservationObject, CrouchHight, 100, TEXT("FrontCrouchHeight")));

	OutObservationObjectElement = ULearningAgentsObservations::MakeStructObservation(InObservationObject, Elements, TEXT("NPCObservation"));
}

void UNPCInteractor::SpecifyAgentAction_Implementation(FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema)
{

}

void UNPCInteractor::PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject, const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId)
{

}
