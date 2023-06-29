// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTreeGenerator.h"
#include "GroundTile.h"
#include "ProceduralGround.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UProceduralTreeGenerator::UProceduralTreeGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UProceduralTreeGenerator::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Component Begin Play"));

	Ground = Cast<AProceduralGround>(GetOwner());
	if (Ground)
	{
		FVector2D GroundSize = Ground->GetMeshSize();
		UE_LOG(LogTemp, Warning, TEXT("Mesh Size: %s"), *GroundSize.ToString());
		Rows = GroundSize.Y / GridCellSize;
		Columns = GroundSize.X / GridCellSize;
		TreeMax = Rows * Columns;
		NumberOfTrees = FMath::Floor(TreeMax * TreeDensity);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot get owner"));
	}
	
	CreateGrid();
	CalculateTreeCells();
	DisplayGrid();
	SpawnTrees();
}

#if WITH_EDITOR
void UProceduralTreeGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	TreeRadius = TreeRadius > GridCellSize / 2 ? GridCellSize / 2 : TreeRadius;
}
#endif

void UProceduralTreeGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	FVector2D GroundSize = Ground->GetMeshSize();
	
}


void UProceduralTreeGenerator::CreateGrid()
{
	if (!Ground)
		return;
	
	FVector Offset = Ground->GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("Ground Location: %s"), *Offset.ToString());

	int32 IndexCount = 0;
	for (int32 r = 0; r < Rows; r++)
	{
		for (int32 c = 0; c < Columns; c++)
		{
			FVector TopLeft, BottomRight, Center;
			TopLeft = FVector(r * GridCellSize, c * GridCellSize, 0);
			Center = FVector(r * GridCellSize + GridCellSize/2, c * GridCellSize + GridCellSize/2, 0);
			BottomRight = FVector(r * GridCellSize + GridCellSize, c * GridCellSize + GridCellSize, 0);
			Grid.Add(TSharedPtr<FGroundTile>(new FGroundTile(Offset + TopLeft, Offset + Center, Offset + BottomRight)));
			GridIndeces.Add(IndexCount);
			IndexCount++;
		}
	}
}

void UProceduralTreeGenerator::DisplayGrid()
{
	UE_LOG(LogTemp, Warning, TEXT("Displaying Grid"));
	for (auto& Cell : Grid)
	{
		if(Cell->bCanTreeBeHere)
			DrawDebugBox(GetWorld(), Cell->Center + FVector(0.f, 0.f, 1000.f), FVector(GridCellSize/2, GridCellSize / 2, 10.f), FColor::Green, true, -1.0f, 0U, 10.0f);
		else
			DrawDebugBox(GetWorld(), Cell->Center + FVector(0.f, 0.f, 1000.f), FVector(GridCellSize / 2, GridCellSize / 2, 10.f), FColor::Orange, true, -1.0f, 0U, 10.0f);

		//DrawDebugSphere(GetWorld(), Cell->TopLeft + FVector(0.f, 0.f, 1000.f), 100.f, 16, FColor::Red, true);
	}
	//DrawDebugSphere(GetWorld(), Grid[0]->TopLeft + 1000.f, 10.f, 16, FColor::Red, true);
	
}

void UProceduralTreeGenerator::CalculateTreeCells()
{
	for (int i = 0; i < NumberOfTrees; i++)
	{
		int Index = FMath::RandRange(0, GridIndeces.Num() - 1);
		Grid[GridIndeces[Index]]->bCanTreeBeHere = true;
		GridIndeces.RemoveAt(Index);
	}
}

void UProceduralTreeGenerator::SpawnTrees()
{
	if (!Ground)
		return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

	for (auto& Cell : Grid)
	{
		if (!Cell->bCanTreeBeHere)
			continue;

		FVector2D Position2D = GetRandomValidTreePositionInCell(Cell);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, FVector(Position2D.X, Position2D.Y, Ground->GetActorLocation().Z + 2 * Ground->GetZMultiplier()),
			FVector(Position2D.X, Position2D.Y, Ground->GetActorLocation().Z - 2 * Ground->GetZMultiplier()), TraceObjectTypes);

		if (bHit)
		{
			DrawDebugSphere(GetWorld(), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z), TreeRadius, 16, FColor::Green, true);
		}
	}
}

FVector2D UProceduralTreeGenerator::GetRandomValidTreePositionInCell(TSharedPtr<FGroundTile> Cell)
{
	float XPosition = FMath::RandRange(Cell->TopLeft.X + TreeRadius, Cell->BottomRight.X - TreeRadius);
	float YPosition = FMath::RandRange(Cell->TopLeft.Y + TreeRadius, Cell->BottomRight.Y - TreeRadius);

	return FVector2D(XPosition, YPosition);
}




