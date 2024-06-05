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

	//���� ���� ID
	UPROPERTY(BlueprintReadWrite)
		int32 StartNodeID; 

	//StartNodeID���� �����ϴ� ��� ���� ���
	UPROPERTY(BlueprintReadWrite)
		TArray<int32> AdjacencyList;

	//���� �Ÿ�
	UPROPERTY(BlueprintReadWrite)
		TArray<float> DistanceList;

	//������ �޽� 
	UPROPERTY(BlueprintReadWrite)
		TArray<UStaticMesh*> StaticMeshList;

	//������ �޽� Ʈ������(����)
	UPROPERTY(BlueprintReadWrite)
		TArray<FTransform> TransformList;
};

//Chromosome
USTRUCT(BlueprintType)
struct FMapInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//�÷��� ���� ����Ʈ (���� ����Ʈ)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		TArray<FPlatformInfoStruct> PlatformInfoList;

	//���� ���� ����Ʈ (�մ� ����ƽ �޽ÿ� �װ��� Transform)
	//�ݵ�� ���� ������ŭ �ʱ�ȭ�ؼ� �� ��.
	//EdgeInfoList[0].AdjacencyList  --> 0�� ����� ��� ���� ��� {1, 2, 3}
	//EdgeInfoList[1].DistanceList[0];  --> 1�� ����� ù��° ���������� �Ÿ�
	//������̱⿡ �ݴ뵵 ������ ó���ؾ���.
	UPROPERTY(BlueprintReadWrite)
		TArray<FEdgeInfoStruct>	EdgeInfoList;

	//�� Fitness ��
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		float TotalFitness = 0.0f;
};

USTRUCT(BlueprintType)
struct FPopulationStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//������ ���� 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		TArray<FMapInfoStruct> Population;

	//��������� �ְ� ��� ��
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		FMapInfoStruct BestResultMap;

	//�� ���� FitnessValue
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		float BestFitnessValue;

	//���յ� ���
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		float FitnessAverage;
};

USTRUCT(BlueprintType)
struct FPlatformPointStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//������
	UPROPERTY(BlueprintReadOnly)
		TArray<FVector> PointList;

	//��ܸ� ��
	UPROPERTY(BlueprintReadOnly)
		TArray<FVector> AdditionalPointList;

	//Origin, Extent �� �پ��� ������Ƽ �� ���� ����
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

	//�÷����� �Ķ���ͷ� ������ ���� �ܺο� �ִ����� ���� ����ġ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float outOfRangeWeight = 0.05f;
	
	//��ġ�� �÷����� �ִ����� ���� ����ġ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float overlappedWeight = 0.05f; 
	
	//�÷��̾ ���� �Ұ����� �÷����� �ִ����� ���� ����ġ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float unreachableWeight = 0.3f; 
	
	//���̵��� �������� �����Ϳ� ���� ����ġ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 1.0f))
		float difficultyWeight = 0.6f; 
};
