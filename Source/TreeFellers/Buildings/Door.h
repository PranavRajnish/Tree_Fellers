// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Buildable.h"
#include "Door.generated.h"

class USphereComponent;
class UTimelineComponent;

/**
 * 
 */


UCLASS()
class TREEFELLERS_API ADoor : public ABuildable
{
	GENERATED_BODY()

public:
	ADoor();

	UFUNCTION()
	void OnInteractionOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractionOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void DoorTimelineFloatReturn(float value);
	UFUNCTION()
	void OnDoorTimelineFinished();

	void Interact();

protected:
	virtual void BeginPlay() override;

	/* Components */
	UPROPERTY(EditAnywhere)
	USceneComponent* Hinge;
	UPROPERTY(EditAnywhere)
	USphereComponent* InteractionSphere;

	// Paramters
	UPROPERTY(EditAnywhere, Category = "Parameters")
	FRotator ClosedRotation;
	UPROPERTY(EditAnywhere, Category = "Parameters")
	FRotator OpenRotation;
	UPROPERTY(EditAnywhere, Category = "Parameters")
	UCurveFloat* DoorCurve;


private:

	UTimelineComponent* DoorTimeline;
	bool bIsOpen = false;
	bool bIsInUse = false;
};
