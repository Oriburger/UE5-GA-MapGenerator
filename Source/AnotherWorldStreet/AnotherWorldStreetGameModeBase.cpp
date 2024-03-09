// Copyright Epic Games, Inc. All Rights Reserved.


#include "AnotherWorldStreetGameModeBase.h"

void AAnotherWorldStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	OnInitializeEndDelegate.AddDynamic(this, &AAnotherWorldStreetGameModeBase::OnInitializeEnded);
}

void AAnotherWorldStreetGameModeBase::OnInitializeEnded_Implementation(bool Result, FVector StartLocation)
{
}