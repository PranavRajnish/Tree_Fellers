// Fill out your copyright notice in the Description page of Project Settings.


#include "ChoppableTree.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "TreeFellers/Axe/Axe.h"

AChoppableTree::AChoppableTree()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

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
		for (int32 i = 0; i < Vertices.Num(); i++)
		{
			UpVertexColors.Add(FColor::Blue);
			VertexColors.Add(FLinearColor::Blue);
		}
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
	MulticastCalculateAxeImpact(ImpactLocation, Axe->GetImpactRadius(), Axe->GetImpactDepth());
}

void AChoppableTree::MulticastCalculateAxeImpact_Implementation(FVector ImpactLocation, float ImpactRadius, float ImpactDepth)
{
	 
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
		float DistanceToCenter = 0.f;
		if (DistanceToImpactVertex < ImpactRadius)
		{
			UpVertexColors[i] = FColor::Red;
			FVector ImpactDirection = FVector(-Vertices[i].X, -Vertices[i].Y, 0);
			ImpactDirection.Normalize();
			float VertexShift = FMath::Lerp(ImpactDepth, 0.f, DistanceToImpactVertex / ImpactRadius);
			Vertices[i] = Vertices[i] + (ImpactDirection * VertexShift);
			/*if ((Vertices[i] - FVector(0, 0, Vertices[i].Z)).Size() <= DistanceTresholdToCenter)
			{
				NumberOfVerticesCloseToCenter++;
			}*/
		}
		/*
		else
		{
			if ((Vertices[i] - FVector(0, 0, Vertices[i].Z)).Size() <= DistanceTresholdToCenter)
			{
				NumberOfVerticesCloseToCenter++;
			}
		}
		*/
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
	TreeProcMesh->UpdateMeshSection(0, Vertices, Normals, UV0, UpVertexColors, Tangents);

	/*
	if (NumberOfVerticesCloseToCenter >= NumberOfCentralVerticesToSplit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Split Tree"));
	}
	LastNumberOfVerticesCloseToCenter = NumberOfVerticesCloseToCenter;
	NumberOfVerticesCloseToCenter = 0;
	*/

	CalculateMeshThicknes(ClosestVertex);

}

void AChoppableTree::CalculateMeshThicknes(FVector ImpactVertice)
{
	float TotalThicknessOfTraces = 0.f;
	for (int32 i = -ThicknessTraceSpread; i <= ThicknessTraceSpread; i+= ThicknessTraceSpread)
	{
		for (int32 j = -ThicknessTraceSpread; j <= ThicknessTraceSpread; j+= ThicknessTraceSpread)
		{
			FVector IncomingLocation;
			FVector OutgoingLocation;

			TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
			TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));
			TArray<FHitResult> HitResults;

			FVector ZVector = FVector(0.f, 0.f, 1.f);
			// Trace in hit direction
			FVector LocalPosition = ImpactVertice - GetActorLocation();
			FVector TraceDirection = FVector(-LocalPosition.X, -LocalPosition.Y, 0);
			TraceDirection.Normalize();

			FVector TraceDirectionPerpendicular = UKismetMathLibrary::Cross_VectorVector(TraceDirection, ZVector);
			TraceDirectionPerpendicular.Normalize();

			FVector NewImpactVertice = ImpactVertice + i * TraceDirectionPerpendicular + j * ZVector;
			
			UKismetSystemLibrary::LineTraceMultiForObjects(this, NewImpactVertice - TraceDirection * 50.f, NewImpactVertice + TraceDirection * 200.f,
				TraceObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, HitResults, false);

			for (auto& HitResult : HitResults)
			{
				if (HitResult.GetActor() == this)
				{
					IncomingLocation = HitResult.Location;
				}
			}

			// Trace in opposite direction

			UKismetSystemLibrary::LineTraceMultiForObjects(this, NewImpactVertice + TraceDirection * 250.f, NewImpactVertice - TraceDirection * 50.f,
				TraceObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, HitResults, false, FLinearColor::Blue);

			for (auto& HitResult : HitResults)
			{
				if (HitResult.GetActor() == this)
				{
					OutgoingLocation = HitResult.Location;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("IncomingHit: %s"), *IncomingLocation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("OutgoingHit: %s"), *OutgoingLocation.ToString());

			TotalThicknessOfTraces += (OutgoingLocation - IncomingLocation).Size();
		}
	}
	
	CurrentMinimumThicknessOfTrunk = FMath::Min(CurrentMinimumThicknessOfTrunk, TotalThicknessOfTraces / 9);

	if (CurrentMinimumThicknessOfTrunk <= ThicknessThresholdForSplittingTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplitTree"));
	}

}
  
