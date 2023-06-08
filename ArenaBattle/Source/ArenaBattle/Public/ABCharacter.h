// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/Character.h"
#include "ABCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);

UCLASS()
class ARENABATTLE_API AABCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AABCharacter();

	void SetCharacterState(ECharacterState NewState);
	ECharacterState GetCharacterState() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	enum class EControlMode
	{
		GTA,
		DIABLO,
		NPC
	};
	
	void SetControlMode(EControlMode NewControlMode);

	EControlMode CurrentControlMode = EControlMode::GTA;
	FVector DirectionToMove = FVector::ZeroVector;

	float ArmLengthTo = 0.0f;
	FRotator ArmRotationTo = FRotator::ZeroRotator;
	float ArmLengthSpeed = 0.0f;
	float ArmRotationSpeed = 0.0f;

	void SetControlMode(int32 ControlMode);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstingator, AActor* DamageCauser) override;

	//virtual void PossessedBy(AController* NewController) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ����ġ
	int32 GetEXP() const;
	// ���� ���ο� ���� ���ݹ��� ����
	float GetFinalAttackRange() const;
	// ���⿡ ���� ���ݷ� ���
	float GetFinalAttackDamage() const;

	// ���� ���� Ȯ�� �Լ�
	bool CanSetWeapon();
	// ���� ���� �Լ�
	void SetWeapon(class AABWeapon* NewWeapon);

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	class AABWeapon* CurrentWeapon;
	
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	USkeletalMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, Category = Stat)
	class UABCharacterStatComponent* CharacterStat;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = UI)
	class UWidgetComponent* HPBarWidget;

	// ���� ���콺 ��ư => ����
	void Attack();
	FOnAttackEndDelegate OnAttackEnd;

private:
	// ��
	// W, S
	void UpDown(float NewAxisValue);
	// A, D
	void LeftRight(float NewAxisValue);
	// ���콺 �¿�(X)
	void Turn(float NewAxisValue);
	// ���콺 ����(Y)
	void LookUp(float NewAxisValue);

	// �׼�
	// Shift + V => GTA, ��ƺ�� ��� ��ȯ
	void ViewChange();

	// ====== ��Ÿ�� =====
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool binterrupted);

	void AttackStartComboState();
	void AttackEndComboState();

	// �浹 Ȯ��
	void AttackCheck();

	// �ּ� �ε� �޽� ����
	void OnAssetLoadCompleted();

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsAttacking;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool CanNextCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsComboInputOn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 MaxCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	float AttackRange;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	float AttackRadius;

	UPROPERTY()
	class UABAnimInstance* ABAnim;

	int32 AssetIndex = 0;

	FSoftObjectPath CharacterAssetToLoad = FSoftObjectPath(nullptr);
	TSharedPtr<struct FStreamableHandle> AssetStreamingHandle;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = State, Meta = (AllowPrivateAccess = true))
	ECharacterState CurrentState;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = State, Meta = (AllowPrivateAccess = true))
	bool bIsPlayer;

	UPROPERTY()
	class AABAIController* ABAIController;

	UPROPERTY()
	class AABPlayerController* ABPlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = State, Meta = (AllowPrivateAccess = true))
	float DeadTimer;

	FTimerHandle DeadTimerHandle = {};

};
