// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameMode.h"
#include "ABCharacter.h"
#include "ABPlayerController.h"
#include "ABPlayerState.h"
#include "ABGameState.h"

AABGameMode::AABGameMode()
{
    DefaultPawnClass = AABCharacter::StaticClass();
    PlayerControllerClass = AABPlayerController::StaticClass();
    PlayerStateClass = AABPlayerState::StaticClass();
    GameStateClass = AABGameState::StaticClass();

    ScoreToClear = 2;

}

void AABGameMode::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    ABGameState = Cast<AABGameState>(GameState);
}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    //ABLOG_S(Warning);
    auto ABPlayerState = Cast<AABPlayerState>(NewPlayer->PlayerState);
    ABCHECK(nullptr != ABPlayerState);
    ABPlayerState->InitPlayerData();
}

void AABGameMode::AddScore(AABPlayerController* ScoredPlayer)
{
    for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
    {
        const auto ABPlayerController = Cast<AABPlayerController>(iter->Get());
        if ((nullptr != ABPlayerController) && (ScoredPlayer == ABPlayerController))
        {
            ABPlayerController->AddGameScore();
            break;
        }
    }

    ABGameState->AddGameScore();

    if (ScoreToClear <= GetScore())
    {
        ABGameState->SetGameCleared();

        for (FConstPawnIterator Iter = GetWorld()->GetPawnIterator(); Iter; ++Iter)
        {
            (*Iter)->TurnOff();
        }
        for (FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
        {
            const auto ABPlayerController = Cast<AABPlayerController>(Iter->Get());
            if (nullptr != ABPlayerController)
            {
                ABPlayerController->ShowResultUI();
            }
        }
    }

}

int32 AABGameMode::GetScore() const
{
    return ABGameState->GetTotalGameScore();
}
