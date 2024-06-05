// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GeneratorInfoStruct.generated.h"

UENUM(BlueprintType)enum class EPlatformType : uint8
{
	E_None			UMETA(DisplayName = "None"), 
	E_Start			UMETA(DisplayName = "Start"), 
	E_Mid			UMETA(DisplayName = "Mid"),
	E_End			UMETA(DisplayName = "End"),
};

//Node & Gene
USTRUCT(BlueprintType)
struct FPlatformInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		UStaticMesh* PlatformStaticMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		FTransform PlatformTransform;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		bool bIsFixedPlatform = false;

	UPROPERTY(VisibleInstanceOnly)
		EPlatformType PlatformType = EPlatformType::E_None;

	//UPROPERTY(BlueprintReadWrite)
		//	TArray<FObstacleInfo> ObstacleInfoList; 
};

//Edge Info
USTRUCT(BlueprintType)
struct FEdgeInfoStruct
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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		TArray<FPlatformInfoStruct> PlatformInfoList;

	//간선 정보 리스트 (잇는 스태틱 메시와 그것의 Transform)
	//반드시 정점 개수만큼 초기화해서 쓸 것.
	//EdgeInfoList[0].AdjacencyList  --> 0과 연결된 모든 정점 목록 {1, 2, 3}
	//EdgeInfoList[1].DistanceList[0];  --> 1과 연결된 첫번째 정점까지의 거리
	//양방향이기에 반대도 무조건 처리해야함.
	UPROPERTY(BlueprintReadWrite)
		TArray<FEdgeInfoStruct>	EdgeInfoList;

	//총 Fitness 값
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
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

USTRUCT(BlueprintType)
struct FPlatformPointStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//꼭지점
	UPROPERTY(BlueprintReadOnly)
		TArray<FVector> PointList;

	//상단면 점
	UPROPERTY(BlueprintReadOnly)
		TArray<FVector> AdditionalPointList;

	//Origin, Extent 등 다양한 프로퍼티 및 정보 포함
	UPROPERTY(BlueprintReadOnly)
		FBox BoxInfo;
};

USTRUCT(BlueprintType)
struct FPoint3D
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "3D Point")
		float x;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "3D Point")
		float y;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "3D Point")
		float z;
};

USTRUCT(BlueprintType)
struct FGAWeightInfo
{
public:
	GENERATED_USTRUCT_BODY()

	//플랫폼이 파라미터로 지정한 범위 외부에 있는지에 대한 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float outOfRangeWeight = 0.05f;
	
	//겹치는 플랫폼이 있는지에 대한 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float overlappedWeight = 0.05f; 
	
	//플레이어가 도달 불가능한 플랫폼이 있는지에 대한 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float unreachableWeight = 0.3f; 
	
	//난이도가 적합하지 않은것에 대한 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float difficultyWeight = 0.6f; 
};
