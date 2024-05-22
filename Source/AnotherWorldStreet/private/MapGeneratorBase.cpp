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

	StartPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartPoint"));
	StartPoint->SetupAttachment(RootComponent);
	StartPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StartPoint->bHiddenInGame = true;

	MidPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MidPoint"));
	MidPoint->SetupAttachment(RootComponent);
	MidPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MidPoint->bHiddenInGame = true;

	EndPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndPoint"));
	EndPoint->SetupAttachment(RootComponent);
	EndPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EndPoint->bHiddenInGame = true;	
}

// Called when the game starts or when spawned
void AMapGeneratorBase::BeginPlay()
{
	Super::BeginPlay();
	
	StartLocation = StartPoint->GetComponentLocation();
	MidLocation = MidPoint->GetComponentLocation();
	EndLocation = EndPoint->GetComponentLocation();

	PlayerRef = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(PlayerRef))
	{
		JumpVelocity.X = PlayerRef->GetCharacterMovement()->MaxWalkSpeed;
		JumpVelocity.Z = PlayerRef->GetCharacterMovement()->JumpZVelocity;
		GravityMultipiler = PlayerRef->GetCharacterMovement()->GravityScale;
	}	
	UE_LOG(LogTemp, Warning, TEXT("Start : %s  /  Mid : %s  /  End : %s"), *StartLocation.ToString(), *MidLocation.ToString(), *EndLocation.ToString());

	InitPlatformMeshPointInfo();
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

		DrawDebugSphere(GetWorld(), StartLocation, 50.0f, 12, FColor::Red, false, 10.0f, 1u, 10.0f);
		DrawDebugSphere(GetWorld(), MidLocation, 50.0f, 12, FColor::Green, false, 10.0f, 1u, 10.0f);
		DrawDebugSphere(GetWorld(), EndLocation, 50.0f, 12, FColor::Blue, false, 10.0f, 1u, 10.0f);

		UE_LOG(LogTemp, Warning, TEXT("-----End-----------"));
		UE_LOG(LogTemp, Warning, TEXT("Start : %s"), *StartLocation.ToString());
		UE_LOG(LogTemp, Warning, TEXT("Mid : %s"), *MidLocation.ToString());
		UE_LOG(LogTemp, Warning, TEXT("End : %s"), *EndLocation.ToString());
	}
	//=== 학습 세대 =======================
	else if(CurrentGen > 0)
	{
		//~~ 적합도 연산 ~~
		CurrentPopulationInfo.BestFitnessValue = CalculateFitness();
		UE_LOG(LogTemp, Warning, TEXT("CurrentGen : %d, Threshold : %d, Fitness : %.2lf"), CurrentGen, GenerationThresold, CurrentPopulationInfo.BestFitnessValue);
		BestFitnessValue = FMath::Min(BestFitnessValue, CurrentPopulationInfo.BestFitnessValue);

		//~~ 선택 ~~
		SelectParents();

		//~~ 교차 및 Mutate~~
		TArray<FMapInfoStruct> newChildList;
		for (int32 cIdx = 1; cIdx < CurrentPopulationInfo.Population.Num() - 2; cIdx++)
		{
			int32 tIdx = UKismetMathLibrary::RandomIntegerInRange(cIdx + 1, CurrentPopulationInfo.Population.Num() - 3);
			FMapInfoStruct newChild = Crossover(CurrentPopulationInfo.Population[cIdx], CurrentPopulationInfo.Population[tIdx]);
			
			if (UKismetMathLibrary::RandomFloat() <= MutatePossibility)
			{
				Mutate(newChild);
			}
			if (bIsRepairActive)
			{
				Repair(newChild);
			}
			newChildList.Add(newChild);
		}
		for (auto& child : newChildList)
		{
			CurrentPopulationInfo.Population.Add(child);
		}

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

	const float outOfRangeValue = 999.0f;
	const float overlappedValue = 200.0f;
	const float unreachableValue = 99.0f;

	float bestFitness = FLT_MAX;
	float avgFitness = 0.0f;

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
				newFitness += (errorValue * unreachableWeight * unreachableValue);

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
		currentMap.TotalFitness = newFitness;
		avgFitness += newFitness;
		bestFitness = FMath::Min(bestFitness, currentMap.TotalFitness);
	}
	UE_LOG(LogTemp, Warning, TEXT("Avg : %.2lf"), avgFitness / CurrentPopulationInfo.Population.Num());
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

