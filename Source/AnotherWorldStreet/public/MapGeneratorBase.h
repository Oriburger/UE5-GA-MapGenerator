// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnotherWorldStreet.h"
#include "GameFramework/Actor.h"
#include "MapGeneratorBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDelegateGAProgressUpdated, int32, CurrentGen, float, Progress, float, BestFitness);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateGAProcessEnd, FVector, StartLocation, float, TotalElapsedTime);

UCLASS()
class ANOTHERWORLDSTREET_API AMapGeneratorBase : public AActor
{
	GENERATED_BODY()
//======= Delegate ===============================
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGAProgressUpdated OnGAUpdatedProgress;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGAProcessEnd OnGAEndProcess;

//======= Basic Event & Component ================
public:	
	// Sets default values for this actor's properties
	AMapGeneratorBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY()
		USceneComponent* DefaultSceneRoot; 

//======= �� �Ӽ� ������Ƽ ============
public:
	//Level of map
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Setting")
		int32 MapLevel = 1;

	//Size/Boundary of Map
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Setting")
		FVector MapSize = { 1000.0f, 1000.0f, 1000.0f };
	
	//if not assigned, it has random value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Setting")
		FVector StartLocation;	

	//if not assigned, it has random value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Setting")
		FVector EndLocation;

	//���� �� ��밡���� ���� �ִ� �Ÿ�, �� �Ӽ� ������Ƽ�� MapLevel ��ŭ�� ����� �־����. DefaultValue : 250.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		TArray<float> LevelPerJumpDistThreshold;

//======= GA ������Ƽ ================
public:
	//�� �н� ���� 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 100000))
		int32 GenerationThresold = 1000;

	//���� �����ϴ� ����ü ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 1000))
		int32 PopulationSize = 50;

	//Crossover Operation Rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MaxCrossOverRate = 0.5f;

	//Mutate Operation Rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MaxMutationRate = 0.1f;

	//���յ� ��� �Լ��� ���� ����ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		TArray<float> FitnessWeightRate; 

protected:
	//���� ���� ��ȣ
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		int32 CurrentGen;

	//���� ���� ���� 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		FPopulationStruct CurrentPopulationInfo;

	//�� ��� �ð� (�н� ���� ������ ����)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		float TotalElapsedTime = 1e9;	

private:
	//1ƽ�� ��� ������ �н��� ������ �ʰ� �ϱ� ���� Ÿ�̸� �ڵ�
	//(����) 1ƽ���� ������ �� ���뾿 �н��� ����
	UPROPERTY()
		FTimerHandle GenerationDelayHandle;

	UPROPERTY()
		FTimerHandle GAElapsedTimeHandle;

//======= GA Function ================
public:
	UFUNCTION(BlueprintCallable)
		void RunGeneticAlgorithm();

private:
	UFUNCTION()
		float CalculateFitness();

	UFUNCTION()
		float SelectParents();

	UFUNCTION()
		FMapInfoStruct Crossover(const FMapInfoStruct& child1, const FMapInfoStruct& child2);

	UFUNCTION()
		bool Mutate(FMapInfoStruct& child);

protected:
	UFUNCTION()
		void Repair(FPopulationStruct& Result);

//======= Other Functions ================
public:
	//�ʱ� �����ܿ� ����� ���� �����Ѵ�.
	//Create initial map for initial population.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void CreateInitialMap();

	//���� �ð�ȭ�Ѵ�. Visualize map with map info struct
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Visualize(FMapInfoStruct MapInfo);

protected:
	UFUNCTION(BlueprintCallable)
		void SetInitialPopulation(FMapInfoStruct MapInfo);
};
