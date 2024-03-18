// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MapGeneratorBase.h"

// Sets default values
AMapGeneratorBase::AMapGeneratorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void AMapGeneratorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapGeneratorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AMapGeneratorBase::RunGeneticAlgorithm()
{
	CreateInitialMap();

	/*
	for(CurrentGen = 1; CurrentGen <= Thresold; CurrentGen++
	{
		//Operation~~
	}
	*/

	Visualize(CurrentPopulationInfo.BestResultMap);

	return StartLocation;
}

float AMapGeneratorBase::CalculateFitness()
{
	return 0.0f;
}

float AMapGeneratorBase::SelectParents()
{
	return 0.0f;
}

FMapInfoStruct AMapGeneratorBase::Crossover(const FMapInfoStruct& child1, const FMapInfoStruct& child2)
{
	return FMapInfoStruct(); 
}

bool AMapGeneratorBase::Mutate(FMapInfoStruct& child)
{
	return false;
}

void AMapGeneratorBase::SetInitialPopulation(FMapInfoStruct MapInfo)
{
}

void AMapGeneratorBase::Repair(FPopulationStruct& Result)
{
}