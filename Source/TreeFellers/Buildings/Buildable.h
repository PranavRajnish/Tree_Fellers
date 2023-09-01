// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buildable.generated.h"

class UBoxComponent;
class USnapCollider;

UCLASS()
class TREEFELLERS_API ABuildable : public AActor
{
	GENERATED_BODY()
	
public:	
	ABuildable();
	virtual void Tick(float DeltaTime) override;

	TArray<USnapCollider*> GetSnapColliders(FName TagName);
	void SetObjectMesh(UStaticMesh* NewMesh);


protected:
	virtual void BeginPlay() override;

//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
//	virtual void PostInitProperties() override;
//#endif

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ObjectMesh;

public:	

};
