// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralTreeGenerator.generated.h"

class AProceduralGround;
struct FGroundTile;
class AChoppableTree;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TREEFELLERS_API UProceduralTreeGenerator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProceduralTreeGenerator();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	AProceduralGround* Ground = nullptr;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1.0f))
	float GridCellSize = 2000.f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float TreeDensity = 1.0f;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> TreeClasses;
	UPROPERTY(EditAnywhere)
	float TreeRadius = 500.f;

	UFUNCTION(CallInEditor)
	void GenerateTrees();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY(VisibleAnywhere)
	int32 Rows;
	UPROPERTY(VisibleAnywhere)
	int32 Columns;
	TArray<TSharedPtr<FGroundTile>> Grid;
	TArray<int32> GridIndeces;

	void CreateGrid();
	void DisplayGrid();
	void CalculateTreeCells();
	void SpawnTrees();
	void ClearTrees();
	FVector2D GetRandomValidTreePositionInCell(TSharedPtr<FGroundTile> Cell);

	UPROPERTY(VisibleAnywhere)
	int32 TreeMax;
	UPROPERTY(VisibleAnywhere)
	int32 NumberOfTrees;

	UPROPERTY()
	TArray<AActor*> SpawnedTrees;

public:
	FORCEINLINE void SetGround(AProceduralGround* NewGround) { UE_LOG(LogTemp, Warning, TEXT("Ground Set")); Ground = NewGround; }
	
};
