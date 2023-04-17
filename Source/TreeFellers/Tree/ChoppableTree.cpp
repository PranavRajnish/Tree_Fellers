// Fill out your copyright notice in the Description page of Project Settings.


#include "ChoppableTree.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "TreeFellers/Axe/Axe.h"
#include "Components/CapsuleComponent.h"

AChoppableTree::AChoppableTree()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	TreeStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticTreeMesh"));
	TreeStaticMesh->SetupAttachment(RootComponent);
	TreeStaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PhysicsCapsule"));
	Capsule->SetupAttachment(RootComponent);
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	Capsule->SetSimulatePhysics(false);
	Capsule->SetCapsuleHalfHeight(205.0f);

	TreeProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralTreeMesh"));
	TreeProcMesh->SetupAttachment(Capsule);
	TreeProcMesh->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	TreeSplitProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SplitProceduralMesh"));
	TreeSplitProcMesh->SetupAttachment(RootComponent);

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
		UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(TreeStaticMesh->GetStaticMesh(), 0, 0, Vertices, Triangles, 
			Normals, UV0, Tangents);
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

	/*DrawDebugCylinder(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0.f, 0.f, 400.f), 
		MinDistanceFromCenter, 16, FColor::Orange, false, 0.1f);*/
	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 200.f),
		(GetActorLocation() + FVector(0.f, 0.f, 200.f)) + FallDirection * 200.f, 20.f, FColor::Cyan, false, 0.1f, (uint8)0U, 5.f);

	/*if (Capsule)
	{
		UE_LOG(LogTemp, Warning, TEXT("Capsule component location: %s"), *Capsule->GetComponentLocation().ToString());
		UE_LOG(LogTemp, Warning, TEXT("Capsule relative location: %s"), *Capsule->GetRelativeLocation().ToString());
		UE_LOG(LogTemp, Warning, TEXT("Capsule height: %f"), Capsule->GetScaledCapsuleHalfHeight());
	}*/

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

// Deforms Tree Mesh based on Axe Impact.
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

		FVector ImpactDirection = FVector(-Vertices[i].X, -Vertices[i].Y, 0);
		float DistanceToCenter = ImpactDirection.Size();
		ImpactDirection.Normalize();
		
		if (DistanceToImpactVertex < ImpactRadius && DistanceToCenter > MinDistanceFromCenter)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Before Shift: %s"), *Vertices[i].ToString());
			UpVertexColors[i] = FColor::Red;
			float VertexShift = FMath::Lerp(ImpactDepth, 0.f, DistanceToImpactVertex / ImpactRadius);

			FVector TempShift = Vertices[i] + (ImpactDirection * VertexShift);
			FVector BoundaryForCurrentVertice = FVector(0.f, 0.f, Vertices[i].Z) + ((-ImpactDirection) * MinDistanceFromCenter);
			FVector VectorToBoundary = BoundaryForCurrentVertice - Vertices[i];
			if ((ImpactDirection * VertexShift).Size() > VectorToBoundary.Size())
			{
				Vertices[i] = BoundaryForCurrentVertice;
			}
			else
			{
				Vertices[i] = TempShift;
			}
			//UE_LOG(LogTemp, Warning, TEXT("After Shift: %s"), *Vertices[i].ToString());
		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
	TreeProcMesh->UpdateMeshSection(0, Vertices, Normals, UV0, UpVertexColors, Tangents);

	if (HasAuthority())
	{
		CalculateMeshThickness(ClosestVertex);
	}
}

