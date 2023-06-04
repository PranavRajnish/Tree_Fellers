// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "TreeFellers/Axe/Axe.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	PlayerCamera->SetupAttachment(GetMesh(), FName("head"));
	PlayerCamera->bUsePawnControlRotation = true;

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); 
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Axe = GetWorld()->SpawnActor<AAxe>(AxeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (Axe)
	{
		Axe->SetOwner(this);
		Axe->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && bCalculateAttackCollision && Axe)
	{
		Axe->CalculateAxeCollision();
	}

}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, bIsSwingingAxe);
}

#pragma region PlayerInput

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
	PlayerInputComponent->BindAction("Attack", EInputEvent::IE_Pressed, this, &ThisClass::AttackButtonPressed);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
}

void APlayerCharacter::MoveForward(float AxisValue)
{
	if (Controller && AxisValue != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, AxisValue);
	}
}

void APlayerCharacter::MoveRight(float AxisValue)
{
	if (Controller && AxisValue != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, AxisValue);
	}
}

void APlayerCharacter::Turn(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

void APlayerCharacter::LookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

void APlayerCharacter::AttackButtonPressed()
{
	if (!bIsSwingingAxe)
	{
		if (!HasAuthority())
		{
			PlayAnimMontage(AxeSwing);
		}

		bIsSwingingAxe = true;
		ServerSwingAxe();
	}
}

#pragma endregion Player Input 


void APlayerCharacter::ServerSwingAxe_Implementation()
{
	PlayAnimMontage(AxeSwing);

	MulticastSwingAxe();
}

void APlayerCharacter::MulticastSwingAxe_Implementation()
{
	if (!IsLocallyControlled())
	{
		PlayAnimMontage(AxeSwing);
	}
}

void APlayerCharacter::StartCalculateAttackCollision()
{
	bCalculateAttackCollision = true;
}

void APlayerCharacter::StopCalculateAttackCollision()
{
	bCalculateAttackCollision = false;

	if (Axe)
	{
		Axe->SetHasHitThisSwing(false);
	}
}

void APlayerCharacter::AxeImpact()
{
	StopAnimMontage(AxeSwing);
	StopCalculateAttackCollision();
	SetIsSwingingAxe(false);
}