// Fill out your copyright notice in the Description page of Project Settings.


#include "Strata/SSStratPrecisionStrike.h"
USSStratPrecisionStrike::USSStratPrecisionStrike()
{
	DelayTime = 5.0f;
	CoolTime = 20.0f;

	CommandArray = TArray<EStrataCommand>{ EStrataCommand::RIGHT, EStrataCommand::RIGHT, EStrataCommand::UP };
}

void USSStratPrecisionStrike::ActivateStratagem(UWorld* const CurWorld, const FVector& TargetLocation)
{
	// �̻��� ����
	FString PrecisionStrikePath = TEXT("/Game/SuperSoldier/Props/BP_PrecisionStrike.BP_PrecisionStrike_C");
	UClass* PrecisionStrikeClass = StaticLoadClass(UObject::StaticClass(), nullptr, *PrecisionStrikePath);
	if (PrecisionStrikeClass)
	{
		AActor* PrecisionStrikeActor = CurWorld->SpawnActor<AActor>(PrecisionStrikeClass);
		
		FVector PrecisionStrikeLocation = TargetLocation;
		PrecisionStrikeLocation.Z += 4000.0f;
		PrecisionStrikeActor->SetActorLocation(PrecisionStrikeLocation);

		UFunction* StrikeFunction = PrecisionStrikeClass->FindFunctionByName(FName(TEXT("Strike")));
		if (StrikeFunction)
		{
			FVector StrikeLocation = TargetLocation;
			PrecisionStrikeActor->ProcessEvent(StrikeFunction, &StrikeLocation);
		}
	}
}
