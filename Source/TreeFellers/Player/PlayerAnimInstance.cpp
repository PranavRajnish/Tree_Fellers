// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if (!PlayerCharacter) return;

	Pitch = CalculateAOPitch();
	bIsSwingingAxe = PlayerCharacter->GetIsSwingingAxe();
}

void UPlayerAnimInstance::AxeSwingFinished()
{
	if (!PlayerCharacter) return;

	PlayerCharacter->SetIsSwingingAxe(false);
	
}

float UPlayerAnimInstance::CalculateAOPitch()
{
	if (!PlayerCharacter) return 0.0f;

	float AimPitch = PlayerCharacter->GetBaseAimRotation().Pitch;

	if (!PlayerCharacter->IsLocallyControlled())
	{
		// Map from 270-360 to -90-0
		if (AimPitch > 90.f)
		{
			FVector2D InRange(270.f, 360.f);
			FVector2D  OutRange(-90.f, 0.f);
			AimPitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimPitch);
		}	
	}
	else
	{
		AimPitch = AimPitch >= 180 ? 360 - AimPitch : AimPitch;
	}
	return AimPitch;
}