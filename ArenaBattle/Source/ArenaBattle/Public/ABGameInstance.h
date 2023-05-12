// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "ABGameInstance.generated.h"

// CSV 파일을 불러들이기 위한 테이블 데이터 구조체
USTRUCT(BlueprintType)
struct FABCharacterData : public FTableRowBase
{
    GENERATED_BODY()

public:
    FABCharacterData()
        : Level(1)
        , MaxHP(100.0f)
        , Attack(10.0f)
        , DropExp(10)
        , NextExp(30)
    {

    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        int32 MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        int32 Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        int32 DropExp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        int32 NextExp;

};

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UABGameInstance();

	virtual void Init() override;
    
    FABCharacterData* GetABCharacterData(int32 Level);

    FStreamableManager StreamableManager;

private:
    UPROPERTY()
    class UDataTable* ABCharacterTable;

};
