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
		//~~ 적합도 연산 ~~
		//CurrentPopulationInfo.BestFitnessValue = CalculateFitness();

		//~~ 선택 ~~
		//SelectParents();

		//~~ 학습 과정 ~~
		//Crossover()
		//Mutate()
		//Repair()

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
	//적합도는 반드시 낮아야한다. 좋지 않은 상황에서 더 더하는 방식
	//적합도 계산 가중치 (도합 1), 높은 가중치 = 반드시 지켜져야하는 룰
	const float outOfRangeWeight = 0.5f; //플랫폼이 파라미터로 지정한 범위 외부에 있는지에 대한 가중치
	const float unreachableWeight = 0.3f; //플레이어가 도달 불가능한 플랫폼이 있는지에 대한 가중치
	const float difficultyWeight = 0.2f; //난이도가 적합하지 않은것에 대한 가중치

	float bestFitness = (float)1e9;

	//모집단의 모든 멤버에 대해 수행
	for (int32 mapIdx = 0; mapIdx < CurrentPopulationInfo.Population.Num(); mapIdx++)
	{
		FMapInfoStruct& currentMap = CurrentPopulationInfo.Population[mapIdx];
		float newFitness = 0.0f;

		//----[ 유효성 체크 ]---------------
		for (int32 platformIdx = 0; platformIdx < currentMap.PlatformInfoList.Num(); platformIdx++)
		{
			FPlatformInfoStruct curr = currentMap.PlatformInfoList[platformIdx];
			FVector currLocation = curr.PlatformTransform.GetLocation();

			//범위 외부에 있는지?
			if (currLocation.X < StartLocation.X && currLocation.X > EndLocation.X
				|| currLocation.Y < StartLocation.Y && currLocation.Y > EndLocation.Y
				|| currLocation.Z < StartLocation.Z && currLocation.Z > EndLocation.Z)
			{
				//표면까지의 거리를 구하는 공식으로 수정해야함 
				newFitness += outOfRangeWeight * UKismetMathLibrary::Vector_Distance(currLocation, (StartLocation + EndLocation) / 2);
			}

			if (platformIdx > 0)
			{
				FPlatformInfoStruct prev = currentMap.PlatformInfoList[platformIdx - 1];
				FVector prevLocation = prev.PlatformTransform.GetLocation();
				float distance = UKismetMathLibrary::Vector_Distance(prevLocation, currLocation);

				//float GetCanReachDistance(StartPos, EndPos, JumpSpeed, Velocity, Gravity) // 착지까지의 거리

				//도달 불가능한 노드가 있는지?


				//난이도와 얼마나 차이가 나는지?
			}
		}
		bestFitness = FMath::Min(bestFitness, currentMap.TotalFitness = newFitness);
	}

	return bestFitness;
}

void AMapGeneratorBase::SelectParents()
{
	//~~ 오름차순 정렬 ~~
	CurrentPopulationInfo.Population.Sort([](const FMapInfoStruct& m1, const FMapInfoStruct& m2) {
		return m1.TotalFitness < m2.TotalFitness;
	});
	
	//상위 n개를 고름
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
	// 원점 상대적으로 좌표 변환
	End = End - Start;
	End = { End.GetAbs().X, End.GetAbs().Y, End.Z};
	Start = FVector::ZeroVector;

	ErrorDist = 0.0f;

	//점프 속력도 절대 벡터로 변환
	//JumpVelocity = { JumpVelocity.GetAbs();

	//중력가속도 초기화
	GravityScale *= 980.0f;

	// 발사체의 초기 위치와 도착 지점 사이의 거리
	float DistanceToTarget = FVector::Dist(Start, End);

	// 최고 높이까지의 도달 시간을 계산
	float VerticalTime = JumpVelocity.Z / (GravityScale * 0.5f);

	// 도달할 수 있는 가장 높은 지점
	float HighestHeight = FMath::Pow(JumpVelocity.Z, 2) / (GravityScale * 2.0f) + 40.0f;

	// 수평 방향으로 이동할 거리 계산
	float HorizontalDistance = JumpVelocity.X * VerticalTime;


	UE_LOG(LogTemp, Warning, TEXT("h dist : %.2lf, hightest : %.2lf, time : %.2lf, gravity : %.2lf"), HorizontalDistance, HighestHeight, VerticalTime, GravityScale);

	// 도착 지점과의 거리가 발사체가 이동할 수 있는 거리보다 짧다면 도달 가능
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