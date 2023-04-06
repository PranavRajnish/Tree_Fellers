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

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;

public:	

};
