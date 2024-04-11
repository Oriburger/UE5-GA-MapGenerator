// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnotherWorldStreet.h"
#include "Components/ActorComponent.h"
#include "InitialMapGeneratorComponent.generated.h"


UCLASS(Blueprintable)
class ANOTHERWORLDSTREET_API UInitialMapGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

//======= Basic Event ================
public:	
	// Sets default values for this component's properties
	UInitialMapGeneratorComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= Key Event ================
public:
	//생성
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Generate();

	//마지막 생성 결과 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FMapInfoStruct GetLastResult() { return LastGenerateResult; }


//======== FPoint 3D ===============
public:
	//Bezier curve 조절점의 FVector 구조체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control points")
		TArray<FVector> ControlPoints;

		//Bezier Curve 점 좌표들의 FVector 구조체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control points")
		TArray<FVector> CurvePoints;

//============ 초기모집단 생성 관련 UFUNCTION ============
public:
	//control point 랜덤 생성 함수
	UFUNCTION()
		FVector GenControlPoint();

	UFUNCTION()
		void GenBezierCurve(FVector Start, FVector End, int32 GenPointNum, int32 ControlPointNum);


	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FMapInfoStruct> GetLastResultList() { return LastGenerateResultList; }

protected:	
	//마지막 생성 결과
	UPROPERTY(BlueprintReadWrite)
		FMapInfoStruct LastGenerateResult;

	UPROPERTY(BlueprintReadWrite)
		TArray<FMapInfoStruct> LastGenerateResultList;

	//Gen_Control_Point함수 결과값
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gen Control point")
	//	FVector ControlPoint;

	//Fvector (Start, End) Location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location Setting")
		FVector StartLoc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location Setting")
		FVector EndLoc;


	//UPROPERTY(BlueprintReadOnly)
	//	UStaticMesh StaticMeshInfo;

	UPROPERTY(BlueprintReadWrite)
		FPlatformInfoStruct PlatformInfoResult;

private:
	UPROPERTY()
		class AMapGeneratorBase* MapGeneratorRef;
};
