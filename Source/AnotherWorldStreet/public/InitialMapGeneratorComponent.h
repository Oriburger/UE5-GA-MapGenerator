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
	//����
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Generate();

	//������ ���� ��� ��ȯ
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FMapInfoStruct GetLastResult() { return LastGenerateResult; }


//======== FPoint 3D ===============
public:
	//Bezier curve �������� FVector ����ü
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control points")
		TArray<FVector> ControlPoints;

		//Bezier Curve �� ��ǥ���� FVector ����ü
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control points")
		TArray<FVector> CurvePoints;

//============ �ʱ������ ���� ���� UFUNCTION ============
public:
	//control point ���� ���� �Լ�
	UFUNCTION()
		FVector GenControlPoint();

	UFUNCTION()
		void GenBezierCurve(FVector Start, FVector End, int32 GenPointNum, int32 ControlPointNum);


	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FMapInfoStruct> GetLastResultList() { return LastGenerateResultList; }

protected:	
	//������ ���� ���
	UPROPERTY(BlueprintReadWrite)
		FMapInfoStruct LastGenerateResult;

	UPROPERTY(BlueprintReadWrite)
		TArray<FMapInfoStruct> LastGenerateResultList;

	//Gen_Control_Point�Լ� �����
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
