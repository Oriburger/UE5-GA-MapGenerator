// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GeneratorInfoStruct.generated.h"

//Node & Gene
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

//Edge Info
USTRUCT(BlueprintType)
struct FEdgeStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//시작 정점 ID
	UPROPERTY(BlueprintReadWrite)
		int32 StartNodeID; 

	//StartNodeID에서 시작하는 모든 정점 목록
	UPROPERTY(BlueprintReadWrite)
		TArray<int32> AdjacencyList;

	//직선 거리
	UPROPERTY(BlueprintReadWrite)
		TArray<float> DistanceList;

	//스폰될 메시 
	UPROPERTY(BlueprintReadWrite)
		TArray<UStaticMesh*> StaticMeshList;

	//스폰할 메시 트랜스폼(월드)
	UPROPERTY(BlueprintReadWrite)
		TArray<FTransform> TransformList;
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
	//반드시 정점 개수만큼 초기화해서 쓸 것.
	//EdgeInfoList[0].AdjacencyList  --> 0과 연결된 모든 정점 목록
	//EdgeInfoList[1].DistanceList[0];  --> 1과 연결된 첫번째 정점까지의 거리
	//양방향이기에 반대도 무조건 처리해야함.
	UPROPERTY(BlueprintReadWrite)
		TArray<FEdgeStruct>	EdgeInfoList;

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