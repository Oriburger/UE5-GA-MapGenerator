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


	// ..

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
	GenBezierCurve(StartLoc, EndLoc, 30, 2);

	PlatformInfoResult.PlatformStaticMesh = MapGeneratorRef->PlatformMeshList[0];

	for (FVector& curvePoint : CurvePoints)
	{
		PlatformInfoResult.PlatformTransform = UKismetMathLibrary::MakeTransform(curvePoint, FRotator::ZeroRotator, FVector(1.0f));
		UE_LOG(LogTemp, Warning, TEXT("curve Point : %.2lf %.2lf %.2lf"), curvePoint.X, curvePoint.Y, curvePoint.Z);
		LastGenerateResult.PlatformInfoList.Add(PlatformInfoResult);
	}
	//LastGenerateResult = FMapInfoStruct();
	for (int32 populationSize = 0; populationSize < MapGeneratorRef->PopulationSize; populationSize++) 
	{
		LastGenerateResultList.Add(LastGenerateResult);
		//LastGenerateResultList[populationSize] = LastGenerateResult;
	}
}

FVector UInitialMapGeneratorComponent::GenControlPoint()
{
	FVector ControlPoint;

	float Rand_X = FMath::FRandRange(0.0f, 10000.0f);
	float Rand_Y = FMath::FRandRange(0.0f, 10000.0f);
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
		ControlPoints.Add(GenControlPoint());
		//ControlPoints[i] = GenControlPoint();
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
		for (int32 idx = 0; idx < GenPointNum; idx++) {
			float t = float(idx) / (GenPointNum - 1);
			float oneMinusT = 1.0 - t;
			float tSquared = t * t;
			float oneMinusTSquared = oneMinusT * oneMinusT;


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