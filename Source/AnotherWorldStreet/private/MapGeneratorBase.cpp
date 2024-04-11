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
		CurrentPopulationInfo.BestFitnessValue = CalculateFitness();
		UE_LOG(LogTemp, Warning, TEXT("CurrentGen : %d, Threshold : %d, Fitness : %.2lf"), CurrentGen, GenerationThresold, CurrentPopulationInfo.BestFitnessValue);
		BestFitnessValue = FMath::Min(BestFitnessValue, CurrentPopulationInfo.BestFitnessValue);

		//~~ ���� ~~
		SelectParents();

		//~~ ���� �� Mutate~~
		TArray<FMapInfoStruct> newChildList;
		for (int32 cIdx = 0; cIdx < CurrentPopulationInfo.Population.Num() - 1; cIdx++)
		{
			int32 tIdx = UKismetMathLibrary::RandomIntegerInRange(cIdx + 1, CurrentPopulationInfo.Population.Num() - 1);
			FMapInfoStruct newChild = Crossover(CurrentPopulationInfo.Population[cIdx], CurrentPopulationInfo.Population[tIdx]);
			
			if (UKismetMathLibrary::RandomFloat() <= MutatePossibility)
			{
				Mutate(newChild);
			}
			newChildList.Add(newChild);
		}
		for (auto& child : newChildList)
		{
			CurrentPopulationInfo.Population.Add(child);
		}

		//Repair()
		

		//���븦 �ϳ� �ø���, ���൵ ������Ʈ �̺�Ʈ�� Ȱ��ȭ��Ų��.
		CurrentGen += 1;
		OnGAUpdatedProgress.Broadcast(CurrentGen, (float)CurrentGen/(float)GenerationThresold, BestFitnessValue);

		//���� ������ �н��� ���� ƽ���� �����Ѵ�.
		GetWorldTimerManager().SetTimer(GenerationDelayHandle, this, &AMapGeneratorBase::RunGeneticAlgorithm, 0.001f, false);	
	}
}

float AMapGeneratorBase::CalculateFitness()
{
	//���յ��� �ݵ�� ���ƾ��Ѵ�. ���� ���� ��Ȳ���� �� ���ϴ� ���
	//���յ� ��� ����ġ (���� 1), ���� ����ġ = �ݵ�� ���������ϴ� ��
	const float outOfRangeWeight = 0.4f; //�÷����� �Ķ���ͷ� ������ ���� �ܺο� �ִ����� ���� ����ġ
	const float overlappedWeight = 0.2f; //��ġ�� �÷����� �ִ����� ���� ����ġ
	const float unreachableWeight = 0.35f; //�÷��̾ ���� �Ұ����� �÷����� �ִ����� ���� ����ġ
	const float difficultyWeight = 0.05f; //���̵��� �������� �����Ϳ� ���� ����ġ

	const float outOfRangeValue = 999.0f;
	const float overlappedValue = 200.0f;

	float bestFitness = FLT_MAX;
	float avgFitness = 0.0f;

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
				newFitness += outOfRangeWeight * outOfRangeValue;
			}
			
			if (platformIdx > 0)
			{
				FPlatformInfoStruct prev = currentMap.PlatformInfoList[platformIdx - 1];
				FVector prevLocation = prev.PlatformTransform.GetLocation();
				float distance = UKismetMathLibrary::Vector_Distance(prevLocation, currLocation);
				
				//��ġ�� �÷����� �ִ���?
				newFitness += (GetIsOverlapped(curr, prev) ? overlappedValue : 0.0f);

				//���� �Ұ����� ��尡 �ִ���?
				float errorValue = 0.0f;
				bool result = GetCanReach(JumpVelocity, prevLocation, currLocation, errorValue, GravityMultipiler);
				newFitness += (errorValue * unreachableWeight);

				//���̵��� �󸶳� ���̰� ������?
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
	
	tempPopulation.BestResultMap = tempPopulation.Population[0];
	CurrentPopulationInfo = tempPopulation;
}

FMapInfoStruct AMapGeneratorBase::Crossover(const FMapInfoStruct& G1, const FMapInfoStruct& G2)
{
	FMapInfoStruct childMap;
	FVector midPoint = (StartLocation + EndLocation) / 2.0f;
	int32 g1Idx = 0, g2Idx = 0;

	for (; g1Idx < G1.PlatformInfoList.Num(); g1Idx++)
	{
		FPlatformInfoStruct platform = G1.PlatformInfoList[g1Idx];
		childMap.PlatformInfoList.Add(platform);
		if (platform.PlatformTransform.GetLocation() == midPoint) break;
	}
	
	bool bIsFound = false;
	for (; g2Idx < G2.PlatformInfoList.Num(); g2Idx++)
	{
		FPlatformInfoStruct platform = G2.PlatformInfoList[g2Idx];
		if (platform.PlatformTransform.GetLocation() == midPoint)
		{
			bIsFound = true;
			continue;
		}
		if (bIsFound)
		{
			childMap.PlatformInfoList.Add(platform);
		}
	}
	return childMap;
}

