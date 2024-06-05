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

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StartPoint;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* MidPoint;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* EndPoint;

//======= 맵 속성 프로퍼티 ============
public:
	//Level of map
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Setting")
		int32 MapLevel = 1;

	//Size/Boundary of Map
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Map Setting")
		FVector MapSize = { 1000.0f, 1000.0f, 1000.0f };

	//레벨 당 허용가능한 점프 최대 거리, 맵 속성 프로퍼티의 MapLevel 만큼의 멤버는 있어야함. DefaultValue : 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Setting", meta = (UIMin = 0.0f, UIMax = 500.0f))
		TArray<float> LevelPerJumpDistThreshold;

	//레벨에 등장시킬 플랫폼 스태틱 메시 종류
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Setting")
		TArray<class UStaticMesh*> PlatformMeshList;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Map Setting")
		FVector StartLocation;

	//if not assigned, it has random value
	UPROPERTY(BlueprintReadOnly, Category = "Map Setting")
		FVector MidLocation;

	//if not assigned, it has random value
	UPROPERTY(BlueprintReadOnly, Category = "Map Setting")
		FVector EndLocation;

protected:
	//플랫폼 메시의 각 포인트 정보
	UPROPERTY(BlueprintReadOnly)
		TMap<class UStaticMesh*, FPlatformPointStruct> PlatformMeshPointInfoMap;

//======= GA 프로퍼티 ================
public:
	//총 학습 세대 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 1))
		int32 GenerationThresold = 1000;

	//집단 구성하는 염색체 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 1000))
		int32 PopulationSize = 50;

	//부모 선택 연산에서 엘리티즘으로 선택할 비율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float ElitismRate = 0.7f;

	//Crossover Operation Rate
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
	//	float MaxCrossOverRate = 0.5f;

	//crossover한 자식에 대해 돌연변이 연산을 가할 확률
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MutatePossibility = 0.25f;

	//Mutate Operation Rate (최대 비율)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MaxMutationRate = 0.1f;

	//돌연변이 연산의 강도 (메시 한 변 길이의 몇 %를 최대로 이동 가능하게 할 것인지, 1.0 == 100%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 10.0f))
		float MutationStrength = 3.0f;

	//적합도 계산 함수에 대한 가중치 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		FGAWeightInfo GAWeightInfo;	

	//수선 연산을 진행할 것인지?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting")
		bool bIsRepairActive = false;

protected:
	//현재 세대 번호
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		int32 CurrentGen;

	//현재 세대 정보 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		FPopulationStruct CurrentPopulationInfo;

	//총 경과 시간 (학습 종료 시점에 갱신)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GA Debug")
		float TotalElapsedTime = 1e9;	

private:
	UPROPERTY()
		float BestFitnessValue = FLT_MAX; 

	//1틱에 모든 세대의 학습을 돌리지 않게 하기 위한 타이머 핸들
	//(거의) 1틱마다 나눠서 한 세대씩 학습을 돌림
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
		void SelectParents();

	UFUNCTION(	)
		FMapInfoStruct Crossover(const FMapInfoStruct& G1, const FMapInfoStruct& G2);

	UFUNCTION()
		bool Mutate(FMapInfoStruct& child);

protected:
	UFUNCTION()
		void Repair(FMapInfoStruct& Result, bool bForceApply = false);

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
	UFUNCTION()
		void InitPlatformMeshPointInfo();

	UFUNCTION(BlueprintCallable)
		void SetInitialPopulation(TArray<FMapInfoStruct> MapInfoList);

//======= Fitness Condition Functions ================
	UFUNCTION(BlueprintCallable)
		float GetNearestPoint(const FPlatformInfoStruct& P1, const FPlatformInfoStruct& P2, FVector& PosA, FVector& PosB);

	UFUNCTION(BlueprintCallable)
		bool GetCanReach(FVector JumpVelocity, FVector Start, FVector End, float& ErrorDist, float GravityScale = 3.0f);

	UFUNCTION(BlueprintCallable)
		bool IsPointInsideCuboid(const FVector& PointA, const FVector& PointB, const FVector& TestPoint);

	UFUNCTION(BlueprintCallable)
		bool GetIsOverlapped(const FPlatformInfoStruct& P1, const FPlatformInfoStruct& P2);

//======== Property ==================================
private:
	UPROPERTY()
		class ACharacter* PlayerRef;

	//캐릭터 무브먼트의 MaxWalkSpeed와 JumpZVelocity를 따름
	UPROPERTY()
		FVector JumpVelocity;

	//캐릭터 무브먼트의 GravityScale을 따름
	UPROPERTY()
		float GravityMultipiler;
};