 // Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AnotherWorldStreetGameModeBase.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateAfterInit, bool, Result, FVector, StartLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateGameFinish, float, TotalTime);

UCLASS()
class ANOTHERWORLDSTREET_API AAnotherWorldStreetGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

//====== Delegate ==================
public:
	virtual void BeginPlay() override;

//====== Delegate ==================
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable, Category = "Event")
		FDelegateAfterInit OnInitializeEndDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable, Category = "Event")
		FDelegateGameFinish OnGameFinishedDelegate;

	UFUNCTION(BlueprintNativeEvent)
		void OnInitializeEnded(bool Result, FVector StartLocation);

//====== Game Function ==================
protected:
	//Initialize map with data
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void InitializeGame();

	//Start game after initializing game
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void StartGame();

	//Finish game
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void FinishGame();
};