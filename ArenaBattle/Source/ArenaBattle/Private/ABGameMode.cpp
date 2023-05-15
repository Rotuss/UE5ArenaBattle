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
}
