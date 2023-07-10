// Fill out your copyright notice in the Description page of Project Settings.


#include "ChoppableTree.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TreeFellers/Axe/Axe.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SplineComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"


AChoppableTree::AChoppableTree()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	CenterSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CenterSpline"));
	CenterSpline->SetupAttachment(RootComponent);

	TrunkStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrunkStaticMesh"));
	TrunkStaticMesh->SetupAttachment(RootComponent);
	TrunkStaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PhysicsCapsule"));
	Capsule->SetupAttachment(RootComponent);
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	Capsule->SetSimulatePhysics(false);
	Capsule->SetCapsuleHalfHeight(205.0f);
	Capsule->SetIsReplicated(true);

	StumpCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("StumpCapsule"));
	StumpCapsule->SetupAttachment(RootComponent);
	StumpCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StumpCapsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	StumpCapsule->SetSimulatePhysics(false);
	StumpCapsule->SetCapsuleHalfHeight(100.0f);

	TreeProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralTreeMesh"));
	TreeProcMesh->SetupAttachment(Capsule);
	TreeProcMesh->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	BranchesStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BranchesStaticMesh"));
	BranchesStaticMesh->SetupAttachment(TreeProcMesh);
	BranchesStaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	TreeStumpProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("StumpProceduralMesh"));
	TreeStumpProcMesh->SetupAttachment(StumpCapsule);

	GroundCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("GroundCollider"));
	GroundCollider->SetupAttachment(Capsule);
	GroundCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GroundCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
	GroundCollider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnGroundOverlap);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio Component"));
	AudioComponent->SetupAttachment(RootComponent);

	//Tangents = TArray<FProcMeshTangent>();
}

void AChoppableTree::BeginPlay()
{
	Super::BeginPlay();
	
	if (TrunkStaticMesh && TrunkStaticMesh->GetStaticMesh())
	{
		TrunkStaticMesh->GetStaticMesh()->bAllowCPUAccess = true;
		TrunkStaticMesh->SetVisibility(false);
		//UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(TreeStaticMesh, 0, TreeProcMesh, true);
		UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(TrunkStaticMesh->GetStaticMesh(), 0, 0, Vertices, Triangles,
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
	if (TreeFallingSFX)
	{
		AudioComponent->SetSound(TreeFallingSFX);
	}

}

void AChoppableTree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChoppableTree::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChoppableTree, FallDirection);
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
	if (Vertices.Num() <= 0)
		return;

	FTransform ActorTransform = GetActorTransform();
	FTransform InverseActorTransform = ActorTransform.Inverse();
	FVector ClosestVertex = GetActorLocation() + ActorTransform.TransformVector(Vertices[0]);

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		FVector GlobalVertexPosition = GetActorLocation() + ActorTransform.TransformVector(Vertices[i]);
		if ((GlobalVertexPosition - ImpactLocation).Size() < (ClosestVertex - ImpactLocation).Size())
		{
			ClosestVertex = GlobalVertexPosition;
		}
		//DrawDebugSphere(GetWorld(), GlobalVertexPosition, 1.f, 12, FColor::Red, false, 20.f);
	}

	//DrawDebugSphere(GetWorld(), ClosestVertex, 1.f, 12, FColor::Red, false, 20.f);
	FVector ImpactDirection = GetImpactDirectionForLocalPoint(ClosestVertex - GetActorLocation());
	ImpactDirection.Normalize();

	// Playing Impact Effects
	if (TreeImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, TreeImpactVFX, ImpactLocation + (ImpactDirection) * ImpactDepth, (-ImpactDirection).ToOrientationRotator());
	}
	if (TreeImpactSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, TreeImpactSFX, ImpactLocation);
	}

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		FVector LocalVertex = ActorTransform.TransformVector(Vertices[i]);
		float DistanceToImpactVertex = (ClosestVertex - (LocalVertex + GetActorLocation())).Size();


		// Calculating which vertices to move
		if (DistanceToImpactVertex < ImpactRadius)
		{
			ImpactDirection = GetImpactDirectionForLocalPoint(LocalVertex);

			//DrawDebugDirectionalArrow(GetWorld(), LocalVertex + GetActorLocation(), LocalVertex + GetActorLocation() + ImpactDirection, 2.0, FColor::Orange, false, 20.f);
			float DistanceToCenter = ImpactDirection.Size();
			ImpactDirection.Normalize();

			if (DistanceToCenter > MinDistanceFromCenter)
			{
				UpVertexColors[i] = FColor::Red;
				float VertexShift = FMath::Lerp(ImpactDepth, 0.f, FMath::Clamp(DistanceToImpactVertex / ImpactRadius, 0.f, 1.f));

				FVector TempShift = LocalVertex + (ImpactDirection * VertexShift);
				FVector BoundaryForCurrentVertice = (LocalVertex + (ImpactDirection * DistanceToCenter)) + ((-ImpactDirection) * MinDistanceFromCenter);
				FVector VectorToBoundary = BoundaryForCurrentVertice - LocalVertex;
				if ((ImpactDirection * VertexShift).Size() > VectorToBoundary.Size())
				{
					Vertices[i] = InverseActorTransform.TransformVector(BoundaryForCurrentVertice);
				}
				else
				{
					Vertices[i] = InverseActorTransform.TransformVector(TempShift);
				}
			}
		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
	TreeProcMesh->UpdateMeshSection(0, Vertices, Normals, UV0, UpVertexColors, Tangents);

	if (HasAuthority())
	{
		CalculateMeshThickness(ClosestVertex, ImpactDirection);
	}
}

