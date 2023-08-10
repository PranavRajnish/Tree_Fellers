// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "BuildComponent.generated.h"

USTRUCT(BlueprintType)
struct FBuildObject : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* ObjectMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ObjectActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TagName;
};


class APlayerCharacter;
class UCameraComponent;
class ABuildable;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TREEFELLERS_API UBuildComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuildComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void NextBuildObject();
	void PreviousBuildObject();
	void PlaceBuilding();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	APlayerCharacter* PlayerCharacter;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CurrentObjectComponent;

	UPROPERTY(EditAnywhere)
	UStaticMesh* CurrentObject;

	// Parameters
	UPROPERTY(EditAnywhere, Category = "Parameters")
	float PreviewTraceDistance = 100.f;
	UPROPERTY(EditAnywhere, Category = "Parameters")
	float TraceAngle = -10.f;
	UPROPERTY(EditAnywhere, Category = "Parameters")
	float TraceZOffset = 100.f;
	UPROPERTY(EditAnywhere, Category = "Parameters")
	float TraceBuffer = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Parameters")
	UDataTable* BuildObjects;

	// Materials
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* GreenPreview;
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* RedPreview;

	void SetObjectMaterial(bool bIsGreen);
	bool DetectSnapColliders(ABuildable* Buildable, UPrimitiveComponent* HitComponent);

private:
	FTransform ObjectTransform;
	bool bIsInBuildMode = false;
	bool bCanBuildHere = false;

	FTimerHandle TraceTimerHandle;
	TArray<FName> BuildObjectRowNames;
	int32 BuildObjectIndex = 0;

	void SpawnObjectPreview();
	void PreviewObjectLocation();
	void ChangeMesh();
	void SpawnBuildActor();

public:
	FORCEINLINE void SetCameraComponent(UCameraComponent* NewCameraComponent) { CameraComponent = NewCameraComponent; };
	FORCEINLINE void SetBuildModeOn(bool bBuildMode) { bIsInBuildMode = bBuildMode; }
		
};
