// Fill out your copyright notice in the Description page of Project Settings.


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


	// ..
	AMapGeneratorBase MapGeneratorBase;
	StartLoc = MapGeneratorBase.StartLocation;
	EndLoc = MapGeneratorBase.EndLocation;

	checkf(IsValid(MapGeneratorBase.StaticMeshList[0]), TEXT("StaticMesh 탐색에 실패했습니다."));

}

// Called when the game starts
void UInitialMapGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UInitialMapGeneratorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInitialMapGeneratorComponent::Generate_Implementation()
{
	AMapGeneratorBase MapGeneratorBase;

	//UKismetMathLibrary::MakeTransform()
	/*
	for (FVector& CurvePoint : CurvePoints)
	{

	}

	PlatformInfoResult.PlatformStaticMesh = MapGeneratorBase.StaticMeshList[0];
	PlatformInfoResult.PlatformTransform = 

	LastGenerateResult.PlatformInfoList.Add(PlatformInfoResult)
	*/

}

FVector UInitialMapGeneratorComponent::GenControlPoint()
{
	FVector ControlPoint;

	float Rand_X = FMath::FRandRange(5000.0f, 10000.0f);
	float Rand_Y = FMath::FRandRange(5000.0f, 10000.0f);
	float Rand_Z = FMath::FRandRange(1000.0f, 1500.0f);

	ControlPoint.X = Rand_X;
	ControlPoint.Y = Rand_Y;
	ControlPoint.Z = Rand_Z;

	return ControlPoint;
}

void UInitialMapGeneratorComponent::GenBezierCurve(FVector Start, FVector End, int32 GenPointNum, int32 ControlPointNum)
{
	FVector ResultCurve;

	for (int32 i = 0; i < ControlPointNum; i++)
	{
		ControlPoints[i] = GenControlPoint();
	}

	//Control Point가 한개일때
	/*
	if(ControlPointNum == 1) 
	{

	}
	*/

	//Control Point가 한개일때
	if(ControlPointNum == 2) 
	{
		for (int32 j = 0; j < GenPointNum; j++) {
			double t = j / (GenPointNum - 1);
			double oneMinusT = 1.0 - t;
			double tSquared = t * t;
			double oneMinusTSquared = oneMinusT * oneMinusT;


			ResultCurve.X = oneMinusTSquared*oneMinusT*Start.X +
				3.0*oneMinusTSquared*t*ControlPoints[0].X +
				3.0*oneMinusT*tSquared*ControlPoints[1].X + 
				tSquared*t*End.X;

			ResultCurve.Y = oneMinusTSquared * oneMinusT * Start.Y +
				3.0 * oneMinusTSquared * t * ControlPoints[0].Y +
				3.0 * oneMinusT * tSquared * ControlPoints[1].Y +
				tSquared * t * End.Y;

			ResultCurve.Z = oneMinusTSquared * oneMinusT * Start.Z +
				3.0 * oneMinusTSquared * t * ControlPoints[0].Z +
				3.0 * oneMinusT * tSquared * ControlPoints[1].Z +
				tSquared * t * End.Z;

			CurvePoints.Add(ResultCurve);
		}
	}
}