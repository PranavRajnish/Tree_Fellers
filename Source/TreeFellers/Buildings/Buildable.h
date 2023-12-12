// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buildable.generated.h"

class UBoxComponent;
class USnapCollider;
class UGeometryCollectionComponent;
class USoundCue;

UCLASS()
class TREEFELLERS_API ABuildable : public AActor
{
	GENERATED_BODY()
	
public:	
	ABuildable();
	virtual void Tick(float DeltaTime) override;

	TArray<USnapCollider*> GetSnapColliders(FName TagName);
	void SetObjectMesh(UStaticMesh* NewMesh);

	void Impacted(FVector ImpactPosition);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DestroyMesh(FVector ImpactPosition);

protected:
	virtual void BeginPlay() override;

//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
//	virtual void PostInitProperties() override;
//#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ObjectMesh;
	/*UPROPERTY(VisibleAnywhere)
	UGeometryCollectionComponent* GeometryCollectionComponent;*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* WallBreakSFX;

public:	

};
