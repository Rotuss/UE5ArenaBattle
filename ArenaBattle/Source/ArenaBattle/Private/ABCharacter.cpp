// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"
#include "ABCharacterStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"
#include "ABAIController.h"
#include "ABCharacterSetting.h"
#include "ABGameInstance.h"
#include "ABPlayerController.h"
#include "ABPlayerState.h"
#include "ABHUDWidget.h"
#include "ABGameMode.h"

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
    AttackRange = 80.0f;
    AttackRadius = 50.0f;

    HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("WidgetBlueprint'/Game/UI/UI_HPBar.UI_HPBar_C'"));
    if (UI_HUD.Succeeded())
    {
        HPBarWidget->SetWidgetClass(UI_HUD.Class);
        HPBarWidget->SetDrawSize(FVector2D(150.0f, 50.0f));
    }

    // ABCharacter마다 ABAIController 액터 생성
    // 플레이어 캐릭터를 제외한 모든 캐릭터는 ABAIController의 지배 받게 됨
    AIControllerClass = AABAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // ArenaBattleSetting 모듈 사용
    // 애셋 목록 로그 출력 확인용
    /*auto DefaultSetting = GetDefault<UABCharacterSetting>();
    if (0 < DefaultSetting->CharacterAssets.Num())
    {
        for (auto CharacterAsset : DefaultSetting->CharacterAssets)
        {
            ABLOG(Warning, TEXT("Character Asset : %s"), *CharacterAsset.ToString());
        }
    }*/

    // 플레이어 애셋 로딩 지정 인덱스
    AssetIndex = 4;

    SetActorHiddenInGame(true);
    HPBarWidget->SetHiddenInGame(true);
    SetCanBeDamaged(false);

    DeadTimer = 5.0f;

}

void AABCharacter::SetCharacterState(ECharacterState NewState)
{
    ABCHECK(CurrentState != NewState);
    CurrentState = NewState;

    switch (CurrentState)
    {
    case ECharacterState::LOADING:
    {
        if (true == bIsPlayer)
        {
            DisableInput(ABPlayerController);

            ABPlayerController->GetHUDWidget()->BindCharacterStat(CharacterStat);

            auto ABPlayerState = Cast<AABPlayerState>(GetPlayerState());
            ABCHECK(nullptr != ABPlayerState);
            CharacterStat->SetNewLevel(ABPlayerState->GetCharacterLevel());
        }
        else
        {
            auto ABGameMode = Cast<AABGameMode>(GetWorld()->GetAuthGameMode());
            ABCHECK(nullptr != ABGameMode);
            int32 TargetLevel = FMath::CeilToInt((float)ABGameMode->GetScore() * 0.8f);
            int32 FinalLevel = FMath::Clamp<int32>(TargetLevel, 1, 20);
            ABLOG(Warning, TEXT("New NPC Level : %d"), FinalLevel);
            CharacterStat->SetNewLevel(FinalLevel);
        }

        SetActorHiddenInGame(true);
        HPBarWidget->SetHiddenInGame(true);
        SetCanBeDamaged(false);
        break;
    }
    case ECharacterState::READY:
    {
        SetActorHiddenInGame(false);
        HPBarWidget->SetHiddenInGame(false);
        SetCanBeDamaged(true);
        CharacterStat->OnHPIsZero.AddLambda([this]() -> void {
            SetCharacterState(ECharacterState::DEAD);
            });

        auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
        ABCHECK(nullptr != CharacterWidget);
        CharacterWidget->BindCharacterStat(CharacterStat);

        if (true == bIsPlayer)
        {
            SetControlMode(EControlMode::DIABLO);
            GetCharacterMovement()->MaxWalkSpeed = 600.0f;
            EnableInput(ABPlayerController);
        }
        else
        {
            SetControlMode(EControlMode::NPC);
            GetCharacterMovement()->MaxWalkSpeed = 400.0f;
            ABAIController->RunAI();
        }

        break;
    }
    case ECharacterState::DEAD:
    {
        SetActorEnableCollision(false);
        GetMesh()->SetHiddenInGame(false);
        HPBarWidget->SetHiddenInGame(true);
        ABAnim->SetDeadAnim();
        SetCanBeDamaged(false);

        if (true == bIsPlayer)
        {
            DisableInput(ABPlayerController);
        }
        else
        {
            ABAIController->StopAI();
        }

        GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
            GetWorld()->GetTimerManager().ClearTimer(DeadTimerHandle);
            
            if (true == bIsPlayer)
            {
                ABPlayerController->ShowResultUI();
            }
            else
            {
                Destroy();
            }

            }), DeadTimer, false);

        break;
    }
    }
}

