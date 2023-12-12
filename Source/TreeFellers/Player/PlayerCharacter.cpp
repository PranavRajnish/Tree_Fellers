// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "TreeFellers/Axe/Axe.h"
#include "TreeFellers/Hammer/Hammer.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraShakeBase.h"
#include "BuildComponent.h"
#include "TreeFellers/Buildings/Door.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	PlayerCamera->SetupAttachment(GetMesh(), FName("head"));
	PlayerCamera->bUsePawnControlRotation = true;

	BuildComponent = CreateDefaultSubobject<UBuildComponent>(TEXT("Build Component"));
	AddOwnedComponent(BuildComponent);
	BuildComponent->SetIsReplicated(true);

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
	Hammer = GetWorld()->SpawnActor<AHammer>(HammerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (Axe)
	{
		Axe->SetOwner(this);
		Axe->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
	}
	if (Hammer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawning Hammer.."));
		Hammer->SetOwner(this);
		Hammer->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
		Hammer->GetMesh()->SetVisibility(false);
	}
	if (BuildComponent)
	{
		BuildComponent->SetCameraComponent(PlayerCamera);
		bIsBuildModeOn = false;
		BuildComponent->SetBuildModeOn(bIsBuildModeOn);
	}

	PlayerController = Cast<APlayerController>(Controller);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && bCalculateAttackCollision)
	{
		if (!bIsBuildModeOn && Axe)
		{
			Axe->CalculateAxeCollision();
		}
		if (bIsBuildModeOn && Hammer)
		{
			Hammer->CalculateCollision();
		}
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
	PlayerInputComponent->BindAction("BuildMode", EInputEvent::IE_Pressed, this, &ThisClass::BuildButtonPressed);
	PlayerInputComponent->BindAction("Build", EInputEvent::IE_Pressed, this, &ThisClass::PlaceBuildPressed);
	PlayerInputComponent->BindAction("Interact", EInputEvent::IE_Pressed, this, &ThisClass::InteractButtonPressed);
	PlayerInputComponent->BindAction("MouseWheelUp", EInputEvent::IE_Pressed, this, &ThisClass::MouseWheelUp);
	PlayerInputComponent->BindAction("MouseWheelDown", EInputEvent::IE_Pressed, this, &ThisClass::MouseWheelDown);
	PlayerInputComponent->BindAction("RotateBuildClockwise", EInputEvent::IE_Pressed, this, &ThisClass::RotateBuildClockwise);
	PlayerInputComponent->BindAction("RotateBuildAntiClockwise", EInputEvent::IE_Pressed, this, &ThisClass::RotateBuildAntiClockwise);
	PlayerInputComponent->BindAction("RotateBuildClockwise", EInputEvent::IE_Released, this, &ThisClass::StopRotatingBuild);
	PlayerInputComponent->BindAction("RotateBuildAntiClockwise", EInputEvent::IE_Released, this, &ThisClass::StopRotatingBuild);
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
	if (!bIsSwingingAxe && !bIsBuildModeOn)
	{
		if (!HasAuthority())
		{
			PlayAnimMontage(AxeSwing);	
			
			if (Axe)
			{
				Axe->PlaySwingSound();
			}
			if (SwingCameraShake && PlayerController)
			{
				
				PlayerController->PlayerCameraManager->StartCameraShake(SwingCameraShake);
			}
		}

		bIsSwingingAxe = true;
		ServerSwingTool();
		
	}
}

void APlayerCharacter::BuildButtonPressed()
{
	if (BuildComponent && !bIsSwingingAxe)
	{
		UE_LOG(LogTemp, Warning, TEXT("Build Button Pressed"));
		bIsBuildModeOn = !bIsBuildModeOn;
		//SwitchTool(bIsBuildModeOn);
		SetToolVisibility(!bIsBuildModeOn);
		BuildComponent->SetBuildModeOn(bIsBuildModeOn);

	}
}

void APlayerCharacter::PlaceBuildPressed()
{
	if (BuildComponent)
	{
		BuildComponent->PlaceBuilding();
	}
}

