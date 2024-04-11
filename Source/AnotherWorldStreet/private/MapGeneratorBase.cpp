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
	
	PlayerRef = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(PlayerRef))
	{
		JumpVelocity.X = PlayerRef->GetCharacterMovement()->MaxWalkSpeed;
		JumpVelocity.Z = PlayerRef->GetCharacterMovement()->JumpZVelocity;
		GravityMultipiler = PlayerRef->GetCharacterMovement()->GravityScale;
	}

	InitPlatformMeshPointInfo();
}

// Called every frame
void AMapGeneratorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMapGeneratorBase::RunGeneticAlgorithm()
{
	UE_LOG(LogTemp, Warning, TEXT("CurrentGen : %d, Threshold : %d"), CurrentGen, GenerationThresold);
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
		CurrentPopulationInfo.BestFitnessValue = CalculateFitness();
		BestFitnessValue = FMath::Min(BestFitnessValue, CurrentPopulationInfo.BestFitnessValue);

		//~~ 선택 ~~
		SelectParents();

		//~~ 학습 과정 ~~
		//Crossover();
		//Mutate()
		//Repair()

		//세대를 하나 늘리고, 진행도 업데이트 이벤트를 활성화시킨다.
		CurrentGen += 1;
		OnGAUpdatedProgress.Broadcast(CurrentGen, (float)CurrentGen/(float)GenerationThresold, BestFitnessValue);

		//다음 세대의 학습을 다음 틱에서 진행한다.
		GetWorldTimerManager().SetTimer(GenerationDelayHandle, this, &AMapGeneratorBase::RunGeneticAlgorithm, 0.001f, false);	
	}
}

float AMapGeneratorBase::CalculateFitness()
{
	//적합도는 반드시 낮아야한다. 좋지 않은 상황에서 더 더하는 방식
	//적합도 계산 가중치 (도합 1), 높은 가중치 = 반드시 지켜져야하는 룰
	const float outOfRangeWeight = 0.4f; //플랫폼이 파라미터로 지정한 범위 외부에 있는지에 대한 가중치
	const float overlappedWeight = 0.2f; //겹치는 플랫폼이 있는지에 대한 가중치
	const float unreachableWeight = 0.35f; //플레이어가 도달 불가능한 플랫폼이 있는지에 대한 가중치
	const float difficultyWeight = 0.05f; //난이도가 적합하지 않은것에 대한 가중치

	const float outOfRangeValue = 99999.0f;
	const float overlappedValue = 200.0f;

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
				newFitness += outOfRangeWeight * outOfRangeValue;
			}
			
			if (platformIdx > 0)
			{
				FPlatformInfoStruct prev = currentMap.PlatformInfoList[platformIdx - 1];
				FVector prevLocation = prev.PlatformTransform.GetLocation();
				float distance = UKismetMathLibrary::Vector_Distance(prevLocation, currLocation);
				
				//겹치는 플랫폼이 있는지?
				newFitness += (GetIsOverlapped(curr, prev) ? overlappedValue : 0.0f);

				//도달 불가능한 노드가 있는지?
				float errorValue = 0.0f;
				bool result = GetCanReach(JumpVelocity, prevLocation, currLocation, errorValue, GravityMultipiler);
				newFitness += (errorValue * unreachableWeight);

				//난이도와 얼마나 차이가 나는지?
				const float errorThreshold = 100.0f;
				if (result)
				{
					if (!LevelPerJumpDistThreshold.IsValidIndex(MapLevel))
					{
						UE_LOG(LogTemp, Error, TEXT("Fitness() : LevelPerJumpDistThreshold %d Data is missing."), MapLevel);
						continue;
					}
					errorValue = FMath::Min(errorThreshold, FMath::Abs(distance - LevelPerJumpDistThreshold[MapLevel]));
					newFitness += (errorValue * difficultyWeight);
				}
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
	
	tempPopulation.BestResultMap = tempPopulation.Population[0];
	CurrentPopulationInfo = tempPopulation;
}

