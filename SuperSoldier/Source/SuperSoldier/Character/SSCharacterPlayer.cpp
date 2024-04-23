// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SSCharacterPlayer.h"
#include "SuperSoldier.h"
#include "EngineUtils.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/SSCharacterControlData.h"
#include "SSCharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "Core/SSGameInstance.h"
#include "Engine/DamageEvents.h"
#include "Physics/SSColision.h"
#include "Strata/SSStratagemManager.h"
#include "Strata/SSStrataIndicator.h"


ASSCharacterPlayer::ASSCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 350.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 0.0f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bAiming = false;
	bSprintKeyPressing = false;
	bReadyForThrowingStrata = false;
	bChangeMontageForThrowingStrata = false;

	// Input Action & Input Mapping Context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(
		TEXT("/Game/SuperSoldier/Input/IMC_Normal.IMC_Normal"));
	if (InputMappingContextRef.Object)
	{
		NormalInputMappingContext = InputMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionMoveRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Move.IA_Move"));
	if (InputActionMoveRef.Object)
	{
		MoveAction = InputActionMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionLookRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Look.IA_Look"));
	if (InputActionLookRef.Object)
	{
		LookAction = InputActionLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionSprintRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Sprint.IA_Sprint"));
	if (InputActionSprintRef.Object)
	{
		SprintAction = InputActionSprintRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAimRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Aim.IA_Aim"));
	if (InputActionAimRef.Object)
	{
		AimAction = InputActionAimRef.Object;
	}

	// Fire Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> FireActionRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Fire.IA_Fire"));
	if (FireActionRef.Object)
	{
		FireAction = FireActionRef.Object;
	}

	// Throw Throw Action
	static ConstructorHelpers::FObjectFinder<UInputAction> ThrowActionRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Grenade.IA_Grenade"));
	if (ThrowActionRef.Object)
	{
		ThrowAction = ThrowActionRef.Object;
	}

	// Call Throw Action
	static ConstructorHelpers::FObjectFinder<UInputAction> CallActionRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Call.IA_Call"));
	if (CallActionRef.Object)
	{
		CallAction = CallActionRef.Object;
	}

	// Strata Command Action
	static ConstructorHelpers::FObjectFinder<UInputAction> CommandActionRef(
		TEXT("/Game/SuperSoldier/Input/Actions/IA_Command.IA_Command"));
	if (CommandActionRef.Object)
	{
		CommandAction = CommandActionRef.Object;
	}

	// Widget (maybe Transfer to somewhere)
	static ConstructorHelpers::FClassFinder<UUserWidget> CrosshairWidgetRef(
		TEXT("/Game/SuperSoldier/UI/HUD.HUD_C"));

	if (CrosshairWidgetRef.Class)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetRef.Class);
	}

	// Character Control Data
	static ConstructorHelpers::FObjectFinder<USSCharacterControlData> NormalModeRef(
		TEXT("/Game/SuperSoldier/Characters/CharacterControl/DA_NormalMode.DA_NormalMode"));
	if (NormalModeRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Normal, NormalModeRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<USSCharacterControlData> AimModeRef(
		TEXT("/Game/SuperSoldier/Characters/CharacterControl/DA_AimMode.DA_AimMode"));
	if (AimModeRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Aiming, AimModeRef.Object);
	}
}

void ASSCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Look);
	EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Sprint);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Aim);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Fire);
	EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Throw);
	EnhancedInputComponent->BindAction(CallAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::Call);
	EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Triggered, this, &ASSCharacterPlayer::ProcessCommandInput);
}

void ASSCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Register Stratagem
	USSGameInstance* SSGameInstance = Cast<USSGameInstance>(GetGameInstance());
	USSStratagemManager* StratagemManager = SSGameInstance->GetStratagemManager();
	USSStratagem* DefaultStratagem = StratagemManager->GetStratagem(FName(TEXT("PrecisionStrike")));
	if (DefaultStratagem)
	{
		AvailableStratagems.Add(std::make_pair(FName(TEXT("PrecisionStrike")), DefaultStratagem));
	}

	// If Locally Controlled
	if (IsLocallyControlled())
	{
		APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
		if (PlayerController)
		{
			EnableInput(PlayerController);

			if (CrosshairWidget)
			{
				CrosshairWidget->AddToViewport();
				CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(NormalInputMappingContext, 0);
		}
	}
}

bool ASSCharacterPlayer::GetAnyMontagePlaying(UAnimMontage* FilterMontage)
{
	bool bRet = false;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (FilterMontage == NULL)
	{
		bRet = AnimInstance->IsAnyMontagePlaying();
	}
	else
	{
		UAnimMontage* CurrentActivateMontage = AnimInstance->GetCurrentActiveMontage();
		UClass* CurMontageClass = CurrentActivateMontage->StaticClass();
		UClass* FilterMontageClass = FilterMontage->StaticClass();

		bRet = AnimInstance->IsAnyMontagePlaying() &&
			CurMontageClass != FilterMontageClass;
	}
	return bRet;
}

void ASSCharacterPlayer::AttemptSprintEndDelegate(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	if (!bAiming && bSprintKeyPressing && GetAnyMontagePlaying(TargetMontage) == false)
	{
		SetSprintToMovementComponent(true);
	}
}

void ASSCharacterPlayer::SetCharacterControlData(const USSCharacterControlData* CharacterControlData)
{
	CharacterControlData->bCrosshairVisibility ? 
		CrosshairWidget->SetVisibility(ESlateVisibility::Visible) : 
		CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
	
	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SocketOffset = CharacterControlData->RelativeLocation;
}

void ASSCharacterPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASSCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ASSCharacterPlayer::SetSprintToMovementComponent(bool bNewSprint)
{
	USSCharacterMovementComponent* SSCharacterMovement = Cast<USSCharacterMovementComponent>(GetCharacterMovement());
	SSCharacterMovement->SetSprint(bNewSprint);
}

void ASSCharacterPlayer::AttemptSprint()
{
	if(!bAiming && bSprintKeyPressing && !GetAnyMontagePlaying())
	{
		SetSprintToMovementComponent(true);
	}
}

void ASSCharacterPlayer::Sprint(const FInputActionValue& Value)
{
	bSprintKeyPressing = Value.Get<bool>();
	if (bSprintKeyPressing)
	{
		AttemptSprint();
	}
	else
	{
		SetSprintToMovementComponent(false);
	}
}

void ASSCharacterPlayer::Aim(const FInputActionValue& Value)
{
	bAiming = Value.Get<bool>();
	SetAimingToMovementComponent(bAiming);

	if (bAiming)
	{
		SetCharacterControlData(*CharacterControlManager.Find(ECharacterControlType::Aiming));
		SetSprintToMovementComponent(false);

	}
	else
	{
		SetCharacterControlData(*CharacterControlManager.Find(ECharacterControlType::Normal));
		AttemptSprint();
	}
}

void ASSCharacterPlayer::SetAimingToMovementComponent(bool bNewAiming)
{
	USSCharacterMovementComponent* SSCharacterMovement = Cast<USSCharacterMovementComponent>(GetCharacterMovement());
	SSCharacterMovement->SetAiming(bNewAiming);
}

void ASSCharacterPlayer::Fire(const FInputActionValue& Value)
{
	if (bReadyForThrowingStrata)
	{
		const float AnimationSpeedRate = 1.0f;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(StrataThrowMontage, AnimationSpeedRate);

		FRotator ControlRotation = GetControlRotation();
		FRotator CurRotation = GetActorRotation();
		SetActorRotation(FRotator(CurRotation.Pitch, ControlRotation.Yaw, CurRotation.Roll));

		ServerRpcStrataThrow();

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASSCharacterPlayer::AttemptSprintEndDelegate);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, StrataThrowMontage);
		return;
	}

	// �ִϸ��̼� ��Ÿ�ְ� ��� ���� �ƴϰ�, ���� ���̶�� �ݹ�
	if (!GetAnyMontagePlaying() && bAiming)
	{
		const float AnimationSpeedRate = 1.0f;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(FireMontage, AnimationSpeedRate);

		ServerRpcFire();
	}
}

