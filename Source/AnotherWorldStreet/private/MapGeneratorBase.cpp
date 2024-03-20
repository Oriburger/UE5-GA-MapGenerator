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
	//=== ���� ������ ���� =======================
	if (CurrentGen == 0)
	{
		//�ʱ� �������� �����ϰ� 
		CreateInitialMap();

		//�н��� �����Ѵ�
		CurrentGen += 1;
		GetWorldTimerManager().SetTimer(GAElapsedTimeHandle, 1e9, true, 0.0f);
		RunGeneticAlgorithm();
	}
	//=== ������ ���� =======================
	if (CurrentGen >= GenerationThresold)
	{
		//����ð��� ���ϰ� Ÿ�̸Ӹ� ��ȯ�Ѵ�.
		TotalElapsedTime = GetWorldTimerManager().GetTimerElapsed(GAElapsedTimeHandle);
		GetWorldTimerManager().ClearTimer(GAElapsedTimeHandle);
		GAElapsedTimeHandle.Invalidate();
		GenerationDelayHandle.Invalidate();

		//�� �ð�ȭ�� �����Ѵ�. 
		Visualize(CurrentPopulationInfo.BestResultMap);

		//�˰����� ����Ǿ��ٰ� �̺�Ʈ�� Ȱ��ȭ�Ѵ�.
		OnGAEndProcess.Broadcast(StartLocation, TotalElapsedTime);
	}
	//=== �н� ���� =======================
	else if(CurrentGen > 0)
	{
		//~ �н� ���� ~

		//���븦 �ϳ� �ø���, ���൵ ������Ʈ �̺�Ʈ�� Ȱ��ȭ��Ų��.
		CurrentGen += 1;
		TotalElapsedTime = FMath::Min(TotalElapsedTime, FMath::RandRange(2150.0f, 50000.0f)); //�ӽ� �ڵ�
		OnGAUpdatedProgress.Broadcast(CurrentGen, (float)CurrentGen / (float)GenerationThresold, TotalElapsedTime);

		//���� ������ �н��� ���� ƽ���� �����Ѵ�.
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