// Fill out your copyright notice in the Description page of Project Settings.


#include "ABAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName AABAIController::HomePosKey(TEXT("HomePos"));
const FName AABAIController::PatrolPosKey(TEXT("PatrolPos"));
const FName AABAIController::TargetKey(TEXT("Target"));

AABAIController::AABAIController()
{
    static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("BlackboardData'/Game/AI/BB_ABCharacter.BB_ABCharacter'"));
    if (BBObject.Succeeded())
    {
        BBAsset = BBObject.Object;
    }

    static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("BehaviorTree'/Game/AI/BT_ABCharacter.BT_ABCharacter'"));
    if (BTObject.Succeeded())
    {
        BTAsset = BTObject.Object;
    }
}

void AABAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

    /*UBlackboardComponent* ABBlackboard = Cast<UBlackboardComponent>(Blackboard);
    if (UseBlackboard(BBAsset, ABBlackboard))
    {
        Blackboard->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());
        if (false == RunBehaviorTree(BTAsset))
        {
            ABLOG(Error, TEXT("AIController couldn't run behavior tree!"));
        }
    }*/
}

void AABAIController::RunAI()
{
    UBlackboardComponent* ABBlackboard = Cast<UBlackboardComponent>(Blackboard);
    if (true == UseBlackboard(BBAsset, ABBlackboard))
    {
        Blackboard->SetValueAsVector(HomePosKey, GetPawn()->GetActorLocation());
        if (false == RunBehaviorTree(BTAsset))
        {
            ABLOG(Error, TEXT("AIController couldn't run behavior tree!"));
        }
    }
}

void AABAIController::StopAI()
{
    auto BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
    if (nullptr != BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
    }
}
