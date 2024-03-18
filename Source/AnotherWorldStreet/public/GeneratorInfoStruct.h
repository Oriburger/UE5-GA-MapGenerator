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

	//�÷��� ���� ����Ʈ (���� ����Ʈ)
	UPROPERTY(BlueprintReadWrite)
		TArray<FPlatformInfoStruct> PlatformInfoList;

	//���� ���� ����Ʈ (�մ� ����ƽ �޽ÿ� �װ��� Transform)
	UPROPERTY(BlueprintReadWrite)
		TArray<FPlatformInfoStruct>	EdgeInfoList;

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