// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameMode.h"
#include "ABCharacter.h"
#include "ABPlayerController.h"
#include "ABPlayerState.h"

AABGameMode::AABGameMode()
{
    DefaultPawnClass = AABCharacter::StaticClass();
    PlayerControllerClass = AABPlayerController::StaticClass();
    PlayerStateClass = AABPlayerState::StaticClass();

}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    //ABLOG_S(Warning);
    auto ABPlayerState = Cast<AABPlayerState>(NewPlayer->PlayerState);
    ABCHECK(nullptr != ABPlayerState);
    ABPlayerState->InitPlayerData();
}