FPopulationStruct AMapGeneratorBase::Crossover()
{
	return FPopulationStruct();
}

bool AMapGeneratorBase::Mutate(FMapInfoStruct& child)
{
	return false;
}

void AMapGeneratorBase::InitPlatformMeshPointInfo()
{
	const float dx[8] = { -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 };
	const float dy[8] = { 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0 };
	const float dz[8] = { 1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0 };
	const float d[8] = { -0.5, 0.5, 1.0, 1.0, 0.5, -0.5, -1.0, -1.0 };

	for (auto& mesh : PlatformMeshList)
	{
		if (mesh == nullptr) continue;

		FBox boxInfo = mesh->GetBoundingBox();
		FVector center = boxInfo.GetCenter();
		FVector extent = boxInfo.GetExtent().GetAbs();

		FPlatformPointStruct pointsInfo; 
		pointsInfo.BoxInfo = boxInfo;

		//윗면 모서리 추가 지점 구성
		int32 xIdx = 0, yIdx = 4;
		for (int32 dir = 0; dir < 8; dir++)
		{
			FVector point = center + FVector(extent.X + d[(xIdx + dir) % 8], extent.Y + dy[(yIdx + dir) % 8], 1.0f);
			pointsInfo.PointList.Add(point);
		}

		//꼭지점 구성
		for (int32 dir = 0; dir < 8; dir++)
		{
			FVector point = center + FVector(extent.X + dx[dir], extent.Y + dy[dir], extent.Z + dz[dir]);
			pointsInfo.PointList.Add(point);
		}

		PlatformMeshPointInfoMap.Add(mesh, pointsInfo);
	}
}

void AMapGeneratorBase::SetInitialPopulation(TArray<FMapInfoStruct> MapInfoList)
{
	for (int32 idx = 0; idx < PopulationSize; idx++)
	{
		CurrentPopulationInfo.BestResultMap = MapInfoList[idx % MapInfoList.Num()];
		CurrentPopulationInfo.Population.Add(MapInfoList[idx % MapInfoList.Num()]);
	}
}

float AMapGeneratorBase::GetNearestPoint(const FPlatformInfoStruct& P1, const FPlatformInfoStruct& P2, FVector& PosA, FVector& PosB)
{
	if (!PlatformMeshPointInfoMap.Contains(P1.PlatformStaticMesh)
		|| !PlatformMeshPointInfoMap.Contains(P2.PlatformStaticMesh)) return FLT_MAX;
	if (GetIsOverlapped(P1, P2)) return FLT_MAX;

	TArray<FVector> p1Points = PlatformMeshPointInfoMap[P1.PlatformStaticMesh].PointList;
	TArray<FVector> p2Points = PlatformMeshPointInfoMap[P2.PlatformStaticMesh].PointList;
	float nearDist = FLT_MAX;

	for (int32 p1Idx = 0; p1Idx < 12; p1Idx++)
	{
		for (int32 p2Idx = 0; p2Idx < 12; p2Idx++)
		{
			float dist = FVector::Dist(p1Points[p1Idx], p2Points[p2Idx]);
			if (dist < nearDist)
			{
				nearDist = dist;
				PosA = p1Points[p1Idx];
				PosB = p2Points[p2Idx];
			}
		}
	}
	return nearDist;
}

