// Fill out your copyright notice in the Description page of Project Settings.


#include "SSStratagemManager.h"
#include "Strata/SSStratagem.h"
#include "SSStratagem.h"

USSStratagemManager::USSStratagemManager()
{
}

void USSStratagemManager::InitializeStratagem()
{
	// ��Ʈ��Ÿ�� ��ü ��� �ʱ�ȭ
	USSStratagem* Stratagem = NewObject<USSStratagem>();
	if (Stratagem)
	{
		StratagemMap.Add(FName(TEXT("Stratagem")), Stratagem);
	}
}

USSStratagem* USSStratagemManager::GetStratagem(const FName& StratagemName)
{
	// ��Ͽ� �ִ� Ư�� ��Ʈ��Ÿ���� ������ ����
	USSStratagem* RetVal = StratagemMap.Find(StratagemName)->Get();
	check(RetVal)
	return RetVal;
}
