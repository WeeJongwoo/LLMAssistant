// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCLearningManager.generated.h"

class ULearningAgentsManager;
class UNPCInteractor;
class UNPCTrainer;
class ULearningAgentsPolicy;
class ULearningAgentsCritic;
class AMLNPCCharacter;
class ULearningAgentsNeuralNetwork;
class UNPCMLManager;

UCLASS()
class LLMASSISTANT_API ANPCLearningManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPCLearningManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<AActor> GoalActor;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TArray<TObjectPtr<AMLNPCCharacter>> NPCAgents;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<ULearningAgentsNeuralNetwork> PolicyNetworkAsset;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<ULearningAgentsNeuralNetwork> CriticNetworkAsset;

	UPROPERTY(EditAnywhere) 
	TObjectPtr<UNPCMLManager> Manager;

	UPROPERTY() 
	TObjectPtr<UNPCInteractor> Interactor;

	UPROPERTY() 
	TObjectPtr<UNPCTrainer> Trainer;

	UPROPERTY() 
	TObjectPtr<ULearningAgentsPolicy> Policy;

	UPROPERTY() 
	TObjectPtr<ULearningAgentsCritic> Critic;

	bool bTrainingStarted = false;
};
