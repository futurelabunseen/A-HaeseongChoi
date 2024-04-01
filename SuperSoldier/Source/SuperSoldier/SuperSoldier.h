// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// NetMode�� �������� ��ũ��
#define LOG_NETMODINFO ((GetNetMode() == NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : (GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER"))

// � �Լ����� ������ �� �� �ִ� �����
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#define SS_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// ī�װ� ����
DECLARE_LOG_CATEGORY_EXTERN(LogSSNetwork, Log, All);