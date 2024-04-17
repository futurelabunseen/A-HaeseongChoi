// Fill out your copyright notice in the Description page of Project Settings.


#include "SSStratagemManager.h"
#include "Interface/SSStratagemInterface.h"
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

ISSStratagemInterface* USSStratagemManager::GetStratagem(const FName& StratagemName)
{
	// ��Ͽ� �ִ� Ư�� ��Ʈ��Ÿ���� ������ ����
	const TScriptInterface<ISSStratagemInterface>& RetStratagem = *StratagemMap.Find(StratagemName);
	ISSStratagemInterface* RetVal = RetStratagem.GetInterface();

	return RetVal;
}
