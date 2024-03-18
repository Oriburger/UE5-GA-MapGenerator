// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GeneratorInfoStruct.generated.h"

//Gene
USTRUCT(BlueprintType)
struct FPlatformInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
		UStaticMesh* PlatformStaticMesh;

	UPROPERTY(BlueprintReadWrite)
		FTransform PlatformTransform;

	//UPROPERTY(BlueprintReadWrite)
		//	TArray<FObstacleInfo> ObstacleInfoList; 
};

//Chromosome
USTRUCT(BlueprintType)
struct FMapInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//플랫폼 정보 리스트 (정점 리스트)
	UPROPERTY(BlueprintReadWrite)
		TArray<FPlatformInfoStruct> PlatformInfoList;

	//간선 정보 리스트 (잇는 스태틱 메시와 그것의 Transform)
	UPROPERTY(BlueprintReadWrite)
		TArray<FPlatformInfoStruct>	EdgeInfoList;

	//총 Fitness 값
	UPROPERTY(BlueprintReadWrite)
		float TotalFitness = 0.0f;
};

USTRUCT(BlueprintType)
struct FPopulationStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//유전자 집단 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		TArray<FMapInfoStruct> Population;

	//현재까지의 최고 결과 맵
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		FMapInfoStruct BestResultMap;

	//위 맵의 FitnessValue
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		float BestFitnessValue;

	//적합도 평균
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		float FitnessAverage;
};