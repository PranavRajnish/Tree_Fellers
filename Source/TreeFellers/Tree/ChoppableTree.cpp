// Fill out your copyright notice in the Description page of Project Settings.


#include "ChoppableTree.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"

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

void AChoppableTree::AxeImpact(FHitResult HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Axe Impact"));
	DrawDebugLine(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + (HitResult.ImpactNormal * 100.f), FColor::Blue, false, 10.f, 0, 2.f);
	/*for (int32 i = 0; i < Vertices.Num(); i++)
	{
		if ((Vertices[i] - ImpactPoint).Size() < ImpactDistance)
		{

		}
	}*/
}

//TArray<FVector>& AChoppableTree::GetVerticesOfStaticMesh()
//{
//	if (!TreeStaticMesh || !TreeStaticMesh->GetStaticMesh() || !TreeStaticMesh->GetStaticMesh()->GetRenderData()) return;
//
//	TArray<FVector> Vertices;
//
//	
//	if (TreeStaticMesh->GetStaticMesh()->GetRenderData()->LODResources.Num() > 0)
//	{
//		FPositionVertexBuffer* VertexBuffer = &TreeStaticMesh->GetStaticMesh()->GetRenderData()->LODResources[0].Get;
//		if (VertexBuffer)
//		{
//			const int32 VertexCount = VertexBuffer->GetNumVertices();
//			for (int32 Index = 0; Index < VertexCount; Index++)
//			{
//				//This is in the Static Mesh Actor Class, so it is location and tranform of the SMActor
//				const FVector WorldSpaceVertexLocation = **GetActorLocation() + GetTransform().TransformVector(VertexBuffer->VertexPosition(Index)); **
//					//add to output FVector array
//			}
//		}
//	}
//}