void APlayerCharacter::InteractButtonPressed()
{
	if (InteractionObject)
	{
		InteractionObject->Interact();
	}
}

void APlayerCharacter::RotateBuildClockwise()
{
	if (BuildComponent)
	{
		BuildComponent->StartRotateBuild(true);
	}
}

void APlayerCharacter::RotateBuildAntiClockwise()
{
	if (BuildComponent)
	{
		BuildComponent->StartRotateBuild(false);
	}
}

void APlayerCharacter::StopRotatingBuild()
{
	if (BuildComponent)
	{
		BuildComponent->StopRotatingBuild();
	}
}

void APlayerCharacter::MouseWheelUp()
{
	if (BuildComponent)
	{
		BuildComponent->NextBuildObject();
	}
}

void APlayerCharacter::MouseWheelDown()
{
	if (BuildComponent)
	{
		BuildComponent->PreviousBuildObject();
	}
}

#pragma endregion Player Input 

void APlayerCharacter::SwitchTool(bool bSwitchToHammer)
{
	if (!Axe || !Hammer) return;

	if (bSwitchToHammer)
	{
		Axe->GetMesh()->SetVisibility(false);
		Hammer->GetMesh()->SetVisibility(true);
	}
	else
	{
		Axe->GetMesh()->SetVisibility(true);
		Hammer->GetMesh()->SetVisibility(false);
	}

	ServerSwitchTool(bSwitchToHammer);
}

void APlayerCharacter::ServerSwitchTool_Implementation(bool bSwitchToHammer)
{
	MulticastSwitchTool(bSwitchToHammer);
}

void APlayerCharacter::MulticastSwitchTool_Implementation(bool bSwitchToHammer)
{
	if (!IsLocallyControlled())
	{
		if (bSwitchToHammer)
		{
			Axe->GetMesh()->SetVisibility(false);
			Hammer->GetMesh()->SetVisibility(true);
		}
		else
		{
			Axe->GetMesh()->SetVisibility(true);
			Hammer->GetMesh()->SetVisibility(false);
		}
	}
}

void APlayerCharacter::ServerSwingTool_Implementation()
{
	if (IsLocallyControlled())
	{
		if (bIsBuildModeOn)
		{
			PlayAnimMontage(AxeSwing);

			if (Hammer)
			{
				Hammer->PlaySwingSound();
			}
			if (SwingCameraShake && PlayerController)
			{
				//PlayerController->PlayerCameraManager->StartCameraShake(SwingCameraShake);
			}
		}
		else
		{
			PlayAnimMontage(AxeSwing);

			if (Axe)
			{
				Axe->PlaySwingSound();
			}
			if (SwingCameraShake && PlayerController)
			{
				//PlayerController->PlayerCameraManager->StartCameraShake(SwingCameraShake);
			}
		}
		
	}

	MulticastSwingTool();
}

void APlayerCharacter::MulticastSwingTool_Implementation()
{
	if (!IsLocallyControlled())
	{
		if (bIsBuildModeOn)
		{
			PlayAnimMontage(AxeSwing);

			if (Hammer)
			{
				Hammer->PlaySwingSound();
			}
		}
		else
		{
			PlayAnimMontage(AxeSwing);

			if (Axe)
			{
				Axe->PlaySwingSound();
			}
		}
	}

}

void APlayerCharacter::SetToolVisibility(bool visiblity)
{
	if (Axe)
	{
		Axe->SetActorHiddenInGame(!visiblity);
	}
	if (Hammer)
	{
		Hammer->SetActorHiddenInGame(!visiblity);
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
	if (Hammer)
	{
		Hammer->SetHasHitThisSwing(false);
	}
}

void APlayerCharacter::AxeImpact()
{
	StopAnimMontage(AxeSwing);
	StopCalculateAttackCollision();
	SetIsSwingingAxe(false);

	if (SwingCameraShake && PlayerController)
	{
		PlayerController->PlayerCameraManager->StartCameraShake(SwingCameraShake);
	}
}