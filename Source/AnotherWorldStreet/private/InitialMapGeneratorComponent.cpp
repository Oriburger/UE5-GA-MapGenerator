// Fill out your copyright notice in the Description page of Proidxect Settings.


#include "../public/InitialMapGeneratorComponent.h"
#include "../public/MapGeneratorBase.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UInitialMapGeneratorComponent::UInitialMapGeneratorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	//...
}

// Called when the game starts
void UInitialMapGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

	MapGeneratorRef = Cast<AMapGeneratorBase>(GetOwner());
	//checkf(IsValid(MapGeneratorRef->StaticMeshList[0]), TEXT("StaticMesh 탐색에 실패했습니다."));

	if (IsValid(MapGeneratorRef))
	{
		StartLoc = MapGeneratorRef->StartLocation;
		EndLoc = MapGeneratorRef->EndLocation;
	}
}

// Called every frame
void UInitialMapGeneratorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInitialMapGeneratorComponent::Generate_Implementation()
{
	FVector mid = FMath::Abs(EndLoc- StartLoc) / 2;

	for (int32 populationSize = 0; populationSize < MapGeneratorRef->PopulationSize; populationSize++) 
	{

		GenBezierCurve(StartLoc, mid, 15, 2);
		GenBezierCurve(mid, EndLoc, 15, 2);

		PlatformInfoResult.PlatformStaticMesh = MapGeneratorRef->PlatformMeshList[0];

		for (FVector& curvePoint : CurvePoints)
		{
			PlatformInfoResult.PlatformTransform = UKismetMathLibrary::MakeTransform(curvePoint, FRotator::ZeroRotator, FVector(1.0f));
			UE_LOG(LogTemp, Warning, TEXT("curve Point : %.2lf %.2lf %.2lf"), curvePoint.X, curvePoint.Y, curvePoint.Z);
			LastGenerateResult.PlatformInfoList.Add(PlatformInfoResult);
		}
		CurvePoints.Empty();
		LastGenerateResultList.Add(LastGenerateResult);
	}
}

FVector UInitialMapGeneratorComponent::GenControlPoint(FVector start, FVector end)
{
	FVector ControlPoint;
	float xRange = FMath::Abs(end.X - start.X);
	float yRange = FMath::Abs(end.Y - start.Y);
	float zRange = FMath::Abs(end.Z - start.Z);


	float randX = FMath::FRandRange(start.X, xRange);
	float randY = FMath::FRandRange(start.Y, yRange);
	float randZ = FMath::FRandRange(start.Z, zRange);

	ControlPoint.X = randX;
	ControlPoint.Y = randY;
	ControlPoint.Z = randZ;

	return ControlPoint;
}

void UInitialMapGeneratorComponent::GenBezierCurve(FVector start, FVector end, int32 genPointNum, int32 controlPointNum)
{
	FVector ResultCurve;

	for (int32 i = 0; i < controlPointNum; i++)
	{
		ControlPoints.Add(GenControlPoint(start, end));
		//ControlPoints[i] = GenControlPoint();
	}

	//Control Point가 한개일때
	/*
	if(ControlPointNum == 1) 
	{

	}
	*/

	//Control Point가 한개일때
	if(controlPointNum == 2) 
	{
		for (int32 idx = 0; idx < genPointNum; idx++) {
			float t = float(idx) / (genPointNum - 1);
			float oneMinusT = 1.0 - t;
			float tSquared = t * t;
			float oneMinusTSquared = oneMinusT * oneMinusT;


			ResultCurve.X = oneMinusTSquared*oneMinusT*start.X +
				3.0*oneMinusTSquared*t*ControlPoints[0].X +
				3.0*oneMinusT*tSquared*ControlPoints[1].X + 
				tSquared*t*end.X;

			ResultCurve.Y = oneMinusTSquared * oneMinusT * start.Y +
				3.0 * oneMinusTSquared * t * ControlPoints[0].Y +
				3.0 * oneMinusT * tSquared * ControlPoints[1].Y +
				tSquared * t * end.Y;

			ResultCurve.Z = oneMinusTSquared * oneMinusT * start.Z +
				3.0 * oneMinusTSquared * t * ControlPoints[0].Z +
				3.0 * oneMinusT * tSquared * ControlPoints[1].Z +
				tSquared * t * end.Z;

			CurvePoints.Add(ResultCurve);
		}
	}
}