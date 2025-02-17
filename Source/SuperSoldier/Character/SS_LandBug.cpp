// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SS_LandBug.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/SSLandBugAIController.h"
#include "Physics/SSColision.h"
#include "Engine/DamageEvents.h"
#include "Character/SSCharacterPlayer.h"
#include "Character/CharacterStat/SSCharacterStatComponent.h"

ASS_LandBug::ASS_LandBug(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(
		TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Meshes/SK_LandBug.SK_LandBug"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(
		TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Animations/ABP_LandBug.ABP_LandBug_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackMontageRef(
		TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Animations/AM_LandBugAttack.AM_LandBugAttack"));
	if (AttackMontageRef.Object)
	{
		AttackMontage = AttackMontageRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitReactMontageRef(
		TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Animations/AM_LandBugHitReact.AM_LandBugHitReact"));
	if (HitReactMontageRef.Object)
	{
		HitReactMontage = HitReactMontageRef.Object;
	}

	bTurnInPlace = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(75.f, 75.0f);

	// Movement
	DefaultWalkSpeed = 750.0f * WALK_SPEED_CORRECTION;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;

	// Mesh & AnimInstance
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -75.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	// Stat
	Stat->SetMaxHP(120.0f);

	// AI
	AttackRange = 220.0f;
	AIControllerClass = ASSLandBugAIController::StaticClass();

	// SFX
	static ConstructorHelpers::FObjectFinder<USoundBase> AttackSoundRef(TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Sounds/SC_LandBugAttack.SC_LandBugAttack"));
	if (AttackSoundRef.Object)
	{
		AttackSound = AttackSoundRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> MoanSoundRef(TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Sounds/SC_LandBugHitMoan.SC_LandBugHitMoan"));
	if (MoanSoundRef.Object)
	{
		MoanSound = MoanSoundRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DeadSoundRef(TEXT("/Game/SuperSoldier/Characters/Monsters/LandBug/Sounds/SC_LandBugDead.SC_LandBugDead"));
	if (DeadSoundRef.Object)
	{
		DeadSound = DeadSoundRef.Object;
	}

	// Attack
	AttackMontageSectionNames.Add(TEXT("RoundKickAttack"));
}

void ASS_LandBug::AttackHitCheck(FName AttackId)
{
	Super::AttackHitCheck(AttackId);

	if (HasAuthority())
	{
		FVector CollisionCenter = GetActorLocation() + GetActorForwardVector() * 150.0f;
		float CollisionRadius = 45.0f;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		TArray<FOverlapResult> OverlapResults;
		FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(CollisionRadius);

		bool bHitDetected = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			CollisionCenter,
			FQuat::Identity,
			CCHANNEL_SSACTION,
			CollisionSphere,
			CollisionParams);


//#if ENABLE_DRAW_DEBUG
//		FColor DrawColor = bHitDetected ? FColor::Green : FColor::Red;
//		DrawDebugSphere(GetWorld(), CollisionCenter, CollisionRadius, 18, DrawColor, false, 2.0f);
//#endif
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* OverlapActor = OverlapResult.GetActor();
			ASSCharacterPlayer* CharacterPlayer = Cast<ASSCharacterPlayer>(OverlapActor);

			if (CharacterPlayer)
			{
				FDamageEvent DamageEvent;
				const float AttackDamage = 25.0f;
				CharacterPlayer->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
			}
		}
	}
}