void AChoppableTree::CalculateMeshThickness(FVector ImpactVertice)
{
	float TotalThicknessOfTraces = 0.f;
	FVector ZVector;
	FVector TraceDirection;
	FVector TraceDirectionPerpendicular;
	FVector LocalPosition = ImpactVertice - GetActorLocation();

	for (int32 i = -ThicknessTraceSpread; i <= ThicknessTraceSpread; i+= ThicknessTraceSpread)
	{
		for (int32 j = -ThicknessTraceSpread; j <= ThicknessTraceSpread; j+= ThicknessTraceSpread)
		{
			FVector IncomingLocation;
			FVector OutgoingLocation;

			TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
			TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));
			TArray<FHitResult> HitResults;

			ZVector = FVector(0.f, 0.f, 1.f);
			// Trace in hit direction

			TraceDirection = FVector(-LocalPosition.X, -LocalPosition.Y, 0);
			TraceDirection.Normalize();

			TraceDirectionPerpendicular = UKismetMathLibrary::Cross_VectorVector(TraceDirection, ZVector);
			TraceDirectionPerpendicular.Normalize();

			FVector NewImpactVertice = ImpactVertice + i * TraceDirectionPerpendicular + j * ZVector;
			
			UKismetSystemLibrary::LineTraceMultiForObjects(this, NewImpactVertice - TraceDirection * 50.f, 
				NewImpactVertice + TraceDirection * 200.f, TraceObjectTypes, false, TArray<AActor*>(), 
				EDrawDebugTrace::None, HitResults, false);

			for (auto& HitResult : HitResults)
			{
				if (HitResult.GetActor() == this)
				{
					IncomingLocation = HitResult.Location;
				}
			}

			// Trace in opposite direction

			UKismetSystemLibrary::LineTraceMultiForObjects(this, NewImpactVertice + TraceDirection * 250.f, 
				NewImpactVertice - TraceDirection * 50.f, TraceObjectTypes, false, TArray<AActor*>(), 
				EDrawDebugTrace::None, HitResults, false, FLinearColor::Blue);

			for (auto& HitResult : HitResults)
			{
				if (HitResult.GetActor() == this)
				{
					OutgoingLocation = HitResult.Location;
				}
			}

			/*UE_LOG(LogTemp, Warning, TEXT("IncomingHit: %s"), *IncomingLocation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("OutgoingHit: %s"), *OutgoingLocation.ToString());*/

			TotalThicknessOfTraces += (OutgoingLocation - IncomingLocation).Size();
		}
	}
	
	// Calculating Fall Direction.
	if (FVector(LocalPosition.X, LocalPosition.Y, 0).Size() < DistanceOfClosestVertexToCenter)
	{
		FallDirection = FVector(LocalPosition.X, LocalPosition.Y, 0).GetSafeNormal();
		DistanceOfClosestVertexToCenter = FVector(LocalPosition.X, LocalPosition.Y, 0).Size();
	}

	CurrentMinimumThicknessOfTrunk = FMath::Min(CurrentMinimumThicknessOfTrunk, TotalThicknessOfTraces / 9);

	if (CurrentMinimumThicknessOfTrunk <= ThicknessThresholdForSplittingTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplitTree"));
		SplitTree(ImpactVertice, ZVector);
	}

}

void  AChoppableTree::SplitTree(FVector SplitPoint, FVector PlaneNormal)
{
	FitPhysicsCapsuleToSplit(SplitPoint);

	UKismetProceduralMeshLibrary::SliceProceduralMesh(TreeProcMesh, SplitPoint, PlaneNormal, true, TreeSplitProcMesh, 
		EProcMeshSliceCapOption::CreateNewSectionForCap, TreeMaterial);

	Capsule->SetSimulatePhysics(true);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->AddImpulse(FallDirection * 100.f, FName(NAME_None), true);
	
}

void AChoppableTree::FitPhysicsCapsuleToSplit(FVector SplitPoint)
{
	if (!Capsule) return;

	FVector LocalPosition = SplitPoint - GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("Local Position of Impact: %s"), *LocalPosition.ToString())
	float DistanceToTopOfTree =  2 * Capsule->GetScaledCapsuleHalfHeight() - LocalPosition.Z;
	UE_LOG(LogTemp, Warning, TEXT("Distance to Top: %f"), DistanceToTopOfTree);
	Capsule->SetCapsuleHalfHeight(DistanceToTopOfTree/2);
	Capsule->SetRelativeLocation(FVector(Capsule->GetRelativeLocation().X, Capsule->GetRelativeLocation().Y,
		Capsule->GetRelativeLocation().Z + LocalPosition.Z/2));
	TreeProcMesh->SetRelativeLocation((FVector(TreeProcMesh->GetRelativeLocation().X, TreeProcMesh->GetRelativeLocation().Y,
		TreeProcMesh->GetRelativeLocation().Z - LocalPosition.Z / 2)));
}