FMapInfoStruct AMapGeneratorBase::Crossover(const FMapInfoStruct& G1, const FMapInfoStruct& G2)
{
	FMapInfoStruct childMap;
	int32 g1Idx = 0, g2Idx = 0;

	for (; g1Idx < G1.PlatformInfoList.Num(); g1Idx++)
	{
		FPlatformInfoStruct platform = G1.PlatformInfoList[g1Idx];
		childMap.PlatformInfoList.Add(platform);
		if (platform.PlatformTransform.GetLocation().Equals(MidLocation, 0.0f)) break;
	}
	
	bool bIsFound = false;
	for (; g2Idx < G2.PlatformInfoList.Num(); g2Idx++)
	{
		FPlatformInfoStruct platform = G2.PlatformInfoList[g2Idx];
		if (platform.PlatformTransform.GetLocation().Equals(MidLocation, 0.0f))
		{
			bIsFound = true;
		}
		if (bIsFound)
		{
			childMap.PlatformInfoList.Add(platform);
		}
	}
	if (!bIsFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("NOT FOUND!"));
	}
	return childMap;
}

bool AMapGeneratorBase::Mutate(FMapInfoStruct& child)
{
	float mutateLength = child.PlatformInfoList.Num() * UKismetMathLibrary::RandomFloatInRange(0.0f, MaxMutationRate);
	int32 mutateStartIdx = UKismetMathLibrary::RandomIntegerInRange(1, child.PlatformInfoList.Num() - (int32)mutateLength - 2);
	int32 mutateFinishIdx = mutateStartIdx + (int32)mutateLength; 
	FVector midPoint = (StartLocation + EndLocation) / 2.0f;

	for (int32 idx = mutateStartIdx; idx <= mutateFinishIdx; idx++)
	{
		if (!child.PlatformInfoList.IsValidIndex(idx)) continue;
		if (child.PlatformInfoList[idx].PlatformTransform.GetLocation() == midPoint) continue;

		FVector location = child.PlatformInfoList[idx].PlatformTransform.GetLocation();
		if (location == MidLocation || location == StartLocation || location == EndLocation) continue;

		FVector extent = child.PlatformInfoList[idx].PlatformStaticMesh->GetBounds().BoxExtent;

		location.X += UKismetMathLibrary::RandomFloatInRange(-1 * extent.X * MutationStrength, extent.X * MutationStrength);
		location.Y += UKismetMathLibrary::RandomFloatInRange(-1 * extent.Y * MutationStrength, extent.Y * MutationStrength);
		location.Z += UKismetMathLibrary::RandomFloatInRange(-1 * extent.Z * MutationStrength, extent.Z * MutationStrength);
		child.PlatformInfoList[idx].PlatformTransform.SetLocation(location);
	}

	return false;
}

