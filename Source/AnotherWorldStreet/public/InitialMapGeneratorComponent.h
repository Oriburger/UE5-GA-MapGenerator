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

protected:	
	//������ ���� ���
	UPROPERTY(BlueprintReadWrite)
		FMapInfoStruct LastGenerateResult;
};