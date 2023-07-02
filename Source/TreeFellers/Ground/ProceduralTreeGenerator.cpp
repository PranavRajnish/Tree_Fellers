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

	GenerateTrees();
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
	if (!Ground)
		return;

	ClearTrees();
	CreateGrid();
	CalculateTreeCells();
	DisplayGrid();
	SpawnTrees();
}


void UProceduralTreeGenerator::CreateGrid()
{
	if (!Ground)
		return;

	FVector Offset = Ground->GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("Ground Location: %s"), *Offset.ToString());

	FVector2D GroundSize = Ground->GetMeshSize();
	UE_LOG(LogTemp, Warning, TEXT("Mesh Size: %s"), *GroundSize.ToString());
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
			GridIndeces.Add(IndexCount);
			IndexCount++;
		}
	}
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
	if (!Ground)
		return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

	int Count = 1;
	for (auto& Cell : Grid)
	{
		if (!Cell->bCanTreeBeHere)
			continue;

		FVector2D Position2D = GetRandomValidTreePositionInCell(Cell);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, FVector(Position2D.X, Position2D.Y, Ground->GetActorLocation().Z + 2 * Ground->GetZMultiplier()),
			FVector(Position2D.X, Position2D.Y, Ground->GetActorLocation().Z - 2 * Ground->GetZMultiplier()), TraceObjectTypes);

		if (bHit && TreeClasses.Num() > 0)
		{
			//DrawDebugSphere(GetWorld(), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z), TreeRadius, 16, FColor::Green, true);
			//DrawDebugLine(GetWorld(), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z), FVector(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z) + HitResult.ImpactNormal * 500.f, FColor::Magenta, true);

			auto TreeClass = TreeClasses[FMath::RandRange(0, TreeClasses.Num() - 1)];

			FVector SpawnLocation(Position2D.X, Position2D.Y, HitResult.ImpactPoint.Z);

			FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromYZ(FVector(0.f, 1.f, 0.f), HitResult.ImpactNormal);

			FActorSpawnParameters SpawnParameters;

			AActor* Tree = GetWorld()->SpawnActor<AActor>(TreeClass, SpawnLocation, SpawnRotation, SpawnParameters);
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




