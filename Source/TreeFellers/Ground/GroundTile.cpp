// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundTile.h"

FGroundTile::FGroundTile()
{
	FGroundTile(FVector(), FVector(), FVector());
}

FGroundTile::FGroundTile(const FVector& TL, const FVector& C, const FVector& BR)
{
	TopLeft = TL;
	Center = C;
	BottomRight = BR;
}
