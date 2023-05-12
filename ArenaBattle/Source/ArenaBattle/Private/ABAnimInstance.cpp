// Fill out your copyright notice in the Description page of Project Settings.


#include "ABAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"

UABAnimInstance::UABAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
    
    IsInAir = false;
    IsDead = false;

    static ConstructorHelpers::FObjectFinder<UAnimMontage> ATTACK_MONTAGE(TEXT("AnimMontage'/Game/Animations/SK_Mannequin_Skeleton_Montage.SK_Mannequin_Skeleton_Montage'"));
    if (ATTACK_MONTAGE.Succeeded())
    {
        AttackMontage = ATTACK_MONTAGE.Object;
    }
}

void UABAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    auto Pawn = TryGetPawnOwner();
    if (false == ::IsValid(Pawn))
    {
        return;
    }

    if (false == IsDead)
    {
        CurrentPawnSpeed = Pawn->GetVelocity().Size();
        auto Character = Cast<ACharacter>(Pawn);
        if (nullptr != Character)
        {
            IsInAir = Character->GetMovementComponent()->IsFalling();
        }
    }
}

void UABAnimInstance::PlayAttackMontage()
{
    ABCHECK(false == IsDead);
    // 멤버 변수 선언 이후 델리게이트에 의해 공격 시작 종료 감지 가능
    // 따라서 Montage_IsPlaying 함수 사용 필요 없어짐
    Montage_Play(AttackMontage, 1.0f);
}

void UABAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
    ABCHECK(false == IsDead);
    ABCHECK(Montage_IsPlaying(AttackMontage));
    Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

void UABAnimInstance::AnimNotify_AttackHitCheck()
{
    OnAttackHitCheck.Broadcast();
}

void UABAnimInstance::AnimNotify_NextAttackCheck()
{
    OnNextAttackCheck.Broadcast();
}

FName UABAnimInstance::GetAttackMontageSectionName(int32 Section)
{
    ABCHECK(FMath::IsWithinInclusive<int32>(Section, 1, 4), NAME_None);
    return FName(*FString::Printf(TEXT("Attack%d"), Section));
}
