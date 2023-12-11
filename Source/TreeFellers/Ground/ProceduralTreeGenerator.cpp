// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTreeGenerator.h"
#include "GroundTile.h"
#include "ProceduralGround.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "TreeFellers/Tree/ChoppableTree.h"

// Sets default values for this component's properties
UProceduralTreeGenerator::UProceduralTreeGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	AltitudePlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltitudePlane"));
	AltitudePlane->SetupAttachment(this);
	AltitudePlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AltitudePlane->bHiddenInGame = true;
}


// Called when the game starts
void UProceduralTreeGenerator::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Component Begin Play"));
	
	//GenerateTrees();
}

#if WITH_EDITOR
void UProceduralTreeGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	TreeRadius = TreeRadius > GridCellSize / 2 ? GridCellSize / 2 : TreeRadius;

	UE_LOG(LogTemp, Warning, TEXT("Change of : %s"), *PropertyChangedEvent.GetPropertyName().ToString());
	if (PropertyChangedEvent.GetPropertyName() == FName("AltitudeThreshold"))
	{
		Ground = Ground ? Ground : Cast<AProceduralGround>(GetOwner());
		if (!Ground)
			return;

		FVector2D GroundSize = Ground->GetMeshSize();
		AltitudePlane->SetRelativeLocation(FVector(GroundSize.X / 2.f, GroundSize.Y / 2.f, AltitudeThreshold));
	}
	else if (PropertyChangedEvent.GetPropertyName() == FName("bShowAltitudePlane"))
	{
		AltitudePlane->SetVisibility(bShowAltitudePlane);
	}
	//GenerateTrees();
}
#endif

void UProceduralTreeGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	FVector2D GroundSize = Ground->GetMeshSize();
	
}

void UProceduralTreeGenerator::GenerateTrees()
{
	UE_LOG(LogTemp, Warning, TEXT("Generating Trees"));

	Ground = Ground ? Ground : Cast<AProceduralGround>(GetOwner());
	if (!Ground || TreeClasses.Num() <= 0)
		return;
	
	ClearTrees();
	CreateGrid();
	CalculateTreeCells();
	if (bShowGrid)
	{
		DisplayGrid();
	}
	SpawnTrees();
}


void UProceduralTreeGenerator::CreateGrid()
{
	if (!Ground)
		return;

	FVector Offset = Ground->GetActorLocation();

	FVector2D GroundSize = Ground->GetMeshSize();
	
	AltitudePlane->SetRelativeLocation(FVector(GroundSize.X/2.f, GroundSize.Y/2.f, AltitudeThreshold));
	AltitudePlane->SetWorldScale3D(FVector(GroundSize.X/100, GroundSize.Y/100, 1.f));
	AltitudePlane->SetVisibility(bShowAltitudePlane);

	Rows = GroundSize.Y / GridCellSize;
	Columns = GroundSize.X / GridCellSize;
	TreeMax = Rows * Columns;
	NumberOfTrees = FMath::Floor(TreeMax * TreeDensity);

	int32 IndexCount = 0;
	for (int32 r = 0; r < Rows; r++)
	{
		for (int32 c = 0; c < Columns; c++)
		{
			FVector TopLeft, BottomRight, Center;
			TopLeft = FVector(c * GridCellSize, r * GridCellSize, 0);
			Center = FVector(c * GridCellSize + GridCellSize/2, r * GridCellSize + GridCellSize/2, 0);
			BottomRight = FVector(c * GridCellSize + GridCellSize, r * GridCellSize + GridCellSize, 0);
			Grid.Add(TSharedPtr<FGroundTile>(new FGroundTile(Offset + TopLeft, Offset + Center, Offset + BottomRight)));
			//GridIndeces.Add(IndexCount);
			IndexCount++;
		}
	}
}

void UProceduralTreeGenerator::CalculateTreeCells()
{
	if (!Ground) return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

	int32 IndexCount = 0;
	for (auto& Cell : Grid)
	{
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, FVector(Cell->Center.X, Cell->Center.Y, Ground->GetActorLocation().Z + 2 * Ground->GetZMultiplier()),
			FVector(Cell->Center.X, Cell->Center.Y, Ground->GetActorLocation().Z - 2 * Ground->GetZMultiplier()), TraceObjectTypes);

		if (bHit && HitResult.ImpactPoint.Z >= AltitudeThreshold)
		{
			Cell->bCanTreeBeHere = false;
		}
		else
		{
			GridIndeces.Add(IndexCount);
		}
		IndexCount++;
	}

	int MaxPossibleTrees = FMath::Min(NumberOfTrees, GridIndeces.Num());
	for (int i = 0; i < MaxPossibleTrees; i++)
	{
		int Index = FMath::RandRange(0, GridIndeces.Num() - 1);
		Grid[GridIndeces[Index]]->bIsTreeHere = true;
		GridIndeces.RemoveAt(Index);
	}
}