bool AMapGeneratorBase::GetCanReach(FVector StartVelocity, FVector Start, FVector End, float& ErrorDist, float GravityScale)
{
	// 원점 상대적으로 좌표 변환
	End = End - Start;
	End = { End.GetAbs().X, End.GetAbs().Y, End.Z};
	Start = FVector::ZeroVector;
	ErrorDist = 0.0f;

	//중력가속도 초기화
	GravityScale *= 980.0f;

	// 발사체의 초기 위치와 도착 지점 사이의 거리
	float DistanceToTarget = FVector::Dist(Start, End);

	// 최고 높이까지의 도달 시간을 계산
	float VerticalTime = StartVelocity.Z / (GravityScale * 0.5f);

	// 도달할 수 있는 가장 높은 지점
	float HighestHeight = FMath::Pow(StartVelocity.Z, 2) / (GravityScale * 2.0f) + 40.0f;

	// 수평 방향으로 이동할 거리 계산
	float HorizontalDistance = StartVelocity.X * VerticalTime;


	//UE_LOG(LogTemp, Warning, TEXT("h dist : %.2lf, hightest : %.2lf, time : %.2lf, gravity : %.2lf"), HorizontalDistance, HighestHeight, VerticalTime, GravityScale);
	//허용 가능한 오차 
	float tolerance = 5.0f;

	//고점보다 타겟의 Z축 좌표가 더 높거나, 타겟까지의 도달거리가 최종 도착 지점보다 멀다면 도달 불가
	if (HighestHeight < End.Z || DistanceToTarget > HorizontalDistance + tolerance)
	{
		ErrorDist = FMath::Max(0.0f, End.Z - HighestHeight) + FMath::Max(0.0f, DistanceToTarget - HorizontalDistance);
		return false;
	}

	float targetTime = DistanceToTarget / StartVelocity.X; //타겟까지의 도달 시간 (Z축 제외)
	float targetTimeVelocity = 0.0f; //타겟 좌표에서의 플레이어의 Z축 속도
	float targetTimeHeight = 0.0f; //타겟 좌표에서의 플레이어의 Z축 좌표

	//플레이어가 고점에 도달하는 지점 전에 타겟이 있다면
	if (targetTime < VerticalTime / 2.0f)
	{
		targetTimeVelocity = StartVelocity.Z - GravityScale * targetTime;
		targetTimeHeight = HighestHeight - (targetTimeVelocity * 0.5f * (VerticalTime / 2.0f - targetTime) * 0.5f);
	}
	//고점 도달 지점 이후에 타겟이 있다면
	else
	{
		targetTimeVelocity = StartVelocity.Z - GravityScale * targetTime;
		targetTimeHeight = HighestHeight + (targetTimeVelocity * 0.5f * FMath::Abs(targetTime - VerticalTime / 2.0f) * 0.5f);
	}

	//타겟의 2차원 좌표에 도달할때의 플레이어의 Z좌표가 타겟의 Z좌표보다 높다면?
	if (targetTimeHeight >= End.Z) return true; 

	ErrorDist = (End.Z - targetTimeHeight);
	return false;
}

// Function to check if a point is inside a cuboid defined by two points
bool AMapGeneratorBase::IsPointInsideCuboid(const FVector& PointA, const FVector& PointB, const FVector& TestPoint)
{
	// Calculate the minimum and maximum x, y, and z coordinates of the cuboid
	float minX = FMath::Min(PointA.X, PointB.X);
	float maxX = FMath::Max(PointA.X, PointB.X);
	float minY = FMath::Min(PointA.Y, PointB.Y);
	float maxY = FMath::Max(PointA.Y, PointB.Y);
	float minZ = FMath::Min(PointA.Z, PointB.Z);
	float maxZ = FMath::Max(PointA.Z, PointB.Z);

	// Check if the test point's coordinates are within the range of the cuboid
	return (TestPoint.X >= minX && TestPoint.X <= maxX &&
		TestPoint.Y >= minY && TestPoint.Y <= maxY &&
		TestPoint.Z >= minZ && TestPoint.Z <= maxZ);
}

bool AMapGeneratorBase::GetIsOverlapped(const FPlatformInfoStruct& P1, const FPlatformInfoStruct& P2)
{
	if (!PlatformMeshPointInfoMap.Contains(P1.PlatformStaticMesh)
		|| !PlatformMeshPointInfoMap.Contains(P2.PlatformStaticMesh)) return false;

	FBox p1Box = PlatformMeshPointInfoMap[P1.PlatformStaticMesh].BoxInfo;
	FBox p2Box = PlatformMeshPointInfoMap[P2.PlatformStaticMesh].BoxInfo;

	return p1Box.Intersect(p2Box);
}

void AMapGeneratorBase::Repair(FPopulationStruct& Result)
{
	
}