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

protected:	
	//마지막 생성 결과
	UPROPERTY(BlueprintReadWrite)
		FMapInfoStruct LastGenerateResult;

	UPROPERTY(BlueprintReadWrite)
		TArray<FMapInfoStruct> LastGenerateResultList;
};
