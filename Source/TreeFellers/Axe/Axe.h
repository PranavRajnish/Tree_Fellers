// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Axe.generated.h"

UCLASS()
class TREEFELLERS_API AAxe : public AActor
{
	GENERATED_BODY()
	
public:	
	AAxe();
	virtual void Tick(float DeltaTime) override;

	void CalculateAxeCollision();
	void PlaySwingSound();


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* CollisionPoint;

	UPROPERTY(VisibleAnywhere)
	class APlayerCharacter* Player;

	UPROPERTY(EditAnywhere)
	class USoundCue* WeaponSwingSFX;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	float CollisionRadius = 20.f;
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float ImpactRadius = 20.f;
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float ImpactDepth = 10.f;
	bool bHasHitThisSwing = false;

public:	
	FORCEINLINE void SetHasHitThisSwing(bool bHasHit) { bHasHitThisSwing = bHasHit; }
	FORCEINLINE float GetImpactRadius() const { return ImpactRadius; }
	FORCEINLINE float GetImpactDepth() const { return ImpactDepth; }
};
