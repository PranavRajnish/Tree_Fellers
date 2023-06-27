// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "ProceduralGround.generated.h"

class UMaterialInterface;

UCLASS()
class TREEFELLERS_API AProceduralGround : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralGround();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	int XSize = 0;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	int YSize = 0;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	float ZMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	float NoiseScale = 1.0f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.00001))
	int Scale = 1;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.00001))
	int UVScale = 1;

protected:

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* GroundMaterial;

private:
	UProceduralMeshComponent* ProceduralMesh;
	TArray<FVector> Vertices;

	TArray<int32> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;

	void CreateVertices();
	void CreateTriangles();
};
