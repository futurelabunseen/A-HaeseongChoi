// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemManager.h"
#include "Interface/StratagemInterface.h"
#include "SSStratagem.h"

UStratagemManager::UStratagemManager()
{
	InitializeStratagem();
}

void UStratagemManager::InitializeStratagem()
{
	// ��Ʈ��Ÿ�� ��ü ��� �ʱ�ȭ
	USSStratagem* Stratagem = NewObject<USSStratagem>();
	if (Stratagem)
	{
		StratagemMap.Add(FName(TEXT("Stratagem")), Stratagem);
	}
}

IStratagemInterface* UStratagemManager::GetStratagem(const FName& StratagemName)
{
	// ��Ͽ� �ִ� Ư�� ��Ʈ��Ÿ���� ������ ����
	IStratagemInterface* RetVal = *StratagemMap.Find(StratagemName);

	return RetVal;
}