ECharacterState AABCharacter::GetCharacterState() const
{
    return ECharacterState();
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();

    //if (false == IsPlayerControlled())
    //{
    //    // ArenaBattleSetting 모듈
    //    // 애셋 랜덤 로드
    //    auto DefaultSetting = GetDefault<UABCharacterSetting>();
    //    int32 RandIndex = FMath::RandRange(0, DefaultSetting->CharacterAssets.Num() - 1);
    //    CharacterAssetToLoad = DefaultSetting->CharacterAssets[RandIndex];

    //    auto ABGameInstance = Cast<UABGameInstance>(GetGameInstance());
    //    if (nullptr != ABGameInstance)
    //    {
    //        AssetStreamingHandle = ABGameInstance->StreamableManager.RequestAsyncLoad(CharacterAssetToLoad, FStreamableDelegate::CreateUObject(this, &AABCharacter::OnAssetLoadCompleted));
    //    }
    //}

    bIsPlayer = IsPlayerControlled();
    if (true == bIsPlayer)
    {
        ABPlayerController = Cast<AABPlayerController>(GetController());
        ABCHECK(nullptr != ABPlayerController);
    }
    else
    {
        ABAIController = Cast<AABAIController>(GetController());
        ABCHECK(nullptr != ABAIController);
    }

    auto DefaultSetting = GetDefault<UABCharacterSetting>();

    if (true == bIsPlayer)
    {
        //AssetIndex = 4;
        auto ABPlayerState = Cast<AABPlayerState>(GetPlayerState());
        ABCHECK(nullptr != ABPlayerState);
        AssetIndex = ABPlayerState->GetCharacterIndex();
    }
    else
    {
        AssetIndex = FMath::RandRange(0, DefaultSetting->CharacterAssets.Num() - 1);
    }

    CharacterAssetToLoad = DefaultSetting->CharacterAssets[AssetIndex];
    auto ABGameInstance = Cast<UABGameInstance>(GetGameInstance());
    ABCHECK(nullptr != ABGameInstance);
    AssetStreamingHandle = ABGameInstance->StreamableManager.RequestAsyncLoad(CharacterAssetToLoad, FStreamableDelegate::CreateUObject(this, &AABCharacter::OnAssetLoadCompleted));
    SetCharacterState(ECharacterState::LOADING);

}

void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
    CurrentControlMode = NewControlMode;

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
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
        ArmLengthTo = 800.0f;
        ArmRotationTo = FRotator(-45.0f, 0.0f, 0.0f);
        SpringArm->bUsePawnControlRotation = false;
        SpringArm->bInheritPitch = false;
        SpringArm->bInheritRoll = false;
        SpringArm->bInheritYaw = false;
        SpringArm->bDoCollisionTest = false;
        // 45도씩 끊기는 회전을 부드럽게 만들어 줌
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    case EControlMode::NPC:
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
        break;
    default:
        break;
    }
}