void ASSCharacterPlayer::Throw(const FInputActionValue& Value)
{
	if (!GetAnyMontagePlaying())
	{
		SetSprintToMovementComponent(false);

		const float AnimationSpeedRate = 1.0f;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(ThrowMontage, AnimationSpeedRate);

		ServerRpcThrow();

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASSCharacterPlayer::AttemptSprintEndDelegate);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrowMontage);
	}
}

void ASSCharacterPlayer::Call(const FInputActionValue& Value)
{
	if (!GetAnyMontagePlaying(CallMontage))
	{
		bCalling = Value.Get<bool>();

		if (bCalling)
		{
			SetSprintToMovementComponent(false);

			const float AnimationSpeedRate = 1.0f;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			AnimInstance->Montage_Play(CallMontage, AnimationSpeedRate);

			ServerRpcCalling();

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ASSCharacterPlayer::EndCalling);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, CallMontage);
		}

		else
		{
			// ���� ������� ������ End�� �ƴϸ� End ������ ���
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			FName CurSection = AnimInstance->Montage_GetCurrentSection(CallMontage);
			bool bNotAlreadyPlaying = CurSection.Compare(FName(TEXT("End"))) != 0;

			if (bNotAlreadyPlaying)
			{
				AnimInstance->Montage_JumpToSection(TEXT("End"), CallMontage);
				ServerRpcEndCalling();
			}

			InputSequence.Reset();
		}
	}
}

void ASSCharacterPlayer::EndCalling(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	if (bChangeMontageForThrowingStrata)
	{
		bReadyForThrowingStrata = true;
		bChangeMontageForThrowingStrata = false;

		const float AnimationSpeedRate = 1.0f;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(StrataReadyMontage, AnimationSpeedRate);

		FName StrataNameToActivate = AvailableStratagems[SelectedStrataIndex].first;
		ServerRpcStrataReady(StrataNameToActivate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASSCharacterPlayer::EndStrata);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, StrataReadyMontage);
	}
	else
	{
		if (!bAiming && bSprintKeyPressing && GetAnyMontagePlaying(TargetMontage) == false)
		{
			SetSprintToMovementComponent(true);
		}
	}
}

void ASSCharacterPlayer::EndStrata(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	bReadyForThrowingStrata = false;

	// �ٸ� ��� �ʿ�
	// GetMesh()->UnHideBoneByName(TEXT("bot_hand"));
}

void ASSCharacterPlayer::TranslateInput(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();
	EStrataCommand StrataCommand = EStrataCommand::NONE;

	if (InputValue.X)
	{
		if (InputValue.X > 0)
		{
			StrataCommand = EStrataCommand::RIGHT;
		}
		else
		{
			StrataCommand = EStrataCommand::LEFT;
		}
	}

	else
	{
		if (InputValue.Y > 0)
		{
			StrataCommand = EStrataCommand::UP;
		}
		else
		{
			StrataCommand = EStrataCommand::DOWN;
		}
	}

	InputSequence.Add(StrataCommand);

	UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EStrataCommand"), true);
	check(EnumPtr);

	UE_LOG(LogTemp, Log, TEXT("============CurrentInputSequence============="))
	for (const EStrataCommand& Cmd : InputSequence)
	{
		FString CommandString = EnumPtr->GetDisplayNameTextByValue(static_cast<uint32>(Cmd)).ToString();

		UE_LOG(LogTemp, Log, TEXT("%s"), *CommandString);
	}
	UE_LOG(LogTemp, Log, TEXT("============================================="))

#ifdef DEBUG_STRATAINPUT
	UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EStrataCommand"), true);
	check(EnumPtr);

	FString CommandString = EnumPtr->GetDisplayNameTextByValue(static_cast<uint32>(StrataCommand)).ToString();
	UE_LOG(LogTemp, Log, TEXT("%s"), *CommandString);
#endif // DEBUG_STRATAINPUT
}

