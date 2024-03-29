// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildable.h"
#include "Components/BoxComponent.h"
#include "SnapCollider.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Sound/SoundCue.h"

ABuildable::ABuildable()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
	ObjectMesh->SetupAttachment(RootComponent);


	/*GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DestructableMesh"));
	GeometryCollectionComponent->SetupAttachment(RootComponent);*/

	UE_LOG(LogTemp, Warning, TEXT("In Constructor of Buildable"));
}

void ABuildable::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABuildable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<USnapCollider*> ABuildable::GetSnapColliders(FName TagName)
{
	TArray<USnapCollider*> SnapColliders;
	TArray<UActorComponent*> ActorComponents = GetComponentsByTag(USnapCollider::StaticClass(), TagName);
	for (UActorComponent* c : ActorComponents)
	{
		USnapCollider* b = Cast<USnapCollider>(c);
		if (b)
			SnapColliders.Add(b);
	}

	UE_LOG(LogTemp, Warning, TEXT("Number of snap colliders: %d"), SnapColliders.Num());
	return SnapColliders;
}

void ABuildable::SetObjectMesh(UStaticMesh* NewMesh)
{
	if (!ObjectMesh) return;

	ObjectMesh->SetStaticMesh(NewMesh);
}

void ABuildable::Impacted(FVector ImpactPosition)
{
	if (!ObjectMesh) return;
	UE_LOG(LogTemp, Warning, TEXT("Buildable Hit"));

	/*ObjectMesh->SetVisibility(false);
	ObjectMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/

	DestroyMesh(ImpactPosition);

	/*GeometryCollectionComponent = NewObject<UGeometryCollectionComponent>(this, UGeometryCollectionComponent::StaticClass(), TEXT("GeometryCollection"));
	if (GeometryCollectionComponent)
	{
		GeometryCollectionComponent->CreationMethod = EComponentCreationMethod::Instance;

		GeometryCollectionComponent->SetupAttachment(GetRootComponent());
		GeometryCollectionComponent->RegisterComponent();
	}*/
}

void ABuildable::DestroyMesh_Implementation(FVector ImpactPosition)
{
	UE_LOG(LogTemp, Warning, TEXT("Blueprint event not implemented!"));
}