void AChoppableTree::CalculateMeshThickness(FVector ImpactVertice, FVector ImpactDirection)
{
	float TotalThicknessOfTraces = 0.f;
	float TotalDistanceFromCenterOfTraces = 0.f;
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

			ZVector = GetActorUpVector();
			// Trace in hit direction
			//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorUpVector() * 100.f, 5.0, FColor::Cyan, false, 10.f);

			TraceDirection = ImpactDirection;
			TraceDirection.Normalize();

			TraceDirectionPerpendicular = UKismetMathLibrary::Cross_VectorVector(TraceDirection, ZVector);
			TraceDirectionPerpendicular.Normalize();

			FVector NewImpactVertice = ImpactVertice + i * TraceDirectionPerpendicular + j * ZVector;
			
			UKismetSystemLibrary::LineTraceMultiForObjects(this, NewImpactVertice - TraceDirection * 50.f, 
				NewImpactVertice + TraceDirection * 200.f, TraceObjectTypes, false, TArray<AActor*>(), 
				EDrawDebugTrace::None, HitResults, false);

			//DrawDebugLine(GetWorld(), NewImpactVertice - TraceDirection * 50.f, NewImpactVertice + TraceDirection * 200.f, FColor::Red, false, 20.f);
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

			FVector NewLocalImpact = NewImpactVertice - GetActorLocation();
			TotalDistanceFromCenterOfTraces += FVector(NewImpactVertice.X, NewImpactVertice.Y, 0).Size();
		}
	}
	
	// Calculating Fall Direction.
	if (TotalDistanceFromCenterOfTraces/9 < DistanceOfClosestVertexToCenter)
	{
		FallDirection = FVector(LocalPosition.X, LocalPosition.Y, 0).GetSafeNormal();
		DistanceOfClosestVertexToCenter = FVector(LocalPosition.X, LocalPosition.Y, 0).Size();
	}

	CurrentMinimumThicknessOfTrunk = FMath::Min(CurrentMinimumThicknessOfTrunk, TotalThicknessOfTraces / 9);

	if (CurrentMinimumThicknessOfTrunk <= ThicknessThresholdForSplittingTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplitTree"));
		MulticastSplitTree(ImpactVertice, ZVector);
	}

}