bool ASSCharacterPlayer::MatchingInput()
{
	bool bSuccessMatching = false;
	if (!AvailableStratagems.IsEmpty())
	{
		// ��� ������ ��Ʈ��Ÿ���� ��ȸ, ��Ī
		for (int i = 0; i < AvailableStratagems.Num(); ++i)
		{
			if (AvailableStratagems.IsValidIndex(i))
			{
				USSStratagem* Stratagem = AvailableStratagems[i].second;

				const TArray<EStrataCommand> StrataCommandArr = Stratagem->GetCommandSequence();

				// ��Ʈ��Ÿ�� Ŀ�ǵ� ������, ���� �Է� Ŀ�ǵ� ���� ������ �˻��� �ʿ䰡 ����
				if (InputSequence.Num() > StrataCommandArr.Num())
				{
					continue;
				}

				// �Է� Ŀ�ǵ� ����ŭ ��ȸ �ϸ鼭, ��Ʈ��Ÿ�� Ŀ�ǵ�� ��
				bool bAllInputCommandMatching = true;
				for (int j = 0; j < InputSequence.Num(); ++j)
				{
					if (InputSequence[j] != StrataCommandArr[j])
					{
						bAllInputCommandMatching = false;
						break;
					}
				}

				// ��� Ŀ�ǵ尡 ��Ī�� �������� ���
				if (bAllInputCommandMatching)
				{
					bSuccessMatching = true;

					// ��� Ŀ�ǵ尡 ��Ī ���� ������, �Է� ���� ��Ʈ��Ÿ�� Ŀ�ǵ� ���� ������
					// ��Ʈ��Ÿ���� �ߵ�, �Է� Ŀ�ǵ� �迭�� ����.
					int32 InputSequenceNum = InputSequence.Num();
					int32 StrataCommandArrNum = StrataCommandArr.Num();

					if (InputSequenceNum == StrataCommandArrNum)
					{
						UE_LOG(LogTemp, Log, TEXT("Matching Success!!!!"))
						SelectedStrataIndex = i;

						UE_LOG(LogTemp, Log, TEXT("Reset InputSequence"))
						InputSequence.Reset();

						UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
						if (AnimInstance)
						{
							AnimInstance->Montage_JumpToSection(TEXT("End"), CallMontage);
							ServerRpcEndCalling();
						}

						bChangeMontageForThrowingStrata = true;
					}
				}
			}
		}
	}
	
	return bSuccessMatching;
}

void ASSCharacterPlayer::ProcessCommandInput(const FInputActionValue& Value)
{
	if (bCalling)
	{
		TranslateInput(Value);
		if (!MatchingInput())
		{
			UE_LOG(LogTemp, Log, TEXT("Matching Fail Reset InputSequence"))
			InputSequence.Reset();
		}
	}
}

void ASSCharacterPlayer::AttackHitCheck()
{
	if (IsLocallyControlled())
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector TraceStart = CameraLocation;
		FVector TraceEnd = TraceStart + CameraRotation.Vector() * 5000.0f;

		FHitResult HitResult;
		FCollisionQueryParams TraceParams(FName(TEXT("Attack")), false, this);

		bool HitDetected = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CCHANNEL_SSACTION, TraceParams);

		if (HasAuthority())
		{
			if (HitDetected)
			{
				FDamageEvent DamageEvent;
				const float AttackDamage = 30.0f;
				HitResult.GetActor()->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
			}
		}
		else
		{
			if (HitDetected)
			{
				ServerRpcNotifyFireHit(HitResult);
			}
			else
			{
				ServerRpcNotifyMiss(TraceStart, TraceEnd);
			}
		}
	
