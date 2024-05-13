// Fill out your copyright notice in the Description page of Project Settings.


#include "SSStratagemManager.h"
#include "Strata/SSStratagem.h"
#include "Strata/SSStratPrecisionStrike.h"

USSStratagemManager::USSStratagemManager()
{
}

void USSStratagemManager::InitializeStratagem()
{
	// ��Ʈ��Ÿ�� ��ü ��� �ʱ�ȭ
	USSStratPrecisionStrike* Stratagem = NewObject<USSStratPrecisionStrike>();
	if (Stratagem)
	{
		StratagemMap.Add(FName(TEXT("PrecisionStrike")), Stratagem);
	}
}

USSStratagem* USSStratagemManager::GetStratagem(const FName& StratagemName)
{
	// ��Ͽ� �ִ� Ư�� ��Ʈ��Ÿ���� ������ ����
	USSStratagem* RetStratagem = *StratagemMap.Find(StratagemName);
	return RetStratagem;
}