void AMapGeneratorBase::Repair(FMapInfoStruct& Result)
{
	TArray<FPlatformInfoStruct> additionalPlatforms;
	
	//모든 플랫폼을 탐색하면서 수선한다.
	for (int32 idx = 0; idx < Result.PlatformInfoList.Num()-2; idx++)
	{
		FPlatformInfoStruct& curr = Result.PlatformInfoList[idx];
		FPlatformInfoStruct& next = Result.PlatformInfoList[idx+1];
		const FVector currLocation = curr.PlatformTransform.GetLocation();
		const FVector nextLocation = next.PlatformTransform.GetLocation();
		const FVector midPoint = (StartLocation + EndLocation) / 2.0f;
		float errorDist = 0.0f;
		
		bool result = GetCanReach(JumpVelocity, currLocation, nextLocation, errorDist, GravityMultipiler);
		//만약 도달이 불가능하다면, 그 사이를 선형으로 채운다.
		if (!result)
		{
			//선형으로 채우기에 가까운 거리라면 채운다.
			if (errorDist <= 1500.0f)
			{
				FVector direction = nextLocation - currLocation;
				float length = direction.Length();
				int32 count = length / 500.0f; //300.0f는 도달 가능한 임의의 거리

				direction = direction / count;
				for (int32 dirCnt = 1; dirCnt < count; dirCnt++)
				{
					FPlatformInfoStruct newPlatformInfo;
					newPlatformInfo.PlatformStaticMesh = PlatformMeshList[UKismetMathLibrary::RandomInteger(PlatformMeshList.Num())];
					newPlatformInfo.PlatformTransform.SetLocation(currLocation + direction * dirCnt);
					additionalPlatforms.Add(newPlatformInfo);
				}
			}
			//그렇지 않다면 이전과 다음 플랫폼의 중간 지점으로 설정한다.
			else if(idx > 0)
			{
				FPlatformInfoStruct& prev = Result.PlatformInfoList[idx - 1];
				const FVector prevLocation = curr.PlatformTransform.GetLocation();
				
				if (nextLocation != prevLocation && currLocation != MidLocation
					&& currLocation != StartLocation && currLocation != EndLocation){
					curr.PlatformTransform.SetLocation((nextLocation + prevLocation) / 2.0f);
				}
			}
		}
	}
	//Result의 플랫폼 리스트에 새로 추가될 플랫폼 정보를 추가한다.
	for (auto& platform : additionalPlatforms)
	{
		Result.PlatformInfoList.Add(platform);
	}

	//새로이 정렬한다.
	Result.PlatformInfoList.Sort([](const FPlatformInfoStruct& p1, const FPlatformInfoStruct& p2) {
			return (p1.PlatformTransform.GetLocation().X < p2.PlatformTransform.GetLocation().X
				? p1.PlatformTransform.GetLocation().X < p2.PlatformTransform.GetLocation().X
				: ((p1.PlatformTransform.GetLocation().Y < p2.PlatformTransform.GetLocation().Y)
					? p1.PlatformTransform.GetLocation().Y < p2.PlatformTransform.GetLocation().Y
					: (p1.PlatformTransform.GetLocation().Z < p2.PlatformTransform.GetLocation().Z)));
			});
	
	//겹치는 것을 제거한다.
	TArray<bool> removeIdx;
	removeIdx.Init(false, Result.PlatformInfoList.Num());
	for (int32 idx = 1; idx < Result.PlatformInfoList.Num() - 2; idx++)
	{
		FPlatformInfoStruct& curr = Result.PlatformInfoList[idx];
		FPlatformInfoStruct& next = Result.PlatformInfoList[idx + 1];
		const FVector currLocation = curr.PlatformTransform.GetLocation();
		const FVector nextLocation = next.PlatformTransform.GetLocation();

		bool result = GetIsOverlapped(curr, next);
		
		//겹치면 제거한다
		if (result)
		{
			if (idx == Result.PlatformInfoList.Num() - 1) continue;
			else if (currLocation == MidLocation || currLocation == StartLocation || currLocation == EndLocation)
			{
				Result.PlatformInfoList.RemoveAt(idx+1);
				idx += 1;
				continue;
			}
			else Result.PlatformInfoList.RemoveAt(idx);
			idx -= 1;
		}
	}
}