void UProceduralTreeGenerator::DisplayGrid()
{
	UE_LOG(LogTemp, Warning, TEXT("Displaying Grid"));
	for (auto& Cell : Grid)
	{
		if(Cell->bCanTreeBeHere)
			DrawDebugBox(GetWorld(), Cell->Center + FVector(0.f, 0.f, 1000.f), FVector(GridCellSize/2, GridCellSize / 2, 10.f), FColor::Green, false, 5.0f, 0U, 10.0f);
		else
			DrawDebugBox(GetWorld(), Cell->Center + FVector(0.f, 0.f, 1000.f), FVector(GridCellSize / 2, GridCellSize / 2, 10.f), FColor::Orange, false, 5.0f, 0U, 10.0f);

		//DrawDebugSphere(GetWorld(), Cell->TopLeft + FVector(0.f, 0.f, 1000.f), 100.f, 16, FColor::Red, true);
	}
	//DrawDebugSphere(GetWorld(), Grid[0]->TopLeft + 1000.f, 10.f, 16, FColor::Red, true);
	
}

void UProceduralTreeGenerator::SpawnTrees()
{
	if (!Ground || TreeClasses.Num() == 0)
		return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

	int Count = 1;
	for (auto& Cell : Grid)
	{
		if (!Cell->bIsTreeHere)
			continue;

		FVector2D Position2D = GetRandomValidTreePositionInCell(Cell);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, FVector(Position2D.X, Position2D.Y, Ground->GetActorLocation().Z + 2 * Ground->GetZMultiplier()),
			FVector(Position2D.X, Position2D.Y, Ground->GetActorLocation().Z - 2 * Ground->GetZMultiplier()), TraceObjectTypes);

		if (bHit)
		{
			//DrawDebugSphere(GetWorld(), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z), TreeRadius, 16, FColor::Green, true);
			//DrawDebugLine(GetWorld(), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z) + HitResult.ImpactNormal * 500.f, FColor::Magenta, true);

			auto TreeClass = TreeClasses[FMath::RandRange(0, TreeClasses.Num() - 1)];
			if (!TreeClass)
				return;

			FVector SpawnLocation(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z);
			SpawnLocation += (-HitResult.ImpactNormal) * TreeInsertionDepth;

			// Rotating so that the tree is perpendicular to the ground.
			FRotator RotFromYZ = UKismetMathLibrary::MakeRotFromYZ(FVector(0.f, 1.f, 0.f), HitResult.ImpactNormal);
			FRotator RotFromXZ = UKismetMathLibrary::MakeRotFromXZ(FVector(1.f, 0.f, 0.f), HitResult.ImpactNormal);

			FRotator SpawnRotation = FRotator(RotFromYZ.Pitch, 0.f, RotFromXZ.Roll);
			if (bRandomYaw)
			{
				SpawnRotation.Add(0.f, FMath::RandRange(0.f, 360.f), 0.f);
			}

			float ScaleRange = 1.f * ScaleRandomness;
			FVector SpawnScale = FVector(1.f, 1.f, 1.f) * FMath::RandRange(1 - ScaleRange, 1 + ScaleRange);

			FTransform SpawnTransform(SpawnRotation, SpawnLocation, SpawnScale);

			FActorSpawnParameters SpawnParameters;

			AActor* Tree = GetWorld()->SpawnActor<AActor>(TreeClass, SpawnTransform, SpawnParameters);
			SpawnedTrees.Add(Tree);

			#if WITH_EDITOR
				Tree->SetFolderPath("/Trees");
				Tree->SetActorLabel("ChoppableTree" + FString::FromInt(Count));
				Count++;
			#endif
		}
	}
}

void UProceduralTreeGenerator::ClearTrees()
{
	for (auto Tree : SpawnedTrees)
	{
		Tree->Destroy();
	}
	
	SpawnedTrees.Empty();
	Grid.Empty();
	GridIndeces.Empty();
}

FVector2D UProceduralTreeGenerator::GetRandomValidTreePositionInCell(TSharedPtr<FGroundTile> Cell)
{
	float XPosition = FMath::RandRange(Cell->TopLeft.X + TreeRadius, Cell->BottomRight.X - TreeRadius);
	float YPosition = FMath::RandRange(Cell->TopLeft.Y + TreeRadius, Cell->BottomRight.Y - TreeRadius);

	return FVector2D(XPosition, YPosition);
}

float UProceduralTreeGenerator::GetAngleBetweenVectors(FVector A, FVector B)
{
	A.Normalize();
	B.Normalize();

	float DotProd = FVector::DotProduct(A, B);

	return FMath::RadiansToDegrees(FMath::Acos(DotProd));

}



