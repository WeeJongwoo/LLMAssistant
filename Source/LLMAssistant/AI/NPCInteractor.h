// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "NPCInteractor.generated.h"

/**
 * 
 */
UCLASS()
class LLMASSISTANT_API UNPCInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()
	
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> GoalActor;

protected:
	
	void SpecifyAgentObservation_Implementation(FLearningAgentsObservationSchemaElement& OutObservationSchemaElement, ULearningAgentsObservationSchema* InObservationSchema);
	
	void GatherAgentObservation_Implementation(FLearningAgentsObservationObjectElement& OutObservationObjectElement, ULearningAgentsObservationObject* InObservationObject, const int32 AgentId);

	void SpecifyAgentAction_Implementation(FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema);

	void PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject, const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId);
};
