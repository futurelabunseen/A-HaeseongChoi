// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SSCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/SSCharacterControlData.h"
#include "Physics/SSColision.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASSCharacterBase::ASSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Capsule
	GetCapsuleComponent()->SetCollisionProfileName(CPROFILE_SSCAPSULE);

	// Mesh
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	bDead = false;
}

void ASSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASSCharacterBase, bDead);
}

void ASSCharacterBase::AttackHitCheck()
{
}

float ASSCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	SetDead();

	return DamageAmount;
}

void ASSCharacterBase::ReleaseThrowable()
{
}

void ASSCharacterBase::SetDead()
{
	bDead = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetActorEnableCollision(false);
}

void ASSCharacterBase::Dissolve()
{
	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();

	if (SkeletalMeshComponent)
	{
		TArray<UMaterialInterface*> Materials = SkeletalMeshComponent->GetMaterials();

		uint32 MaterialIndex = 0;
		for (UMaterialInterface* const Material : Materials)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
			if (DynamicMaterial)
			{
				SkeletalMeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
				DynamicMaterialIndices.Add(MaterialIndex);
			}
			++MaterialIndex;
		}

		if (!DynamicMaterialIndices.IsEmpty())
		{
			const float TimerRate = DissolveDuration / 60.0f;		// DissolveDuration���� 60�� ������Ʈ
			DissolveStartTime = GetWorld()->GetTimeSeconds();
			GetWorld()->GetTimerManager().SetTimer(DissolveTimerHandle, this, &ASSCharacterBase::UpdateDissolveProgress, TimerRate, true);
		}
	}
}

void ASSCharacterBase::UpdateDissolveProgress()
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - DissolveStartTime;
	float Alpha = FMath::Clamp(ElapsedTime / DissolveDuration, 0.0f, 1.0f);

	for (const int32& DynamicMaterialIndex : DynamicMaterialIndices)
	{
		UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(GetMesh()->GetMaterial(DynamicMaterialIndex));
		DynamicMaterial->SetScalarParameterValue("DissolveParams", Alpha);
	}

	if (Alpha >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
		Destroy();
	}
}

void ASSCharacterBase::OnRep_ServerCharacterbDead()
{
	if (bDead)
	{
		SetDead();
	}
}


