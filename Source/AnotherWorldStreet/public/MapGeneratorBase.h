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

//======= �� �Ӽ� ������Ƽ ============
public:
	//Level of map
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Setting")
		int32 MapLevel = 1;

	//Size/Boundary of Map
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Map Setting")
		FVector MapSize = { 1000.0f, 1000.0f, 1000.0f };

	//���� �� ��밡���� ���� �ִ� �Ÿ�, �� �Ӽ� ������Ƽ�� MapLevel ��ŭ�� ����� �־����. DefaultValue : 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Setting", meta = (UIMin = 0.0f, UIMax = 500.0f))
		TArray<float> LevelPerJumpDistThreshold;

	//������ �����ų �÷��� ����ƽ �޽� ����
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
	//�÷��� �޽��� �� ����Ʈ ����
	UPROPERTY(BlueprintReadOnly)
		TMap<class UStaticMesh*, FPlatformPointStruct> PlatformMeshPointInfoMap;

//======= GA ������Ƽ ================
public:
	//�� �н� ���� 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 1))
		int32 GenerationThresold = 1000;

	//���� �����ϴ� ����ü ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0, UIMax = 1000))
		int32 PopulationSize = 50;

	//�θ� ���� ���꿡�� ����Ƽ������ ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float ElitismRate = 0.7f;

	//Crossover Operation Rate
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
	//	float MaxCrossOverRate = 0.5f;

	//crossover�� �ڽĿ� ���� �������� ������ ���� Ȯ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MutatePossibility = 0.25f;

	//Mutate Operation Rate (�ִ� ����)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		float MaxMutationRate = 0.1f;

	//�������� ������ ���� (�޽� �� �� ������ �� %�� �ִ�� �̵� �����ϰ� �� ������, 1.0 == 100%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 10.0f))
		float MutationStrength = 3.0f;

	//���յ� ��� �Լ��� ���� ����ġ ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting", meta = (UIMin = 0.0f, UIMax = 1.0f))
		FGAWeightInfo GAWeightInfo;	

	//���� ������ ������ ������?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA Setting")
		bool bIsRepairActive = false;

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
	UPROPERTY()
		float BestFitnessValue = FLT_MAX; 

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
	//�ʱ� �����ܿ� ����� ���� �����Ѵ�.
	//Create initial map for initial population.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void CreateInitialMap();

	//���� �ð�ȭ�Ѵ�. Visualize map with map info struct
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

	//ĳ���� �����Ʈ�� MaxWalkSpeed�� JumpZVelocity�� ����
	UPROPERTY()
		FVector JumpVelocity;

	//ĳ���� �����Ʈ�� GravityScale�� ����
	UPROPERTY()
		float GravityMultipiler;
};