// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/InitialMapGeneratorComponent.h"

// Sets default values for this component's properties
UInitialMapGeneratorComponent::UInitialMapGeneratorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UInitialMapGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UInitialMapGeneratorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInitialMapGeneratorComponent::Generate_Implementation()
{
	//BP Native Event
	//필요시에만 네이티브 코드를 추가하세요	
}