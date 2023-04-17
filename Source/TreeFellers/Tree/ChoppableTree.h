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
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCalculateAxeImpact(FVector ImpactLocation, float ImpactRadius, float ImpactDepth);

protected:
	virtual void BeginPlay() override;

	void GenerateMesh();
	void Subdivide(int32 A, int32 B, int32 C);

	void FitPhysicsCapsuleToSplit(FVector SplitPoint);
	void SplitTree(FVector SplitPoint, FVector PlaneNormal);

	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	UProceduralMeshComponent* TreeProcMesh;
	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	UProceduralMeshComponent* TreeSplitProcMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defaults")
	class UCapsuleComponent* Capsule;

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<int32> Triangles;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	TArray<FLinearColor> VertexColors;

	TArray<FVector2D> UV0;
	TArray<FColor> UpVertexColors;
	TArray<FProcMeshTangent> Tangents;

	UPROPERTY()
	TArray<int32> NewTriangles;
	UPROPERTY()
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
	void CalculateMeshThickness(FVector ImpactPoint);

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* TreeStaticMesh;
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* TreeMaterial;

	// Calculating Thickness of Trunk
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float MinDistanceFromCenter = 5.f;
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float ThicknessThresholdForSplittingTree = 5.f;
	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	int32 CurrentMinimumThicknessOfTrunk = 100.f;
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float ThicknessTraceSpread = 5.f;
	UPROPERTY(VisibleAnywhere, Category = "Defaults")
	FVector FallDirection;
	UPROPERTY()
	float DistanceOfClosestVertexToCenter = 500.f;

public:	


};
