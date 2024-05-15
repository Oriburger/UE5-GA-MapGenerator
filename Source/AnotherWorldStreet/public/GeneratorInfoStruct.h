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

	UPROPERTY(BlueprintReadWrite)
		bool bIsFixedPlatform = false;
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
	UPROPERTY(BlueprintReadWrite)
		TArray<FPlatformInfoStruct> PlatformInfoList;

	//���� ���� ����Ʈ (�մ� ����ƽ �޽ÿ� �װ��� Transform)
	//�ݵ�� ���� ������ŭ �ʱ�ȭ�ؼ� �� ��.
	//EdgeInfoList[0].AdjacencyList  --> 0�� ����� ��� ���� ��� {1, 2, 3}
	//EdgeInfoList[1].DistanceList[0];  --> 1�� ����� ù��° ���������� �Ÿ�
	//������̱⿡ �ݴ뵵 ������ ó���ؾ���.
	UPROPERTY(BlueprintReadWrite)
		TArray<FEdgeInfoStruct>	EdgeInfoList;

	//�� Fitness ��
	UPROPERTY(BlueprintReadWrite)
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

