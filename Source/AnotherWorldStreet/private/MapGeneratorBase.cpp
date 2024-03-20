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

void AMapGeneratorBase::RunGeneticAlgorithm()
{
	//=== 최초 모집단 생성 =======================
	if (CurrentGen == 0)
	{
		//초기 모집단을 생성하고 
		CreateInitialMap();

		//학습을 시작한다
		CurrentGen += 1;
		GetWorldTimerManager().SetTimer(GAElapsedTimeHandle, 1e9, true, 0.0f);
		RunGeneticAlgorithm();
	}
	//=== 마지막 세대 =======================
	if (CurrentGen >= GenerationThresold)
	{
		//경과시간을 구하고 타이머를 반환한다.
		TotalElapsedTime = GetWorldTimerManager().GetTimerElapsed(GAElapsedTimeHandle);
		GetWorldTimerManager().ClearTimer(GAElapsedTimeHandle);
		GAElapsedTimeHandle.Invalidate();
		GenerationDelayHandle.Invalidate();

		//맵 시각화를 진행한다. 
		Visualize(CurrentPopulationInfo.BestResultMap);

		//알고리즘이 종료되었다고 이벤트를 활성화한다.
		OnGAEndProcess.Broadcast(StartLocation, TotalElapsedTime);
	}
	//=== 학습 세대 =======================
	else if(CurrentGen > 0)
	{
		//~ 학습 과정 ~

		//세대를 하나 늘리고, 진행도 업데이트 이벤트를 활성화시킨다.
		CurrentGen += 1;
		TotalElapsedTime = FMath::Min(TotalElapsedTime, FMath::RandRange(2150.0f, 50000.0f)); //임시 코드
		OnGAUpdatedProgress.Broadcast(CurrentGen, (float)CurrentGen / (float)GenerationThresold, TotalElapsedTime);

		//다음 세대의 학습을 다음 틱에서 진행한다.
		GetWorldTimerManager().SetTimer(GenerationDelayHandle, this, &AMapGeneratorBase::RunGeneticAlgorithm, 0.001f, false);	
	}
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