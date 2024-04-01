// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SSGameMode.h"
#include "Core/SSPlayerController.h"
#include "Core/SSGameState.h"
#include "GameFramework/HUD.h"
#include "SuperSoldier.h"


ASSGameMode::ASSGameMode()
{
	// Set Character
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(
		TEXT("/Script/SuperSoldier.SSMurdockPlayer"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}

	// Set PlayerController
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(
		TEXT("/Script/SuperSoldier.SSPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}

	// Set GameState
	GameStateClass = ASSGameState::StaticClass();
}

void ASSGameMode::StartPlay()
{
	// ������ ������ �����Ѵ�.
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::StartPlay();
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("End"));
}

void ASSGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// Ŭ���̾�Ʈ�� ���� ��û�� ó���Ѵ�.
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("====================================================================="));
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("End"));
}

APlayerController* ASSGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// ������ ����� Ŭ���̾�Ʈ�� �����ϴ� �÷��̾� ��Ʈ�ѷ��� �����.
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("Begin"));
	APlayerController* NewPlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("End"));

	return NewPlayerController;
}

void ASSGameMode::PostLogin(APlayerController* NewPlayer)
{
	// �÷��̾� ������ ���� �÷��̾ �ʿ��� �⺻ ������ ��� ������ �Ѵ�.
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::PostLogin(NewPlayer);
	SS_LOG(LogSSNetwork, Log, TEXT("%s"), TEXT("End"));
}
