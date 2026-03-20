// Fill out your copyright notice in the Description page of Project Settings.


#include "MLGoalActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AMLGoalActor::AMLGoalActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	GoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesk"));
	GoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTrigger"));

	RootComponent = GoalMesh;
	GoalTrigger->SetupAttachment(RootComponent);
}


