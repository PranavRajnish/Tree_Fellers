// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildable.h"
#include "Components/BoxComponent.h"
#include "SnapCollider.h"

ABuildable::ABuildable()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
	ObjectMesh->SetupAttachment(RootComponent);

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