void AChoppableTree::MulticastSplitTree_Implementation(FVector SplitPoint, FVector ZVector)
{
	//DrawDebugDirectionalArrow(GetWorld(), SplitPoint, SplitPoint + PlaneNormal * 50.f, 5.f, FColor::White, true);
	FitPhysicsCapsuleToSplit(SplitPoint);

	UKismetProceduralMeshLibrary::SliceProceduralMesh(TreeProcMesh, SplitPoint, ZVector, true, TreeStumpProcMesh,
		EProcMeshSliceCapOption::CreateNewSectionForCap, TreeMaterial);

	if (HasAuthority())
	{
		Capsule->SetSimulatePhysics(true);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->AddImpulse(FallDirection * FallImpulse, FName(NAME_None), false);

		StumpCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	FTransform ActorTransform = GetActorTransform();
	FTransform InverseActorTransform = ActorTransform.Inverse();

	float HeightOfCap;
	bool bDoOnce = false;
	FProcMeshSection* CapSection = TreeStumpProcMesh->GetProcMeshSection(1);
	TArray<FVector> CapVertices;
	TArray<FVector> CapNormals;
	TArray<FVector2D> CapUV;
	TArray<FColor> CapColors;
	TArray<FProcMeshTangent> CapTangents;
	TArray<FVector> NewCapVertices;
	for (const auto &ver : CapSection->ProcVertexBuffer)
	{
		FVector LocalVertex = ActorTransform.TransformVector(ver.Position);
		if (!bDoOnce)
		{
			HeightOfCap = ver.Position.Z;
			bDoOnce = true;
		}

		CapVertices.Add(ver.Position);

		FVector ImpactDirection = GetImpactDirectionForLocalPoint(LocalVertex);

		float DistanceToCenter = ImpactDirection.Size();
		ImpactDirection.Normalize();

		if (DistanceToCenter > MinDistanceFromCenter)
		{
			FVector BoundaryForCurrentVertice = (LocalVertex + ImpactDirection * DistanceToCenter) + ((-ImpactDirection) * MinDistanceFromCenter);

			NewCapVertices.Add(InverseActorTransform.TransformVector(BoundaryForCurrentVertice));

		}
		else
		{
			NewCapVertices.Add(ver.Position);
		}

		//DrawDebugSphere(GetWorld(), GetActorLocation() + NewCapVertices[NewCapVertices.Num() - 1], 1.f, 4, FColor::Red, true, 10.f);
		CapNormals.Add(ver.Normal);
		CapUV.Add(ver.UV0);
		CapColors.Add(ver.Color);
		CapTangents.Add(ver.Tangent);
		//DrawDebugSphere(GetWorld(), GetActorLocation() + ver.Position, 0.5f, 4, FColor::Orange, true, 10.f);
	}

	TreeStumpProcMesh->UpdateMeshSection(1, NewCapVertices, CapNormals, CapUV, CapColors, CapTangents);

	FProcMeshSection* StumpSection = TreeStumpProcMesh->GetProcMeshSection(0);
	TArray<FVector> StumpVertices;
	TArray<FVector> StumpNormals;
	TArray<FVector2D> StumpUV;
	TArray<FColor> StumpColors;
	TArray<FProcMeshTangent> StumpTangents;
	for (const auto &ver : StumpSection->ProcVertexBuffer)
	{
		//DrawDebugSphere(GetWorld(), GetActorLocation() + ver.Position, 0.5f, 4, FColor::Purple, true, 10.f);
		bool bIsAlsoCapVertice = false;
		bool bisAlsoStumpColor = false;


		for (int32 i = 0; i < CapVertices.Num(); i++)
		{
			if ((ver.Position - CapVertices[i]).Size() < 0.5f)
			{
				//DrawDebugSphere(GetWorld(), GetActorLocation() + ver.Position, 1.f, 4, FColor::Cyan, true, 10.f);
				//UE_LOG(LogTemp, Warning, TEXT("Cap Color: %s"), *ver.Color.ToString());

				StumpVertices.Add(NewCapVertices[i]);
				bIsAlsoCapVertice = true;
				break;
			}
			//if ((ver.Position - CapVertices[i]).Size() < DistanceFromImpactToBecomeStumpColor)
			//{
			//	StumpColors.Add(FColor::Red);
			//	bisAlsoStumpColor = true;
			//	//DrawDebugSphere(GetWorld(), GetActorLocation() + ver.Position, 1.f, 4, FColor::Cyan, true, 10.f);
			//	break;
			//}
		}
		if (!bIsAlsoCapVertice)
		{
			StumpVertices.Add(ver.Position);
		}
		//DrawDebugSphere(GetWorld(), GetActorLocation() + StumpVertices[StumpVertices.Num() - 1], 0.5f, 4, FColor::Purple, true, 10.f);
		StumpNormals.Add(ver.Normal);
		StumpUV.Add(ver.UV0);	
		StumpTangents.Add(ver.Tangent);
		if (!bisAlsoStumpColor)
		{
			StumpColors.Add(ver.Color);
		}
	}

	TreeStumpProcMesh->UpdateMeshSection(0, StumpVertices, StumpNormals, StumpUV, StumpColors, StumpTangents);

	//GetWorld()->GetTimerManager().SetTimer(TreeFallingSFXDelayHandle, this, &ThisClass::PlayTreeFallingSFX, TreeFallingSFXDelay);
	PlayTreeFallingSFX();
}

void AChoppableTree::FitPhysicsCapsuleToSplit(FVector SplitPoint)
{
	if (!Capsule) return;

	FVector LocalPosition = SplitPoint - GetActorLocation();
	float DistanceToTopOfTree =  2 * Capsule->GetScaledCapsuleHalfHeight() - LocalPosition.Z;
	Capsule->SetCapsuleHalfHeight(DistanceToTopOfTree/2);
	Capsule->SetRelativeLocation(FVector(Capsule->GetRelativeLocation().X, Capsule->GetRelativeLocation().Y,
		Capsule->GetRelativeLocation().Z + LocalPosition.Z/2));
	TreeProcMesh->SetRelativeLocation((FVector(TreeProcMesh->GetRelativeLocation().X, TreeProcMesh->GetRelativeLocation().Y,
		TreeProcMesh->GetRelativeLocation().Z - LocalPosition.Z / 2)));

	if (!StumpCapsule) return;

	StumpCapsule->SetCapsuleHalfHeight(LocalPosition.Z/2);
	StumpCapsule->SetRelativeLocation(FVector(StumpCapsule->GetRelativeLocation().X, StumpCapsule->GetRelativeLocation().Y,
		StumpCapsule->GetRelativeLocation().Z + LocalPosition.Z / 2));
	TreeStumpProcMesh->SetRelativeLocation(FVector(TreeStumpProcMesh->GetRelativeLocation().X, TreeStumpProcMesh->GetRelativeLocation().Y,
		TreeStumpProcMesh->GetRelativeLocation().Z - LocalPosition.Z / 2));

}

FVector AChoppableTree::GetImpactDirectionForLocalPoint(FVector LocalPoint)
{
	if (!CenterSpline) return FVector(-LocalPoint.X, -LocalPoint.Y, 0);

	FVector ClosestCenterPoint = GetClosestPointOnCenterSpline(LocalPoint);

	FVector ImpactDirection = ClosestCenterPoint - (GetActorLocation() + LocalPoint);

	//DrawDebugDirectionalArrow(GetWorld(), LocalPoint + GetActorLocation(), LocalPoint + GetActorLocation() + ImpactDirection, 2.0, FColor::Orange, false, 20.f);

	return ImpactDirection;
}

FVector AChoppableTree::GetClosestPointOnCenterSpline(FVector LocalPoint)
{
	if (!CenterSpline) return GetActorLocation();
	
	return CenterSpline->FindLocationClosestToWorldLocation(GetActorLocation() + LocalPoint, ESplineCoordinateSpace::World);

}

// When the tree hits the ground after splitting
void AChoppableTree::OnGroundOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit Ground"));
	
	if (AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
	UGameplayStatics::PlaySoundAtLocation(this, TreeGroundImpactSFX, SweepResult.ImpactPoint);
}

void AChoppableTree::PlayTreeFallingSFX()
{
	AudioComponent->Play();
	//UGameplayStatics::PlaySoundAtLocation(this, TreeFallingSFX, GetActorLocation());
}

