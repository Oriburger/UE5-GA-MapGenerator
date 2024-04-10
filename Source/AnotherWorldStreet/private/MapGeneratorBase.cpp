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

		//~~ ���� ~~
		SelectParents();

		//~~ �н� ���� ~~
		//Crossover();
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
	const float outOfRangeWeight = 0.6f; //�÷����� �Ķ���ͷ� ������ ���� �ܺο� �ִ����� ���� ����ġ
	const float outOfRangeValue = 99999.0f; 
	const float unreachableWeight = 0.35f; //�÷��̾ ���� �Ұ����� �÷����� �ִ����� ���� ����ġ
	const float difficultyWeight = 0.05f; //���̵��� �������� �����Ϳ� ���� ����ġ

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
				newFitness += outOfRangeWeight * outOfRangeValue;
			}
			
			if (platformIdx > 0)
			{
				FPlatformInfoStruct prev = currentMap.PlatformInfoList[platformIdx - 1];
				FVector prevLocation = prev.PlatformTransform.GetLocation();
				float distance = UKismetMathLibrary::Vector_Distance(prevLocation, currLocation);

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

FPopulationStruct AMapGeneratorBase::Crossover()
{
	return FPopulationStruct();
}

bool AMapGeneratorBase::Mutate(FMapInfoStruct& child)
{
	return false;
}

void AMapGeneratorBase::SetInitialPopulation(TArray<FMapInfoStruct> MapInfoList)
{
	for (int32 idx = 0; idx < PopulationSize; idx++)
	{
		CurrentPopulationInfo.BestResultMap = MapInfoList[idx % MapInfoList.Num()];
		CurrentPopulationInfo.Population.Add(MapInfoList[idx % MapInfoList.Num()]);
	}
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

void AMapGeneratorBase::Repair(FPopulationStruct& Result)
{
	
}