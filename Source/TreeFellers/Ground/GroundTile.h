// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GroundTile.generated.h"
/**
 * 
 */

USTRUCT()
struct TREEFELLERS_API FGroundTile
{
	GENERATED_BODY()

	FGroundTile(const FVector& TL, const FVector& C, const FVector& BR);
	FGroundTile();
	

public:
	FVector TopLeft;
	FVector Center;
	FVector BottomRight;
	bool bCanTreeBeHere = false;
};
