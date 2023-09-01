// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "SnapCollider.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TREEFELLERS_API USnapCollider : public UBoxComponent
{
	GENERATED_BODY()

private:
	bool bIsInUse = false;

public:
	FORCEINLINE bool GetIsInUse() const { return bIsInUse; }
	FORCEINLINE void SetIsInUse(bool Value) { bIsInUse = Value; }
	
};
