// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnotherWorldStreet.h"
#include "Components/ActorComponent.h"
#include "MapVisualizerComponent.generated.h"


UCLASS(Blueprintable)
class ANOTHERWORLDSTREET_API UMapVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()

//======= Basic Event ================
public:	
	// Sets default values for this component's properties
	UMapVisualizerComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= Key Event ================
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Visualize(FMapInfoStruct MapInfo);
};
