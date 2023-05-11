// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "AIController.h"
#include "ABAIController.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABAIController : public AAIController
{
	GENERATED_BODY()
	
public:
    AABAIController();

//protected:
//    virtual void OnPossess(APawn* InPawn) override;
//    virtual void OnUnPossess() override;
//
//private:
//    void OnRepeatTimer();
//
//    FTimerHandle RepeatTimerHandle;
//    float RepeatInterval;

    virtual void OnPossess(APawn* InPawn) override;

    static const FName HomePosKey;
    static const FName PatrolPosKey;
    static const FName TargetKey;

private:
    UPROPERTY()
        class UBehaviorTree* BTAsset;

    UPROPERTY()
        class UBlackboardData* BBAsset;

};
