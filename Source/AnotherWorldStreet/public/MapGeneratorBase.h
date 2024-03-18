// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnotherWorldStreet.h"
#include "GameFramework/Actor.h"
#include "MapGeneratorBase.generated.h"


UCLASS()
class ANOTHERWORLDSTREET_API AMapGeneratorBase : public AActor
{
	GENERATED_BODY()

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

//======= 맵 속성 프로퍼티 ============
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

	//레벨 당 허용가능한 점프 최대 거리, 맵 속성 프로퍼티의 MapLevel 만큼의 멤버는 있어야함. DefaultValue : 250.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		TArray<float> LevelPerJumpDistThreshold;

//======= GA 프로퍼티 ================
public:
	//총 학습 세대 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 100000))
		int32 GenerationThresold = 100;

	//집단 구성하는 염색체 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 1000))
		int32 PopulationSize = 50;

	//Crossover Operation Rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MaxCrossOverRate = 0.5f;

	//Mutate Operation Rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MaxMutationRate = 0.1f;

	//적합도 계산 함수에 대한 가중치 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		TArray<float> FitnessWeightRate; 

protected:
	//현재 세대 번호
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		int32 CurrentGen;

	//현재 세대 정보 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		FPopulationStruct CurrentPopulationInfo;

//======= GA Function ================
public:
	UFUNCTION(BlueprintCallable)
		FVector RunGeneticAlgorithm();

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
	//초기 모집단에 사용할 맵을 생성한다.
	//Create initial map for initial population.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void CreateInitialMap();

	//맵을 시각화한다. Visualize map with map info struct
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Visualize(FMapInfoStruct MapInfo);

protected:
	UFUNCTION(BlueprintCallable)
		void SetInitialPopulation(FMapInfoStruct MapInfo);
};
