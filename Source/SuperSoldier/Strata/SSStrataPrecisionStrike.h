// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Strata/SSStratagem.h"
#include "SSStrataPrecisionStrike.generated.h"

/**
 * 
 */
UCLASS()
class SUPERSOLDIER_API USSStrataPrecisionStrike : public USSStratagem
{
	GENERATED_BODY()
public:
	USSStrataPrecisionStrike();
	virtual void ActivateStratagem(UWorld* const CurWorld, AController* const StrataCauser, const FVector TargetLocation, const FVector ThrowedDirection) override;
};