void AABCharacter::SetControlMode(int32 ControlMode)
{
    if (0 == ControlMode)
    {
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

    ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    ABCHECK(nullptr != ABAnim);

    ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);
    ABAnim->OnNextAttackCheck.AddLambda([this]()->void {
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
    
    //ABLOG(Warning, TEXT("Actor : %s took damage : %f"), *GetName(), FinalDamage);

    CharacterStat->SetDamage(FinalDamage);

    if (ECharacterState::DEAD == CurrentState)
    {
        if (true == EventInstingator->IsPlayerController())
        {
            auto instigator = Cast<AABPlayerController>(EventInstingator);
            ABCHECK(nullptr != instigator, 0.0f);
            instigator->NPCKill(this);
        }
    }

    return FinalDamage;
}

//void AABCharacter::PossessedBy(AController* NewController)
//{
//    Super::PossessedBy(NewController);
//
//    if (true == IsPlayerControlled())
//    {
//        SetControlMode(EControlMode::DIABLO);
//        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
//    }
//    else
//    {
//        SetControlMode(EControlMode::NPC);
//        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
//    }
//}

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

int32 AABCharacter::GetEXP() const
{
    return CharacterStat->GetDropExp();
}

float AABCharacter::GetFinalAttackRange() const
{
    return (nullptr != CurrentWeapon) ? CurrentWeapon->GetAttackRange() : AttackRange;
}

float AABCharacter::GetFinalAttackDamage() const
{
    float AttackDamage = (nullptr != CurrentWeapon) ? (CharacterStat->GetAttack() + CurrentWeapon->GetAttackDamage()) : CharacterStat->GetAttack();
    float AttackModifier = (nullptr != CurrentWeapon) ? (CurrentWeapon->GetAttackModifier()) : (1.0f);

    return AttackDamage * AttackModifier;
}

bool AABCharacter::CanSetWeapon()
{
    return true;
}

void AABCharacter::SetWeapon(AABWeapon* NewWeapon)
{
    //ABCHECK(nullptr != NewWeapon && nullptr == CurrentWeapon);
    ABCHECK(nullptr != NewWeapon);

    if (nullptr != CurrentWeapon)
    {
        CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

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
    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddControllerYawInput(NewAxisValue);
        break;
    }
}

void AABCharacter::LookUp(float NewAxisValue)
{
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
    if (true == IsAttacking)
    {
        ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
        if (true == CanNextCombo)
        {
            IsComboInputOn = true;
        }
    }
    else
    {
        ABCHECK(0 == CurrentCombo);
        AttackStartComboState();
        ABAnim->PlayAttackMontage();
        ABAnim->JumpToAttackMontageSection(CurrentCombo);
        IsAttacking = true;
    }
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool binterrupted)
{
    ABCHECK(IsAttacking);
    ABCHECK(0 < CurrentCombo); 
    IsAttacking = false;
    AttackEndComboState();
    OnAttackEnd.Broadcast();
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
    float FinalAttackRange = GetFinalAttackRange();
    
    FHitResult HitResult;
    FCollisionQueryParams Params(NAME_None, false, this);
    bool bResult = GetWorld()->SweepSingleByChannel(
        HitResult,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * FinalAttackRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(AttackRadius),
        Params);

#if ENABLE_DRAW_DEBUG
    FVector TraceVec = GetActorForwardVector() * FinalAttackRange;
    FVector Center = GetActorLocation() + TraceVec * 0.5f;
    float HalfHeight = FinalAttackRange * 0.5f + AttackRadius;
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
            HitResult.GetActor()->TakeDamage(GetFinalAttackDamage(), DamageEvent, GetController(), this);
        }
    }
}

void AABCharacter::OnAssetLoadCompleted()
{
    AssetStreamingHandle->ReleaseHandle();
    TSoftObjectPtr<USkeletalMesh> LoadedAssetPath(CharacterAssetToLoad);
    ABCHECK(LoadedAssetPath.IsValid());

    /*if (true == LoadedAssetPath.IsValid())
    {
        GetMesh()->SetSkeletalMesh(LoadedAssetPath.Get());
    }*/
    GetMesh()->SetSkeletalMesh(LoadedAssetPath.Get());
    SetCharacterState(ECharacterState::READY);
}

