// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MapVisualizerComponent.h"

// Sets default values for this component's properties
UMapVisualizerComponent::UMapVisualizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UMapVisualizerComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UMapVisualizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMapVisualizerComponent::Visualize_Implementation(FMapInfoStruct MapInfo)
{
	//BP Native Event
	//필요시에만 네이티브 코드를 추가하세요
}