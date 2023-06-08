// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameplayWidget.h"
#include "Components/Button.h"
#include "ABPlayerController.h"

void UABGameplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ResumeButton = Cast<UButton>(GetWidgetFromName(TEXT("btnResume")));
    if (nullptr != ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UABGameplayWidget::OnResumeCliked);
    }

    ReturnToTitleButton = Cast<UButton>(GetWidgetFromName(TEXT("btnReturnToTitle")));
    if (nullptr != ReturnToTitleButton)
    {
        ReturnToTitleButton->OnClicked.AddDynamic(this, &UABGameplayWidget::OnReturnToTitleCliked);
    }

    RetryGameButton = Cast<UButton>(GetWidgetFromName(TEXT("btnRetryGame")));
    if (nullptr != RetryGameButton)
    {
        RetryGameButton->OnClicked.AddDynamic(this, &UABGameplayWidget::OnResumeCliked);
    }
}

void UABGameplayWidget::OnResumeCliked()
{
    auto ABPlayerController = Cast<AABPlayerController>(GetOwningPlayer());
    ABCHECK(nullptr != ABPlayerController);

    RemoveFromParent();
    ABPlayerController->ChangeInputMode(true);
    ABPlayerController->SetPause(false);
}

void UABGameplayWidget::OnReturnToTitleCliked()
{
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("Title"));
}

void UABGameplayWidget::OnRetryCliked()
{
    auto ABPlayerController = Cast<AABPlayerController>(GetOwningPlayer());
    ABCHECK(nullptr != ABPlayerController);
    ABPlayerController->RestartLevel();
}
