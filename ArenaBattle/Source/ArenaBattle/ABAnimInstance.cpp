// Fill out your copyright notice in the Description page of Project Settings.


#include "ABAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"

UABAnimInstance::UABAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
    
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
    if (::IsValid(Pawn))
    {
        // ���ǵ� �� ����
        CurrentPawnSpeed = Pawn->GetVelocity().Size();

        // ���� ������ �ƴ��� �������� ����
        auto Character = Cast<ACharacter>(Pawn);
        if (Character)
        {
            IsInAir = Character->GetMovementComponent()->IsFalling();
        }
    }
}

void UABAnimInstance::PlayAttackMontage()
{
    // ���� ��Ÿ�ְ� ��� ���� �ƴϸ� ����ض�
    /*if (false == Montage_IsPlaying(AttackMontage))
    {
        Montage_Play(AttackMontage, 1.0f);
    }*/
    // ��� ���� ���� ���� ��������Ʈ�� ���� ���� ���� ���� ���� ����
    // ���� Montage_IsPlaying �Լ� ��� �ʿ� ������
    Montage_Play(AttackMontage, 1.0f);
}

void UABAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
    ABCHECK(Montage_IsPlaying(AttackMontage));
    Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

void UABAnimInstance::AnimNotify_AttackHitCheck()
{
    // ABLOG_S(Warning);
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
