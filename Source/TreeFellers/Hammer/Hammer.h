// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hammer.generated.h"

UCLASS()
class TREEFELLERS_API AHammer : public AActor
{
	GENERATED_BODY()
	
public:
	AHammer();
	virtual void Tick(float DeltaTime) override;

	void CalculateCollision();
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

	bool bHasHitThisSwing = false;


public:
	FORCEINLINE void SetHasHitThisSwing(bool bHasHit) { bHasHitThisSwing = bHasHit; }
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }

};
