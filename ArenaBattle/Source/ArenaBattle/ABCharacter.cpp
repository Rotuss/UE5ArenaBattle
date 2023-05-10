// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"
#include "ABCharacterStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"

// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
    CharacterStat = CreateDefaultSubobject<UABCharacterStatComponent>(TEXT("CHARACTERSTAT"));
    HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBARWIDGET"));

    SpringArm->SetupAttachment(GetCapsuleComponent());
    Camera->SetupAttachment(SpringArm);
    HPBarWidget->SetupAttachment(GetMesh());

    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard'"));
    if (SK_CARDBOARD.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(SK_CARDBOARD.Object);
    }

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

    static ConstructorHelpers::FClassFinder<UAnimInstance> BP_WARRIORS_ANIM(TEXT("AnimBlueprint'/Game/Animations/WarriorAnimBluprint.WarriorAnimBluprint_C'"));
    if (BP_WARRIORS_ANIM.Succeeded())
    {
        GetMesh()->SetAnimInstanceClass(BP_WARRIORS_ANIM.Class);
    }

    // 해당 소켓에 무기 부착
    /*FName WeaponSocket(TEXT("hand_rSocket"));
    if (GetMesh()->DoesSocketExist(WeaponSocket))
    {
        Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WEAPON"));
        static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_WEAPON(TEXT("SkeletalMesh'/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight'"));
        if (SK_WEAPON.Succeeded())
        {
            Weapon->SetSkeletalMesh(SK_WEAPON.Object);
        }

        Weapon->SetupAttachment(GetMesh(), WeaponSocket);
    }*/

    SetControlMode(0);
    SetControlMode(EControlMode::DIABLO);

    ArmLengthSpeed = 3.0f;
    ArmRotationSpeed = 10.0f;

    // 캐릭터 점프 높이 수치 설정
    GetCharacterMovement()->JumpZVelocity = 800.0f;

    IsAttacking = false;
    MaxCombo = 4;
    AttackEndComboState();

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));
    AttackRange = 200.0f;
    AttackRadius = 50.0f;

    HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("WidgetBlueprint'/Game/UI/UI_HPBar.UI_HPBar_C'"));
    if (UI_HUD.Succeeded())
    {
        HPBarWidget->SetWidgetClass(UI_HUD.Class);
        HPBarWidget->SetDrawSize(FVector2D(150.0f, 50.0f));
    }
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    // 소켓에 Weapon 액터 부착
    /*FName WeaponSocket(TEXT("hand_rSocket"));
    auto CurWeapon = GetWorld()->SpawnActor<AABWeapon>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (nullptr != CurWeapon)
    {
        CurWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
    }*/

}

void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
    CurrentControlMode = NewControlMode;

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        /*SpringArm->TargetArmLength = 450.0f;
        SpringArm->SetRelativeRotation(FRotator::ZeroRotator);*/
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bInheritPitch = true;
        SpringArm->bInheritRoll = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bDoCollisionTest = true;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    case EControlMode::DIABLO:
        /*SpringArm->TargetArmLength = 800.0f;
        SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));*/
        ArmLengthTo = 800.0f;
        ArmRotationTo = FRotator(-45.0f, 0.0f, 0.0f);
        SpringArm->bUsePawnControlRotation = false;
        SpringArm->bInheritPitch = false;
        SpringArm->bInheritRoll = false;
        SpringArm->bInheritYaw = false;
        SpringArm->bDoCollisionTest = false;
        // 45도씩 끊기는 회전을 부드럽게 만듦어 줌
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    default:
        break;
    }
}

void AABCharacter::SetControlMode(int32 ControlMode)
{
    if (0 == ControlMode)
    {
        /*SpringArm->TargetArmLength = 450.0f;
        SpringArm->SetRelativeRotation(FRotator::ZeroRotator);*/
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bInheritPitch = true;
        SpringArm->bInheritRoll = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bDoCollisionTest = true;
        // bUseControllerRotationYaw = false로 해야 캐릭터는 가만히 있고 카메라만 회전
        bUseControllerRotationYaw = false;
        // 카메라 화면 방향으로 캐릭터 회전 이동
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
    }
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLengthTo, DeltaTime, ArmLengthSpeed);

    switch (CurrentControlMode)
    {
    case EControlMode::DIABLO:
        SpringArm->SetRelativeRotation(FMath::RInterpTo(SpringArm->GetRelativeRotation(), ArmRotationTo, DeltaTime, ArmRotationSpeed)); 
        if (0.0f < DirectionToMove.SizeSquared())
        {
            GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionToMove).Rotator());
            AddMovementInput(DirectionToMove);
        }
        break;
    }

}

void AABCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    /*auto AnimInstance = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    ABCHECK(nullptr != AnimInstance);

    AnimInstance->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);*/
    ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    ABCHECK(nullptr != ABAnim);

    ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);
    ABAnim->OnNextAttackCheck.AddLambda([this]()->void {
        //ABLOG(Warning, TEXT("OnNextAttackCheck"));
        CanNextCombo = false;

        if (true == IsComboInputOn)
        {
            AttackStartComboState();
            ABAnim->JumpToAttackMontageSection(CurrentCombo);
        }
        });
    ABAnim->OnAttackHitCheck.AddUObject(this, &AABCharacter::AttackCheck);

    CharacterStat->OnHPIsZero.AddLambda([this]() -> void {
        ABLOG(Warning, TEXT("OnHPIsZero"));
        ABAnim->SetDeadAnim();
        SetActorEnableCollision(false);
        });

    HPBarWidget->InitWidget();
    auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
    if (nullptr != CharacterWidget)
    {
        CharacterWidget->BindCharacterStat(CharacterStat);
    }
}

float AABCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstingator, AActor* DamageCauser)
{
    float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstingator, DamageCauser);
    
    ABLOG(Warning, TEXT("Actor : %s took damage : %f"), *GetName(), FinalDamage);

    /*if (0.0f < FinalDamage)
    {
        ABAnim->SetDeadAnim();
        SetActorEnableCollision(false);
    }*/

    CharacterStat->SetDamage(FinalDamage);

    return FinalDamage;
}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 축
    PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AABCharacter::UpDown);
    PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AABCharacter::LeftRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);

    // 액션
    PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
    PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AABCharacter::Attack);
}

bool AABCharacter::CanSetWeapon()
{
    return (nullptr == CurrentWeapon);
}

void AABCharacter::SetWeapon(AABWeapon* NewWeapon)
{
    ABCHECK(nullptr != NewWeapon && nullptr == CurrentWeapon);
    
    // 소켓에 웨폰 부착
    FName WeaponSocket(TEXT("hand_rSocket"));
    if (nullptr != NewWeapon)
    {
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
        NewWeapon->SetOwner(this);
        CurrentWeapon = NewWeapon;
    }
}

void AABCharacter::UpDown(float NewAxisValue)
{
    // 컨트롤러 즉 캐릭터의 시선 방향으로 이동
    //AddMovementInput(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X), NewAxisValue);

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddMovementInput(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X), NewAxisValue);
        break;
    case EControlMode::DIABLO:
        DirectionToMove.X = NewAxisValue;
        break;
    }
}

void AABCharacter::LeftRight(float NewAxisValue)
{
    // 컨트롤러 즉 캐릭터의 시선 방향으로 이동
    //AddMovementInput(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y), NewAxisValue);

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddMovementInput(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y), NewAxisValue);
        break;
    case EControlMode::DIABLO:
        DirectionToMove.Y = NewAxisValue;
        break;
    }
}

void AABCharacter::Turn(float NewAxisValue)
{
    //AddControllerYawInput(NewAxisValue);

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddControllerYawInput(NewAxisValue);
        break;
    }
}

void AABCharacter::LookUp(float NewAxisValue)
{
    //AddControllerPitchInput(NewAxisValue);

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddControllerPitchInput(NewAxisValue);
        break;
    }
}

void AABCharacter::ViewChange()
{
    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        GetController()->SetControlRotation(GetActorRotation());
        SetControlMode(EControlMode::DIABLO);
        break;
    case EControlMode::DIABLO:
        GetController()->SetControlRotation(SpringArm->GetRelativeRotation());
        SetControlMode(EControlMode::GTA);
        break;
    }
}

void AABCharacter::Attack()
{
    // ABLOG_S(Warning);

    if (true == IsAttacking)
    {
        ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
        if (true == CanNextCombo)
        {
            IsComboInputOn = true;
        }
        //return;
    }
    else
    {
        ABCHECK(0 == CurrentCombo);
        AttackStartComboState();
        ABAnim->PlayAttackMontage();
        ABAnim->JumpToAttackMontageSection(CurrentCombo);
        IsAttacking = true;
    }
    /*auto AnimInstance = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    if (nullptr == AnimInstance)
    {
        return;
    }*/
    /*if (nullptr == ABAnim)
    {
        return;
    }*/

    //AnimInstance->PlayAttackMontage();
    //ABAnim->PlayAttackMontage();
    //IsAttacking = true;
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool binterrupted)
{
    ABCHECK(IsAttacking);
    ABCHECK(0 < CurrentCombo); 
    IsAttacking = false;
    AttackEndComboState();
}

void AABCharacter::AttackStartComboState()
{
    CanNextCombo = true;
    IsComboInputOn = false;
    ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 0, MaxCombo - 1));
    CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void AABCharacter::AttackEndComboState()
{
    IsComboInputOn = false;
    CanNextCombo = false;
    CurrentCombo = 0;
}

void AABCharacter::AttackCheck()
{
    /*FHitResult HitResult;
    FCollisionQueryParams Params(NAME_None, false, this);
    bool bResult = GetWorld()->SweepSingleByChannel(
        HitResult,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * 200.0f,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(50.0f),
        Params);

    if (true == bResult)
    {
        if (nullptr != HitResult.GetActor())
        {
            ABLOG(Warning, TEXT("Hit Actor Name : %s"), *HitResult.GetActor()->GetName());
        }
    }*/

    FHitResult HitResult;
    FCollisionQueryParams Params(NAME_None, false, this);
    bool bResult = GetWorld()->SweepSingleByChannel(
        HitResult,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * AttackRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(AttackRadius),
        Params);

#if ENABLE_DRAW_DEBUG
    FVector TraceVec = GetActorForwardVector() * AttackRange;
    FVector Center = GetActorLocation() + TraceVec * 0.5f;
    float HalfHeight = AttackRange * 0.5f + AttackRadius;
    FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
    FColor DrawColor = true == bResult ? FColor::Green : FColor::Red;
    float DebugLifeTime = 5.0f;

    DrawDebugCapsule(
        GetWorld(),
        Center,
        HalfHeight,
        AttackRadius,
        CapsuleRot,
        DrawColor,
        false,
        DebugLifeTime);
#endif

    if (true == bResult)
    {
        if (nullptr != HitResult.GetActor())
        {
            ABLOG(Warning, TEXT("Hit Actor Name : %s"), *HitResult.GetActor()->GetName());

            FDamageEvent DamageEvent;
            HitResult.GetActor()->TakeDamage(/*50.0f*/CharacterStat->GetAttack(), DamageEvent, GetController(), this);
        }
    }
}

