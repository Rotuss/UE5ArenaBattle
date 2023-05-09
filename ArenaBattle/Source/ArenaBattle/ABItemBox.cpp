// Fill out your copyright notice in the Description page of Project Settings.


#include "ABItemBox.h"
#include "ABWeapon.h"
#include "ABCharacter.h"

// Sets default values
AABItemBox::AABItemBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TRIGGER"));
    Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BOX"));
    Effect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EFFECT"));

    RootComponent = Trigger;
    Box->SetupAttachment(RootComponent);
    Effect->SetupAttachment(RootComponent);

    Trigger->SetBoxExtent(FVector(40.0f, 42.0f, 30.0f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_BOX(TEXT("StaticMesh'/Game/InfinityBladeGrassLands/Environments/Breakables/StaticMesh/Box/SM_Env_Breakables_Box1.SM_Env_Breakables_Box1'"));
    if (SM_BOX.Succeeded())
    {
        Box->SetStaticMesh(SM_BOX.Object);
    }

    static ConstructorHelpers::FObjectFinder<UParticleSystem> P_CHESTOPEN(TEXT("ParticleSystem'/Game/InfinityBladeGrassLands/Effects/FX_Treasure/Chest/P_TreasureChest_Open_Mesh.P_TreasureChest_Open_Mesh'"));
    if (P_CHESTOPEN.Succeeded())
    {
        Effect->SetTemplate(P_CHESTOPEN.Object);
        Effect->bAutoActivate = false;
    }

    Box->SetRelativeLocation(FVector(0.0f, -3.5f, -30.0f));

    Trigger->SetCollisionProfileName(TEXT("ItemBox"));
    Box->SetCollisionProfileName(TEXT("NoCollision"));

    WeaponItemClass = AABWeapon::StaticClass();

}

// Called when the game starts or when spawned
void AABItemBox::BeginPlay()
{
	Super::BeginPlay();
	
}

void AABItemBox::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // 플레이어 캐릭터와 오버랩되면 반응
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AABItemBox::OnCharacterOverlap);
}

// Called every frame
void AABItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AABItemBox::OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABLOG_S(Warning);

    auto ABCharacter = Cast<AABCharacter>(OtherActor);
    ABCHECK(nullptr != ABCharacter);

    if (nullptr != ABCharacter && nullptr != WeaponItemClass)
    {
        // 현재 캐릭터에 무기가 없을 때
        if (ABCharacter->CanSetWeapon())
        {
            auto NewWeapon = GetWorld()->SpawnActor<AABWeapon>(WeaponItemClass, FVector::ZeroVector, FRotator::ZeroRotator);
            ABCharacter->SetWeapon(NewWeapon);
            Effect->Activate(true);
            Box->SetHiddenInGame(true, true);
            SetActorEnableCollision(false);
            Effect->OnSystemFinished.AddDynamic(this, &AABItemBox::OnEffectFinished);
        }
        // 현재 캐릭터에 무기가 있을 때
        else
        {
            ABLOG(Warning, TEXT("%s can't equip weapon currently."), *ABCharacter->GetName());
        }
    }
}

void AABItemBox::OnEffectFinished(UParticleSystemComponent* PSystem)
{
    Destroy();
}

