// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SSCharacterPlayer.h"
#include "SS_RespawnTankPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SUPERSOLDIER_API ASS_RespawnTankPlayer : public ASSCharacterPlayer
{
	GENERATED_BODY()
public:
	ASS_RespawnTankPlayer(const FObjectInitializer& ObjectInitializer);

	virtual void Landed(const FHitResult& Hit) override;
protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_Controller() override;
protected:
	FTimerHandle RespawnTimerHandle;

	UPROPERTY()
	TObjectPtr<class ASSCharacterPlayer> MurdockCharacter;
public:
	virtual void Tick(float DeltaSeconds) override;

protected:
	float LerpCharacterAlpha;

	UFUNCTION()
	void RespawnMurdockCharacter();

	UFUNCTION()
	void SetRespawnMurdockLocation();

// Camera Shake Section
protected:
	UPROPERTY()
	TSubclassOf<UCameraShakeBase> LandingCameraLocationShakeClass;

// Camera Lerp Section
public:
	void SetRespawnStartCameraLocation(const FVector_NetQuantize& NewLocation);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(Replicated)
	FVector_NetQuantize RespawnStartCameraRelativeLocation;
	FVector RespawnEndCameraRelativeLocation;

// VFX Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UNiagaraComponent> TrailNiagara;

// SFX Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USoundBase> FallingSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USoundBase> LandingSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UAudioComponent> FallingSoundComponent;

protected:
	UFUNCTION(NetMulticast, UnReliable)
	void NetMulticastSettingEffectOnLand();
};
