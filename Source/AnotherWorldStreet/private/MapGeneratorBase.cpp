// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MapGeneratorBase.h"
#include "Kismet/KismetMathLibrary.h"

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
		//~~ ���յ� ���� ~~
		//CurrentPopulationInfo.BestFitnessValue = CalculateFitness();

		//~~ ���� ~~
		//SelectParents();

		//~~ �н� ���� ~~
		//Crossover()
		//Mutate()
		//Repair()

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
	//���յ��� �ݵ�� ���ƾ��Ѵ�. ���� ���� ��Ȳ���� �� ���ϴ� ���
	//���յ� ��� ����ġ (���� 1), ���� ����ġ = �ݵ�� ���������ϴ� ��
	const float outOfRangeWeight = 0.5f; //�÷����� �Ķ���ͷ� ������ ���� �ܺο� �ִ����� ���� ����ġ
	const float unreachableWeight = 0.3f; //�÷��̾ ���� �Ұ����� �÷����� �ִ����� ���� ����ġ
	const float difficultyWeight = 0.2f; //���̵��� �������� �����Ϳ� ���� ����ġ

	float bestFitness = (float)1e9;

	//�������� ��� ����� ���� ����
	for (int32 mapIdx = 0; mapIdx < CurrentPopulationInfo.Population.Num(); mapIdx++)
	{
		FMapInfoStruct& currentMap = CurrentPopulationInfo.Population[mapIdx];
		float newFitness = 0.0f;

		//----[ ��ȿ�� üũ ]---------------
		for (int32 platformIdx = 0; platformIdx < currentMap.PlatformInfoList.Num(); platformIdx++)
		{
			FPlatformInfoStruct curr = currentMap.PlatformInfoList[platformIdx];
			FVector currLocation = curr.PlatformTransform.GetLocation();

			//���� �ܺο� �ִ���?
			if (currLocation.X < StartLocation.X && currLocation.X > EndLocation.X
				|| currLocation.Y < StartLocation.Y && currLocation.Y > EndLocation.Y
				|| currLocation.Z < StartLocation.Z && currLocation.Z > EndLocation.Z)
			{
				//ǥ������� �Ÿ��� ���ϴ� �������� �����ؾ��� 
				newFitness += outOfRangeWeight * UKismetMathLibrary::Vector_Distance(currLocation, (StartLocation + EndLocation) / 2);
			}

			if (platformIdx > 0)
			{
				FPlatformInfoStruct prev = currentMap.PlatformInfoList[platformIdx - 1];
				FVector prevLocation = prev.PlatformTransform.GetLocation();
				float distance = UKismetMathLibrary::Vector_Distance(prevLocation, currLocation);

				//float GetCanReachDistance(StartPos, EndPos, JumpSpeed, Velocity, Gravity) // ���������� �Ÿ�

				//���� �Ұ����� ��尡 �ִ���?


				//���̵��� �󸶳� ���̰� ������?
			}
		}
		bestFitness = FMath::Min(bestFitness, currentMap.TotalFitness = newFitness);
	}

	return bestFitness;
}

void AMapGeneratorBase::SelectParents()
{
	//~~ �������� ���� ~~
	CurrentPopulationInfo.Population.Sort([](const FMapInfoStruct& m1, const FMapInfoStruct& m2) {
		return m1.TotalFitness < m2.TotalFitness;
	});
	
	//���� n���� ��
	FPopulationStruct tempPopulation;
	for (int32 idx = 0; idx < PopulationSize; idx++)
	{
		tempPopulation.Population.Add(CurrentPopulationInfo.Population[idx]);
	}
	CurrentPopulationInfo = tempPopulation;
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
	CurrentPopulationInfo.BestResultMap = MapInfo;
}

bool AMapGeneratorBase::GetCanReach(FVector JumpVelocity, FVector Start, FVector End, float& ErrorDist, float GravityScale)
{
	// ���� ��������� ��ǥ ��ȯ
	End = End - Start;
	End = { End.GetAbs().X, End.GetAbs().Y, End.Z};
	Start = FVector::ZeroVector;

	ErrorDist = 0.0f;

	//���� �ӷµ� ���� ���ͷ� ��ȯ
	//JumpVelocity = { JumpVelocity.GetAbs();

	//�߷°��ӵ� �ʱ�ȭ
	GravityScale *= 980.0f;

	// �߻�ü�� �ʱ� ��ġ�� ���� ���� ������ �Ÿ�
	float DistanceToTarget = FVector::Dist(Start, End);

	// �ְ� ���̱����� ���� �ð��� ���
	float VerticalTime = JumpVelocity.Z / (GravityScale * 0.5f);

	// ������ �� �ִ� ���� ���� ����
	float HighestHeight = FMath::Pow(JumpVelocity.Z, 2) / (GravityScale * 2.0f) + 40.0f;

	// ���� �������� �̵��� �Ÿ� ���
	float HorizontalDistance = JumpVelocity.X * VerticalTime;


	UE_LOG(LogTemp, Warning, TEXT("h dist : %.2lf, hightest : %.2lf, time : %.2lf, gravity : %.2lf"), HorizontalDistance, HighestHeight, VerticalTime, GravityScale);

	// ���� �������� �Ÿ��� �߻�ü�� �̵��� �� �ִ� �Ÿ����� ª�ٸ� ���� ����
	float tolerance = 5.0f;
	
	if (HighestHeight < End.Z || DistanceToTarget > HorizontalDistance + tolerance) return false;

	float targetTime = DistanceToTarget / JumpVelocity.X;
	float targetTimeVelocity = 0.0f; 
	float targetTimeHeight = 0.0f;

	if (targetTime < VerticalTime / 2.0f)
	{
		targetTimeVelocity = JumpVelocity.Z - GravityScale * targetTime;
		targetTimeHeight = HighestHeight - (targetTimeVelocity * 0.5f * (VerticalTime / 2.0f - targetTime) * 0.5f);
		UE_LOG(LogTemp, Warning, TEXT("target1) time : %.2lf, velo : %.2lf, height : %.2lf"), targetTime, targetTimeVelocity, targetTimeHeight);
	}
	else
	{
		targetTimeVelocity = JumpVelocity.Z - GravityScale * targetTime;
		targetTimeHeight = HighestHeight + (targetTimeVelocity * 0.5f * FMath::Abs(targetTime - VerticalTime / 2.0f) * 0.5f);
		UE_LOG(LogTemp, Warning, TEXT("target2) time : %.2lf, velo : %.2lf, height : %.2lf"), targetTime, targetTimeVelocity, targetTimeHeight);
	}


	if (targetTimeHeight >= End.Z) return true; 

	ErrorDist = DistanceToTarget - HorizontalDistance;
	return false;
}

void AMapGeneratorBase::Repair(FPopulationStruct& Result)
{
	
}