bool AMapGeneratorBase::Mutate(FMapInfoStruct& child)
{
	float mutateLength = child.PlatformInfoList.Num() * UKismetMathLibrary::RandomFloatInRange(0.0f, MaxMutationRate);
	int32 mutateStartIdx = UKismetMathLibrary::RandomIntegerInRange(0, child.PlatformInfoList.Num() - (int32)mutateLength - 1);
	int32 mutateFinishIdx = mutateStartIdx + (int32)mutateLength; 

	for (int32 idx = mutateStartIdx; idx <= mutateFinishIdx; idx++)
	{
		FVector location = child.PlatformInfoList[idx].PlatformTransform.GetLocation();
		location.X += UKismetMathLibrary::RandomFloatInRange(0.0f, 1000.0f);
		location.Y += UKismetMathLibrary::RandomFloatInRange(0.0f, 1000.0f);
		location.Z += UKismetMathLibrary::RandomFloatInRange(0.0f, 1000.0f);
		child.PlatformInfoList[idx].PlatformTransform.SetLocation(location);
	}

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

		//���� �𼭸� �߰� ���� ����
		int32 xIdx = 0, yIdx = 4;
		for (int32 dir = 0; dir < 8; dir++)
		{
			FVector point = center + FVector(extent.X + d[(xIdx + dir) % 8], extent.Y + dy[(yIdx + dir) % 8], 1.0f);
			pointsInfo.PointList.Add(point);
		}

		//������ ����
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
		FMapInfoStruct& targetMap = MapInfoList[idx % MapInfoList.Num()];
		//~~ �������� ���� ~~
		targetMap.PlatformInfoList.Sort([](const FPlatformInfoStruct& p1, const FPlatformInfoStruct& p2) {
			return (p1.PlatformTransform.GetLocation().X > p2.PlatformTransform.GetLocation().X
				   ? p1.PlatformTransform.GetLocation().X > p2.PlatformTransform.GetLocation().X
				   : ((p1.PlatformTransform.GetLocation().Y > p2.PlatformTransform.GetLocation().Y)
					  ? p1.PlatformTransform.GetLocation().Y > p2.PlatformTransform.GetLocation().Y
					  : (p1.PlatformTransform.GetLocation().Z > p2.PlatformTransform.GetLocation().Z)));
		});
		CurrentPopulationInfo.BestResultMap = targetMap;
		CurrentPopulationInfo.Population.Add(targetMap);
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
	// ���� ��������� ��ǥ ��ȯ
	End = End - Start;
	End = { End.GetAbs().X, End.GetAbs().Y, End.Z};
	Start = FVector::ZeroVector;
	ErrorDist = 0.0f;

	//�߷°��ӵ� �ʱ�ȭ
	GravityScale *= 980.0f;

	// �߻�ü�� �ʱ� ��ġ�� ���� ���� ������ �Ÿ�
	float DistanceToTarget = FVector::Dist(Start, End);

	// �ְ� ���̱����� ���� �ð��� ���
	float VerticalTime = StartVelocity.Z / (GravityScale * 0.5f);

	// ������ �� �ִ� ���� ���� ����
	float HighestHeight = FMath::Pow(StartVelocity.Z, 2) / (GravityScale * 2.0f) + 40.0f;

	// ���� �������� �̵��� �Ÿ� ���
	float HorizontalDistance = StartVelocity.X * VerticalTime;


	//UE_LOG(LogTemp, Warning, TEXT("h dist : %.2lf, hightest : %.2lf, time : %.2lf, gravity : %.2lf"), HorizontalDistance, HighestHeight, VerticalTime, GravityScale);
	//��� ������ ���� 
	float tolerance = 5.0f;

	//�������� Ÿ���� Z�� ��ǥ�� �� ���ų�, Ÿ�ٱ����� ���ްŸ��� ���� ���� �������� �ִٸ� ���� �Ұ�
	if (HighestHeight < End.Z || DistanceToTarget > HorizontalDistance + tolerance)
	{
		ErrorDist = FMath::Max(0.0f, End.Z - HighestHeight) + FMath::Max(0.0f, DistanceToTarget - HorizontalDistance);
		return false;
	}

	float targetTime = DistanceToTarget / StartVelocity.X; //Ÿ�ٱ����� ���� �ð� (Z�� ����)
	float targetTimeVelocity = 0.0f; //Ÿ�� ��ǥ������ �÷��̾��� Z�� �ӵ�
	float targetTimeHeight = 0.0f; //Ÿ�� ��ǥ������ �÷��̾��� Z�� ��ǥ

	//�÷��̾ ������ �����ϴ� ���� ���� Ÿ���� �ִٸ�
	if (targetTime < VerticalTime / 2.0f)
	{
		targetTimeVelocity = StartVelocity.Z - GravityScale * targetTime;
		targetTimeHeight = HighestHeight - (targetTimeVelocity * 0.5f * (VerticalTime / 2.0f - targetTime) * 0.5f);
	}
	//���� ���� ���� ���Ŀ� Ÿ���� �ִٸ�
	else
	{
		targetTimeVelocity = StartVelocity.Z - GravityScale * targetTime;
		targetTimeHeight = HighestHeight + (targetTimeVelocity * 0.5f * FMath::Abs(targetTime - VerticalTime / 2.0f) * 0.5f);
	}

	//Ÿ���� 2���� ��ǥ�� �����Ҷ��� �÷��̾��� Z��ǥ�� Ÿ���� Z��ǥ���� ���ٸ�?
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