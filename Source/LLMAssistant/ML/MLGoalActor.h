// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MLGoalActor.generated.h"

UCLASS()
class LLMASSISTANT_API AMLGoalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMLGoalActor();

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> GoalMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> GoalTrigger;
};
