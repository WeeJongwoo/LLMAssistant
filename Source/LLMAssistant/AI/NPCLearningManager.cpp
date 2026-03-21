// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCLearningManager.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "NPCInteractor.h"
#include "NPCTrainer.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "NPCMLManager.h"


// Sets default values
ANPCLearningManager::ANPCLearningManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.TickInterval = 0.1f;

	Manager = CreateDefaultSubobject<UNPCMLManager>(TEXT("Manager"));
}

// Called when the game starts or when spawned
void ANPCLearningManager::BeginPlay()
{
	Super::BeginPlay();

	Interactor = Cast<UNPCInteractor>(ULearningAgentsInteractor::MakeInteractor(Manager, UNPCInteractor::StaticClass(), TEXT("NPCInteractor")));
	Interactor->SetGoalActor(GoalActor);

	Policy = ULearningAgentsPolicy::MakePolicy(Manager, Interactor, ULearningAgentsPolicy::StaticClass(), TEXT("Policy"), nullptr, PolicyNetworkAsset, nullptr);
	Critic = ULearningAgentsCritic::MakeCritic(Manager, Interactor, Policy, ULearningAgentsCritic::StaticClass(), TEXT("Critic"), CriticNetworkAsset);
	Trainer = Cast<UNPCTrainer>(ULearningAgentsTrainer::MakeTrainer(Manager, Interactor, Policy, Critic, UNPCTrainer::StaticClass(), TEXT("Trainer")));
	Trainer->SetGoalActor(GoalActor);

	for (AMLNPCCharacter* NPC : NPCAgents)
	{
		if (NPC)
		{
			int32 Id = Manager->AddAgent(NPC);
			//UE_LOG(LogTemp, Warning, TEXT("Registered Agent %d: %s"), Id, *NPC->GetName());
		}
	}
}

void ANPCLearningManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Manager를 강제로 재생성해서 MaxAgentNum 보장
	/*if (Manager)
	{
		Manager->DestroyComponent();
	}
	Manager = NewObject<UNPCMLManager>(this, TEXT("Manager"));

	FProperty* Prop = UNPCMLManager::StaticClass()->FindPropertyByName(TEXT("MaxAgentNum"));
	if (Prop)
	{
		int32 Value = 20;
		Prop->SetValue_InContainer(Manager, &Value);
	}
	Manager->RegisterComponent();*/
}

// Called every frame
void ANPCLearningManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Log, TEXT("Manager Tick - Running Training"));

	if (!Manager || !Trainer)
	{
		return;
	}

	Trainer->RunTraining();
}

