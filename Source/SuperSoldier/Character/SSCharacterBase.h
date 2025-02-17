// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Interface/SSAnimationAttackInterface.h"
#include "Interface/SSAnimationPlaySoundInterface.h"
#include "SSCharacterBase.generated.h"

UENUM()
enum class ECharacterCollisionType : uint8
{
	Normal,
	NoCollision
};

UCLASS()
class SUPERSOLDIER_API ASSCharacterBase : public ACharacter, public ISSAnimationAttackInterface, public ISSAnimationPlaySoundInterface
{
	GENERATED_BODY()

public:
	ASSCharacterBase(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

// Stat Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USSCharacterStatComponent> Stat;

// Attack Hit Section
protected:
	virtual void AttackHitCheck(FName AttackId = TEXT("None")) override;
	virtual void PlayMoanSound() override;

	virtual void ShowAttackEffect();
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
protected:
	AController* LastDamageInstigator;

// Overwhelming Section
protected:
	bool bIsOverwhelming;
	float OverwhelmingTime;

// Throw Section
public:
	virtual void ReleaseThrowable();

// Dead Section
protected:
	virtual void SetDead();
	void Dissolve();

	UFUNCTION()
	void UpdateDissolveProgress(const float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USoundBase> DeadSound;

	virtual void PlayDeadSound();

protected:
	const float DissolveDelayTime = 5.0f;
	TArray<uint32> DynamicMaterialIndices;

	UPROPERTY(EditAnywhere) // Timeline 생성
	FTimeline DissolveTimeline;

	UPROPERTY(EditAnywhere) // Timeline 커브
	TObjectPtr<UCurveFloat> DissolveCurveFloat;

public:
	UPROPERTY(ReplicatedUsing = OnRep_ServerCharacterbDead)
	uint8 bDead : 1;

protected:
	UFUNCTION()
	virtual void OnRep_ServerCharacterbDead();

public:
	void SetCharacterCollisionType(ECharacterCollisionType NewCharacterCollisionType);

	UFUNCTION()
	virtual void Respawn(const FVector& TargetLocation);
protected:
	UPROPERTY(ReplicatedUsing = OnRep_ServerCharacterCollisionType)
	ECharacterCollisionType CharacterCollisionType;

	UFUNCTION()
	void OnRep_ServerCharacterCollisionType();

// HitReact Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> HitReactMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USoundBase> MoanSound;

	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticastRpcShowAnimationMontage(UAnimMontage* MontageToPlay, const float AnimationSpeedRate);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastRpcShowAnimationMontageWithSection(UAnimMontage* MontageToPlay, FName SectionName, const float AnimationSpeedRate);
};