void AMapGeneratorBase::InitPlatformMeshPointInfo()
{
	const float dx[8] = { -1.0, -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0 };
	const float dy[8] = { -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0 };
	const float dz[8] = { -2.0, 1.0, 1.0, 1.0, -2.0, -2.0, -2.0, 1.0 };
	const float d[12] = { -0.5, 0.0, 0.5, 1.0, 1.0, 1.0, 0.5, 0.0, -0.5, -1.0, -1.0, -1.0 };

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
		for (int32 dir = 0; dir < 12; dir++)
		{
			FVector point = center + FVector(extent.X * d[(xIdx + dir) % 12], extent.Y * dy[(yIdx + dir) % 12], extent.Z);
			pointsInfo.AdditionalPointList.Add(point);
		}

		//꼭지점 구성
		for (int32 dir = 0; dir < 8; dir++)
		{
			FVector point = center + FVector(extent.X * dx[dir], extent.Y * dy[dir], extent.Z * dz[dir]);
			pointsInfo.PointList.Add(point);
		}

		PlatformMeshPointInfoMap.Add(mesh, pointsInfo);
	}
}

void AMapGeneratorBase::SetInitialPopulation(TArray<FMapInfoStruct> MapInfoList)
{
	FPlatformInfoStruct startInfo, finInfo, midInfo;
	startInfo.PlatformStaticMesh = midInfo.PlatformStaticMesh = finInfo.PlatformStaticMesh = PlatformMeshList[0];
	startInfo.PlatformTransform.SetLocation(StartLocation);
	midInfo.PlatformTransform.SetLocation(MidLocation);
	finInfo.PlatformTransform.SetLocation(EndLocation);
	for (int32 idx = 0; idx < PopulationSize; idx++)
	{
		FMapInfoStruct& targetMap = MapInfoList[idx % MapInfoList.Num()];
		//~~ 오름차순 정렬 ~~
		targetMap.PlatformInfoList.Add(startInfo);
		targetMap.PlatformInfoList.Add(midInfo);
		targetMap.PlatformInfoList.Add(finInfo);
		targetMap.PlatformInfoList.Sort([](const FPlatformInfoStruct& p1, const FPlatformInfoStruct& p2) {
			return (p1.PlatformTransform.GetLocation().X < p2.PlatformTransform.GetLocation().X
				   ? p1.PlatformTransform.GetLocation().X < p2.PlatformTransform.GetLocation().X
				   : ((p1.PlatformTransform.GetLocation().Y < p2.PlatformTransform.GetLocation().Y)
					  ? p1.PlatformTransform.GetLocation().Y < p2.PlatformTransform.GetLocation().Y
					  : (p1.PlatformTransform.GetLocation().Z < p2.PlatformTransform.GetLocation().Z)));
		});
		CurrentPopulationInfo.BestResultMap = targetMap;
		CurrentPopulationInfo.Population.Add(targetMap);
	}

	UE_LOG(LogTemp, Warning, TEXT("-----End-----------"));
	UE_LOG(LogTemp, Warning, TEXT("Start : %s"), *StartLocation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Mid : %s"), *MidLocation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("End : %s"), *EndLocation.ToString());
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

	TArray<FVector> p1Box = PlatformMeshPointInfoMap[P1.PlatformStaticMesh].PointList;
	TArray<FVector> p2Box = PlatformMeshPointInfoMap[P2.PlatformStaticMesh].PointList;
	
	const FVector p2BeginPos = p2Box[0] + P2.PlatformTransform.GetLocation();
	const FVector p2EndPos = p2Box[7] + P2.PlatformTransform.GetLocation();


	for (int32 idxA = 0; idxA < 8; idxA++)
	{
		FVector p1Pos = p1Box[idxA] + P1.PlatformTransform.GetLocation();
		
		if (IsPointInsideCuboid(p2BeginPos, p2EndPos, p1Pos))
		{
			//UE_LOG(LogTemp, Warning, TEXT("p2: {%s, %s}, p1 : %s"), *p2BeginPos.ToString(), *p2EndPos.ToString(), *p1Pos.ToString());
			return true;
		}
	}
	
	return false;
}