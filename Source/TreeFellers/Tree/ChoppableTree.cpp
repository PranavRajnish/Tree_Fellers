// Fill out your copyright notice in the Description page of Project Settings.


#include "ChoppableTree.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"
#include "TreeFellers/Axe/Axe.h"

AChoppableTree::AChoppableTree()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	TreeStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticTreeMesh"));
	TreeStaticMesh->SetupAttachment(RootComponent);
	TreeStaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	TreeProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralTreeMesh"));
	TreeProcMesh->SetupAttachment(RootComponent);
	TreeProcMesh->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	//Tangents = TArray<FProcMeshTangent>();
}

void AChoppableTree::BeginPlay()
{
	Super::BeginPlay();
	
	if (TreeStaticMesh)
	{
		TreeStaticMesh->GetStaticMesh()->bAllowCPUAccess = true;
		TreeStaticMesh->SetVisibility(false);
		//UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(TreeStaticMesh, 0, TreeProcMesh, true);
		UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(TreeStaticMesh->GetStaticMesh(), 0, 0, Vertices, Triangles, Normals, UV0, Tangents);
		GenerateMesh();
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
		TreeProcMesh->UpdateMeshSection(0, Vertices, Normals, UV0, UpVertexColors, Tangents);

		TreeProcMesh->SetMaterial(0, TreeMaterial);
	}

}

void AChoppableTree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChoppableTree::GenerateMesh()
{
	TreeProcMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);

	NewVertices = Vertices;

	if (Recursions > 0)
	{
		for (int32 i = 0; i < Recursions; i++)
		{
			for (int32 j = 0; j < Triangles.Num() / 3; j++)
			{
				Subdivide(Triangles[IndexA], Triangles[IndexB], Triangles[IndexC]);
			}

			Vertices.Empty();
			Vertices = NewVertices;

			Triangles.Empty();
			Triangles = NewTriangles;
			NewTriangles.Empty();

			IndexA = 0;
			IndexB = 1;
			IndexC = 2;

			VertexSet.Empty();
			IndicesSet.Empty();
		}

		TreeProcMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
	}
}

// Subdivides a triangle into 4 more triangles, thereby increasing the geometry of the mesh.
void AChoppableTree::Subdivide(int32 A, int32 B, int32 C)
{
	// Endpoints
	FVector VA = Vertices[A];
	FVector VB = Vertices[B];
	FVector VC = Vertices[C];

	// Midpoints
	FVector VAB = FMath::Lerp(VA, VB, 0.5f);
	FVector VBC = FMath::Lerp(VB, VC, 0.5f);
	FVector VCA = FMath::Lerp(VC, VA, 0.5f);

	// Endpoint Indeces
	TempIndexA = A;
	TempIndexB = B;
	TempIndexC = C;

	// Midpoint duplicate bools
	bool VABDuplicate = false;
	bool VBCDuplicate = false;
	bool VCADuplicate = false;

	// Check for midpoint duplicates

	for (int32 i = 0; i < VertexSet.Num(); i++)
	{
		if (VAB == VertexSet[i])
		{
			VABDuplicate = true;
			TempIndexAB = IndicesSet[i];
		}
		if (VBC == VertexSet[i])
		{
			VBCDuplicate = true;
			TempIndexBC = IndicesSet[i];
		}
		if (VCA == VertexSet[i])
		{
			VCADuplicate = true;
			TempIndexCA = IndicesSet[i];
		}
	}

	if (!VABDuplicate)
	{
		NewVertices.Add(VAB);
		VertexSet.Add(VAB);
		IndicesSet.Add(NewVertices.Num() - 1);
		TempIndexAB = NewVertices.Num() - 1;
	}
	if (!VBCDuplicate)
	{
		NewVertices.Add(VBC);
		VertexSet.Add(VBC);
		IndicesSet.Add(NewVertices.Num() - 1);
		TempIndexBC = NewVertices.Num() - 1;
	}
	if (!VCADuplicate)
	{
		NewVertices.Add(VCA);
		VertexSet.Add(VCA);
		IndicesSet.Add(NewVertices.Num() - 1);
		TempIndexCA = NewVertices.Num() - 1;
	}

	// First Triangle
	NewTriangles.Add(TempIndexA);
	NewTriangles.Add(TempIndexAB);
	NewTriangles.Add(TempIndexCA);

	// Second Triangle
	NewTriangles.Add(TempIndexCA);
	NewTriangles.Add(TempIndexBC);
	NewTriangles.Add(TempIndexC);

	// Third Triangle
	NewTriangles.Add(TempIndexAB);
	NewTriangles.Add(TempIndexB);
	NewTriangles.Add(TempIndexBC);

	// Fourth Triangle
	NewTriangles.Add(TempIndexAB);
	NewTriangles.Add(TempIndexBC);
	NewTriangles.Add(TempIndexCA);

	IndexA += 3;
	IndexB += 3;
	IndexC += 3;
}

void AChoppableTree::AxeImpact(FVector ImpactLocation, FVector ImpactNormal, AAxe* Axe)
{
	if (Vertices.Num() == 0) return;

	//UE_LOG(LogTemp, Warning, TEXT("Impact Location : %s"), *ImpactLocation.ToString());

	/*DrawDebugLine(GetWorld(), ImpactLocation, ImpactLocation + (ImpactNormal * 100.f), FColor::Blue, false, 10.f, 0, 2.f);
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		FVector GlobalVertexPosition = GetActorLocation() + Vertices[i];
		if ((GlobalVertexPosition - ImpactLocation).Size() < Axe->GetImpactRadius())
		{
			UE_LOG(LogTemp, Warning, TEXT("Vertice before impact: %s"), *Vertices[i].ToString());
			FVector ImpactDirection = -ImpactNormal;
			ImpactDirection.Normalize();
			UE_LOG(LogTemp, Warning, TEXT("Vertice shift direction: %s"), *ImpactDirection.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Vertice shift ratio: %d"), (GlobalVertexPosition - ImpactLocation).Size() / Axe->GetImpactRadius());
			float VertexShift = FMath::Lerp(Axe->GetImpactDepth(), 0.f, (GlobalVertexPosition - ImpactLocation).Size() / Axe->GetImpactRadius());
			UE_LOG(LogTemp, Warning, TEXT("Vertice shift amount: %d"), VertexShift);
			Vertices[i] = Vertices[i] + (ImpactDirection * VertexShift);
			UE_LOG(LogTemp, Warning, TEXT("Vertice after impact: %s"), *Vertices[i].ToString());
		}
	}*/

	FVector ClosestVertex = GetActorLocation() + Vertices[0];
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		FVector GlobalVertexPosition = GetActorLocation() + Vertices[i];
		if ((GlobalVertexPosition - ImpactLocation).Size() < (ClosestVertex - ImpactLocation).Size())
		{
			ClosestVertex = GlobalVertexPosition;
		}
	}

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		float DistanceToImpactVertex = (ClosestVertex - (Vertices[i] + GetActorLocation())).Size();
		if (DistanceToImpactVertex < Axe->GetImpactRadius())
		{
			FVector ImpactDirection = FVector(-Vertices[i].X, -Vertices[i].Y, 0);
			ImpactDirection.Normalize();
			float VertexShift = FMath::Lerp(Axe->GetImpactDepth(), 0.f, DistanceToImpactVertex / Axe->GetImpactRadius());
			Vertices[i] = Vertices[i] + (ImpactDirection * VertexShift);
		}
	}

	TreeProcMesh->UpdateMeshSection(0, Vertices, Normals, UV0, UpVertexColors, Tangents);
}
