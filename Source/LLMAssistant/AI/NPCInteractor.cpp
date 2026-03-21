// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCInteractor.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsActions.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"


void UNPCInteractor::SpecifyAgentObservation_Implementation(FLearningAgentsObservationSchemaElement& OutObservationSchemaElement, ULearningAgentsObservationSchema* InObservationSchema)
{
	TMap<FName, FLearningAgentsObservationSchemaElement> Elements;

	Elements.Add(TEXT("GoalDist"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("GoalDist")));
	Elements.Add(TEXT("GoalAngle"), ULearningAgentsObservations::SpecifyAngleObservation(InObservationSchema, TEXT("GoalAngle")));
	Elements.Add(TEXT("Speed"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("Speed")));
	Elements.Add(TEXT("IsCrouching"), ULearningAgentsObservations::SpecifyBoolObservation(InObservationSchema, TEXT("IsCrouching")));
	Elements.Add(TEXT("IsOnGround"), ULearningAgentsObservations::SpecifyBoolObservation(InObservationSchema, TEXT("IsOnGround")));
	Elements.Add(TEXT("FrontWallDist"), ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, TEXT("FrontWallDist")));
	Elements.Add(TEXT("TopHit"), ULearningAgentsObservations::SpecifyBoolObservation(InObservationSchema, TEXT("TopHit")));
	Elements.Add(TEXT("DownHit"), ULearningAgentsObservations::SpecifyBoolObservation(InObservationSchema, TEXT("DownHit")));

	OutObservationSchemaElement = ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, Elements, TEXT("NPCObservation"));
}

void UNPCInteractor::GatherAgentObservation_Implementation(FLearningAgentsObservationObjectElement& OutObservationObjectElement, ULearningAgentsObservationObject* InObservationObject, const int32 AgentId)
{
	AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
	if (!NPC || !GoalActor)
	{
		return;
	}

	const FVector Loc = NPC->GetActorLocation();
	const FVector GoalLoc = GoalActor->GetActorLocation();
	const FVector ToGoal = GoalLoc - Loc;
	const float FootZ = Loc.Z - NPC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// ── 목표 방향 각도 계산 ──
	const FVector Forward = NPC->GetActorForwardVector();
	const FVector ToGoalDir = ToGoal.GetSafeNormal();
	const float Dot = FVector::DotProduct(Forward, ToGoalDir);
	const float Cross = FVector::CrossProduct(Forward, ToGoalDir).Z;
	const float Angle = FMath::Atan2(Cross, Dot);

	// ── 전방 장애물 레이캐스트 ──

	const FVector RayStart = FVector(Loc.X, Loc.Y, FootZ + NPC->GetStandingTraceHeight());
	const FVector RayEnd = RayStart + Forward * TraceRange;
	UWorld* World = GetWorld();

	FCollisionQueryParams Query;
	Query.AddIgnoredActor(NPC);
	Query.bTraceComplex = false;

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic, Query);

	bool bIsDownHit = false;
	bool bISTopHit = false;

	/*if (bHit)
	{
		DrawDebugLine(World, RayStart, RayEnd, FColor::Cyan, false, 3.0f);
		UE_LOG(LogTemp, Warning, TEXT("Center Hit / Hight: %f"), RayStart.Z);
	}*/

	FHitResult TopHit;
	const FVector TopStart = FVector(Loc.X, Loc.Y, FootZ + NPC->GetJumpTraceHeight());
	const FVector TopEnd = TopStart + (Forward * TraceRange);
	bISTopHit = GetWorld()->LineTraceSingleByChannel(TopHit, TopStart, TopEnd, ECC_WorldStatic, Query);
	/*if (bISTopHit)
	{
		DrawDebugLine(World, TopStart, TopEnd, FColor::Green, false, 3.0f);
		UE_LOG(LogTemp, Warning, TEXT("Top Hit / Hight: %f"), TopEnd.Z);
	}*/

	FHitResult DownHit;
	const FVector DownStart = FVector(Loc.X, Loc.Y, FootZ + NPC->GetCrouchTraceHeight());
	const FVector DownEnd = DownStart + (Forward * TraceRange);
	bIsDownHit = GetWorld()->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ECC_WorldStatic, Query);
	/*if (bIsDownHit)
	{
		DrawDebugLine(World, DownStart, DownEnd, FColor::Red, false, 3.0f);
		UE_LOG(LogTemp, Warning, TEXT("Down Hit / Hight: %f"), DownStart.Z);
	}*/
	

	TMap<FName, FLearningAgentsObservationObjectElement> Elements;

	Elements.Add(TEXT("GoalDist"),ULearningAgentsObservations::MakeFloatObservation(InObservationObject, ToGoal.Size(), 2000.f, TEXT("GoalDist")));
	Elements.Add(TEXT("GoalAngle"), ULearningAgentsObservations::MakeAngleObservation(InObservationObject, Angle, 0.0f, TEXT("GoalAngle")));
	Elements.Add(TEXT("Speed"), ULearningAgentsObservations::MakeFloatObservation(InObservationObject, NPC->GetVelocity().Size(), 2000.f, TEXT("Speed")));
	Elements.Add(TEXT("IsCrouching"), ULearningAgentsObservations::MakeBoolObservation(InObservationObject, NPC->GetCharacterMovement()->IsCrouching(), TEXT("IsCrouching")));
	Elements.Add(TEXT("IsOnGround"), ULearningAgentsObservations::MakeBoolObservation(InObservationObject, NPC->GetCharacterMovement()->IsMovingOnGround(), TEXT("IsOnGround")));
	Elements.Add(TEXT("FrontWallDist"), ULearningAgentsObservations::MakeFloatObservation(InObservationObject, bHit ? Hit.Distance : 500.f, 500.f, TEXT("FrontWallDist")));
	Elements.Add(TEXT("TopHit"), ULearningAgentsObservations::MakeBoolObservation(InObservationObject, bISTopHit, TEXT("TopHit")));
	Elements.Add(TEXT("DownHit"), ULearningAgentsObservations::MakeBoolObservation(InObservationObject, bIsDownHit, TEXT("DownHit")));

	OutObservationObjectElement = ULearningAgentsObservations::MakeStructObservation(InObservationObject, Elements, TEXT("NPCObservation"));
}

void UNPCInteractor::SpecifyAgentAction_Implementation(FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema)
{
	OutActionSchemaElement = ULearningAgentsActions::SpecifyExclusiveDiscreteAction(InActionSchema, 3, TArray<float>(), TEXT("Movement"));
}

void UNPCInteractor::PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject, const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId)
{
	int32 ActionIndex = 0;
	ULearningAgentsActions::GetExclusiveDiscreteAction(ActionIndex, InActionObject, InActionObjectElement, TEXT("Movement"));

	//UE_LOG(LogTemp, Warning, TEXT("Agent %d Action: %d"), AgentId, ActionIndex);

	const ENPCAction Action = StaticCast<ENPCAction>(ActionIndex);
	AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
	if (!NPC || !GoalActor)
	{
		return;
	}

	const FVector Dir = (GoalActor->GetActorLocation() - NPC->GetActorLocation()).GetSafeNormal();

	NPC->ExecuteAction(Action, Dir);
}