#if ENABLE_DRAW_DEBUG
		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DrawColor, false, 5.0f);
#endif
	}
}

void ASSCharacterPlayer::ReleaseThrowable()
{
	// Throw Actor
	if (CurStrataIndicator)
	{
		CurStrataIndicator->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurStrataIndicator->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f).Quaternion());

		UClass* StrataIndicatorClass = CurStrataIndicator.GetClass();
		UFunction* ThrowFunction = CurStrataIndicator->FindFunction(FName(TEXT("Throw")));

		if (ThrowFunction)
		{
			// Throw Camera Direction Vector
			FVector CameraLocation;
			FRotator CameraRotation;
			GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
			
			FVector ThrowPos = CameraLocation + CameraRotation.Vector() * 2000.0f;
			FVector ThrowDirection = ThrowPos - CurStrataIndicator->GetActorLocation();
			ThrowDirection.Normalize();

			CurStrataIndicator->ProcessEvent(ThrowFunction, &ThrowDirection);
		}
	}
}

void ASSCharacterPlayer::RpcPlayAnimation(UAnimMontage* MontageToPlay)
{
	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (PlayerController && GetController() != PlayerController)
		{
			if (!PlayerController->IsLocalController())
			{
				ASSCharacterPlayer* OtherPlayer = Cast<ASSCharacterPlayer>(PlayerController->GetPawn());

				if (OtherPlayer)
				{
					OtherPlayer->ClientRpcPlayAnimation(this, FireMontage);
				}
			}
		}
	}
}

void ASSCharacterPlayer::RpcJumpToSection(UAnimMontage* MontageToPlay, FName SectionName)
{
	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (PlayerController && GetController() != PlayerController)
		{
			if (!PlayerController->IsLocalController())
			{
				ASSCharacterPlayer* OtherPlayer = Cast<ASSCharacterPlayer>(PlayerController->GetPawn());

				if (OtherPlayer)
				{
					OtherPlayer->ClientRpcJumpToSection(this, FireMontage, SectionName);
				}
			}
		}
	}
}

void ASSCharacterPlayer::ClientRpcJumpToSection_Implementation(ASSCharacterPlayer* CharacterToPlay, UAnimMontage* MontageToPlay, FName SectionName)
{
	if (CharacterToPlay)
	{
		const float AnimationSpeedRate = 1.0f;
		UAnimInstance* AnimInstance = CharacterToPlay->GetMesh()->GetAnimInstance();
		FName CurSection = AnimInstance->Montage_GetCurrentSection(MontageToPlay);

		if (CurSection != NAME_None)
		{
			bool bNotAlreadyPlaying = CurSection.Compare(SectionName) != 0;

			if (bNotAlreadyPlaying)
			{
				AnimInstance->Montage_JumpToSection(SectionName, CallMontage);
			}
		}
	}
}

void ASSCharacterPlayer::ClientRpcPlayAnimation_Implementation(ASSCharacterPlayer* CharacterToPlay, UAnimMontage* MontageToPlay)
{
	if (CharacterToPlay)
	{
		const float AnimationSpeedRate = 1.0f;
		UAnimInstance* AnimInstance = CharacterToPlay->GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MontageToPlay, AnimationSpeedRate);
	}
}

bool ASSCharacterPlayer::ServerRpcFire_Validate()
{
	return true;
}

void ASSCharacterPlayer::ServerRpcFire_Implementation()
{
	const float AnimationSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(FireMontage, AnimationSpeedRate);

	RpcPlayAnimation(FireMontage);
}

bool ASSCharacterPlayer::ServerRpcNotifyFireHit_Validate(const FHitResult& HitResult)
{
	return true;
}

