// Fill out your copyright notice in the Description page of Project Settings.


#include "ABSection.h"
#include "ABCharacter.h"
#include "ABItemBox.h"
#include "ABPlayerController.h"
#include "ABGameMode.h"

// Sets default values
AABSection::AABSection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MESH"));
    RootComponent = Mesh;

    // ¸Ê ¹è°æ ÁöÇü
    FString AssetPath = TEXT("StaticMesh'/Game/StaticMesh/SM_SQUARE.SM_SQUARE'");
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_SQUARE(*AssetPath);
    if (true == SM_SQUARE.Succeeded())
    {
        Mesh->SetStaticMesh(SM_SQUARE.Object);
    }
    else
    {
        ABLOG(Error, TEXT("Failed to load staticmesh asset. : %s"), *AssetPath);
    }

    // ¹Ú½º Æ®¸®°Å
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TRIGGER"));
    Trigger->SetBoxExtent(FVector(775.0f, 775.0f, 300.0f));
    Trigger->SetupAttachment(RootComponent);
    Trigger->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
    Trigger->SetCollisionProfileName(TEXT("ABTrigger"));
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AABSection::OnTriggerBeginOverlap);

    // ¸Ê Ã¶¹®
    FString GateAssetPath = TEXT("StaticMesh'/Game/StaticMesh/SM_GATE.SM_GATE'");
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_GATE(*GateAssetPath);
    if (false == SM_GATE.Succeeded())
    {
        ABLOG(Error, TEXT("Failed to load staticmesh asset : %s"), *GateAssetPath);
    }

    static FName GateSockets[] = { { TEXT("+XGate")}, { TEXT("-XGate")} , { TEXT("+YGate")} , { TEXT("-YGate")} };
    for (FName GateSocket : GateSockets)
    {
        ABCHECK(Mesh->DoesSocketExist(GateSocket));
        UStaticMeshComponent* NewGate = CreateDefaultSubobject<UStaticMeshComponent>(*GateSocket.ToString());
        NewGate->SetStaticMesh(SM_GATE.Object);
        NewGate->SetupAttachment(RootComponent, GateSocket);
        NewGate->SetRelativeLocation(FVector(0.0f, -80.5f, 0.0f));
        GateMeshes.Add(NewGate);

        UBoxComponent* NewGateTrigger = CreateDefaultSubobject<UBoxComponent>(*GateSocket.ToString().Append(TEXT("Trigger")));
        NewGateTrigger->SetBoxExtent(FVector(100.0f, 100.0f, 300.0f));
        NewGateTrigger->SetupAttachment(RootComponent, GateSocket);
        NewGateTrigger->SetRelativeLocation(FVector(70.0f, 0.0f, 250.0f));
        NewGateTrigger->SetCollisionProfileName(TEXT("ABTrigger"));
        GateTriggers.Add(NewGateTrigger);

        // ¸Ê È®Àå
        NewGateTrigger->OnComponentBeginOverlap.AddDynamic(this, &AABSection::OnGateTriggerBeginOverlap);
        NewGateTrigger->ComponentTags.Add(GateSocket);
    }

    bNoBattle = false;

    EnemySpawnTime = 2.0f;
    ItemBoxSpawnTime = 5.0f;

}

void AABSection::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    SetState(true == bNoBattle ? ESectionState::COMPLETE : ESectionState::READY);
}

// Called when the game starts or when spawned
void AABSection::BeginPlay()
{
	Super::BeginPlay();
	
    SetState(true == bNoBattle ? ESectionState::COMPLETE : ESectionState::READY);

}

// Called every frame
void AABSection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AABSection::SetState(ESectionState NewState)
{
    switch (NewState)
    {
    case ESectionState::READY:
    {
        Trigger->SetCollisionProfileName(TEXT("ABTrigger"));
        for (UBoxComponent* GateTrigger : GateTriggers)
        {
            GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
        }

        OperateGates(true);
        break;
    }
    case ESectionState::BATTLE:
    {
        Trigger->SetCollisionProfileName(TEXT("NoCollision"));
        for (UBoxComponent* GateTrigger : GateTriggers)
        {
            GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
        }

        OperateGates(false);

        // NPC »ý¼º
        GetWorld()->GetTimerManager().SetTimer(SpawnNPCTimerHandle, FTimerDelegate::CreateUObject(this, &AABSection::OnNPCSpawn), EnemySpawnTime, false);

        // Item Box ·£´ý »ý¼º
        GetWorld()->GetTimerManager().SetTimer(SpawnItemBoxTimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
            FVector2D RandXY = FMath::RandPointInCircle(600.0f);
            GetWorld()->SpawnActor<AABItemBox>(GetActorLocation() + FVector(RandXY, 30.0f), FRotator::ZeroRotator);
            }
        ), ItemBoxSpawnTime, false);

        break;
    }
    case ESectionState::COMPLETE:
    {
        Trigger->SetCollisionProfileName(TEXT("NoCollision"));
        for (UBoxComponent* GateTrigger : GateTriggers)
        {
            GateTrigger->SetCollisionProfileName(TEXT("ABTrigger"));
        }

        OperateGates(true);
        break;
    }
    default:
        break;
    }

    CurrentState = NewState;
}

void AABSection::OperateGates(bool bOpen)
{
    for (UStaticMeshComponent* Gate : GateMeshes)
    {
        Gate->SetRelativeRotation(true == bOpen ? FRotator(0.0f, -90.0f, 0.0f) : FRotator::ZeroRotator);
    }
}

void AABSection::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentState == ESectionState::READY)
    {
        SetState(ESectionState::BATTLE);
    }
}

void AABSection::OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABCHECK(1 == OverlappedComponent->ComponentTags.Num());

    FName ComponentTag = OverlappedComponent->ComponentTags[0];
    FName SocketName = FName(*ComponentTag.ToString().Left(2));
    if (false == Mesh->DoesSocketExist(SocketName))
    {
        return;
    }

    FVector NewLocation = Mesh->GetSocketLocation(SocketName);

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionQueryParam(NAME_None, false, this);
    FCollisionObjectQueryParams ObjectQueryParam(FCollisionObjectQueryParams::InitType::AllObjects);
    bool bResult = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        NewLocation,
        FQuat::Identity,
        ObjectQueryParam,
        FCollisionShape::MakeSphere(775.0f),
        CollisionQueryParam
    );

    if (false == bResult)
    {
        auto NewSection = GetWorld()->SpawnActor<AABSection>(NewLocation, FRotator::ZeroRotator);
    }
    else
    {
        ABLOG(Warning, TEXT("New section area is not empty."));
    }
}

void AABSection::OnNPCSpawn()
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnNPCTimerHandle);
    auto KeyNPC = GetWorld()->SpawnActor<AABCharacter>(GetActorLocation() + FVector::UpVector * 88.0f, FRotator::ZeroRotator);
    if (nullptr != KeyNPC)
    {
        KeyNPC->OnDestroyed.AddDynamic(this, &AABSection::OnKeyNPCDestoryed);
    }
}

void AABSection::OnKeyNPCDestoryed(AActor* DestroyedActor)
{
    auto ABCharacter = Cast<AABCharacter>(DestroyedActor);
    ABCHECK(nullptr != ABCharacter);

    auto ABPlayerController = Cast<AABPlayerController>(ABCharacter->LastHitBy);
    ABCHECK(nullptr != ABPlayerController);

    auto ABGameMode = Cast<AABGameMode>(GetWorld()->GetAuthGameMode());
    ABCHECK(nullptr != ABGameMode);
    ABGameMode->AddScore(ABPlayerController);

    SetState(ESectionState::COMPLETE);
}

