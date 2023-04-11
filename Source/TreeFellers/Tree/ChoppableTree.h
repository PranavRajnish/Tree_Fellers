// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "ChoppableTree.generated.h"

UCLASS()
class TREEFELLERS_API AChoppableTree : public AActor
{
	GENERATED_BODY()
	
public:	
	AChoppableTree();
	virtual void Tick(float DeltaTime) override;

	void AxeImpact(FVector ImpactLocation, FVector ImpactNormal, class AAxe* Axe);

protected:
	virtual void BeginPlay() override;

	void GenerateMesh();
	void Subdivide(int32 A, int32 B, int32 C);


	UPROPERTY(EditAnywhere, Category = "Defaults")
	UProceduralMeshComponent* TreeProcMesh;

	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	TArray<FVector> Vertices;

	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	TArray<int32> Triangles;

	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	TArray<FVector> Normals;

	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	TArray<FLinearColor> VertexColors;

	TArray<FVector2D> UV0;
	TArray<FColor> UpVertexColors;
	TArray<FProcMeshTangent> Tangents;

	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	TArray<int32> NewTriangles;
	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	TArray<FVector> NewVertices;

	int32 IndexA = 0;
	int32 IndexB = 1;
	int32 IndexC = 2;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	int32 Recursions;

	TArray<FVector> VertexSet;
	TArray<int32> IndicesSet;

	int32 TempIndexA;
	int32 TempIndexB;
	int32 TempIndexC;
	int32 TempIndexAB;
	int32 TempIndexBC;
	int32 TempIndexCA;


private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* TreeStaticMesh;
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* TreeMaterial;

	//TArray<FVector>& GetVerticesOfStaticMesh();

public:	


};