void ASSCharacterPlayer::ServerRpcNotifyFireHit_Implementation(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (IsValid(HitActor))
	{
		const float AcceptCheckDistance = 300.0f;

		const FVector HitLocation = HitResult.Location;
		const FBox HitBox = HitActor->GetComponentsBoundingBox();
		const FVector ActorBoxCenter = (HitBox.Min + HitBox.Max) * 0.5f;

		if (FVector::DistSquared(HitLocation, ActorBoxCenter) <= AcceptCheckDistance * AcceptCheckDistance)
		{
			FDamageEvent DamageEvent;
			const float AttackDamage = 30.0f;
			HitResult.GetActor()->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
		}
	}
}

bool ASSCharacterPlayer::ServerRpcNotifyMiss_Validate(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd)
{
#if ENABLE_DRAW_DEBUG
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Cyan, false, 5.0f);
#endif
	return true;
}

void ASSCharacterPlayer::ServerRpcNotifyMiss_Implementation(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd)
{

}

bool ASSCharacterPlayer::ServerRpcThrow_Validate()
{
	return true;
}

void ASSCharacterPlayer::ServerRpcThrow_Implementation()
{
	const float AnimationSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ThrowMontage, AnimationSpeedRate);

	RpcPlayAnimation(ThrowMontage);
}

bool ASSCharacterPlayer::ServerRpcCalling_Validate()
{
	return true;
}

void ASSCharacterPlayer::ServerRpcCalling_Implementation()
{
	const float AnimationSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(CallMontage, AnimationSpeedRate);

	RpcPlayAnimation(CallMontage);
}

bool ASSCharacterPlayer::ServerRpcEndCalling_Validate()
{
	return true;
}

void ASSCharacterPlayer::ServerRpcEndCalling_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	FName CurSection = AnimInstance->Montage_GetCurrentSection(CallMontage);

	if (CurSection != NAME_None)
	{
		bool bNotAlreadyPlaying = CurSection.Compare(FName(TEXT("End"))) != 0;

		if (bNotAlreadyPlaying)
		{
			AnimInstance->Montage_JumpToSection(TEXT("End"), CallMontage);
			RpcJumpToSection(CallMontage, TEXT("End"));
		}
	}
}

bool ASSCharacterPlayer::ServerRpcStrataReady_Validate(const FName& StratagemName)
{
	return true;
}

void ASSCharacterPlayer::ServerRpcStrataReady_Implementation(const FName& StratagemName)
{
	const float AnimationSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(StrataReadyMontage, AnimationSpeedRate);

	RpcPlayAnimation(StrataReadyMontage);

	CurStrataIndicator = GetWorld()->SpawnActor<ASSStrataIndicator>(ASSStrataIndicator::StaticClass());

	USSGameInstance* SSGameInstance = Cast<USSGameInstance>(GetGameInstance());
	USSStratagemManager* StratagemManager = SSGameInstance->GetStratagemManager();
	USSStratagem* SelectedStratagem = StratagemManager->GetStratagem(StratagemName);
	CurStrataIndicator->SetStratagem(SelectedStratagem);

	if (CurStrataIndicator)
	{
		USkeletalMeshComponent* PlayerSkeletalMesh = GetMesh();
		if (PlayerSkeletalMesh)
		{
			FName SocketName = TEXT("Socket_StrataIndicator");
			FAttachmentTransformRules AttachmentRules(
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepRelative,
				true);

			CurStrataIndicator->AttachToComponent(PlayerSkeletalMesh, AttachmentRules, SocketName);
		}
	}
}

bool ASSCharacterPlayer::ServerRpcStrataThrow_Validate()
{
	return true;
}

void ASSCharacterPlayer::ServerRpcStrataThrow_Implementation()
{
	const float AnimationSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(StrataThrowMontage, AnimationSpeedRate);

	FRotator ControlRotation = GetControlRotation();
	FRotator CurRotation = GetActorRotation();
	SetActorRotation(FRotator(CurRotation.Pitch, ControlRotation.Yaw, CurRotation.Roll));

	RpcPlayAnimation(StrataThrowMontage);
}

void ASSCharacterPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
