// Fill out your copyright notice in the Description page of Project Settings.


#include "SSStratagemManager.h"
#include "Interface/SSStratagemInterface.h"
#include "SSStratagem.h"

USSStratagemManager::USSStratagemManager()
{
	InitializeStratagem();
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

ISSStratagemInterface* USSStratagemManager::GetStratagem(const FName& StratagemName)
{
	// ��Ͽ� �ִ� Ư�� ��Ʈ��Ÿ���� ������ ����
	ISSStratagemInterface* RetVal = *StratagemMap.Find(StratagemName);

	return RetVal;
}
