// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildable.h"
#include "Components/BoxComponent.h"

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

//#if WITH_EDITOR
//
//void ABuildable::PostInitProperties()
//{
//	Super::PostInitProperties();
//
//	SnapColliders.Empty();
//	for (int32 i = 0; i < NumberOfSnapColliders; i++)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Collider%d"), i);
//		
//		UBoxComponent* BoxComponent = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("SnapCollider"));
//		if (BoxComponent)
//		{
//			BoxComponent->CreationMethod = EComponentCreationMethod::Instance;
//
//			BoxComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
//			BoxComponent->RegisterComponent();
//
//			SnapColliders.Add(BoxComponent);
//		}
//	}
//
//}
//
//void ABuildable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	for (int32 i = 0; i < SnapColliders.Num(); i++)
//	{
//		SnapColliders[i]->UnregisterComponent();
//	}
//	SnapColliders.Empty();
//
//	for (int32 i = 0; i < NumberOfSnapColliders; i++)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Collider%d"), i);
//
//		UBoxComponent* BoxComponent = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("SnapCollider"));
//		if (BoxComponent)
//		{
//			BoxComponent->CreationMethod = EComponentCreationMethod::Instance;
//
//			BoxComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
//			BoxComponent->RegisterComponent();
//
//			SnapColliders.Add(BoxComponent);
//		}
//	}
//
//}
//
//#endif

TArray<UBoxComponent*> ABuildable::GetSnapColliders(FName TagName)
{
	TArray<UBoxComponent*> BoxComponents;
	TArray<UActorComponent*> ActorComponents = GetComponentsByTag(UBoxComponent::StaticClass(), TagName);
	for (UActorComponent* c : ActorComponents)
	{
		UBoxComponent* b = Cast<UBoxComponent>(c);
		if (b)
			BoxComponents.Add(b);
	}

	UE_LOG(LogTemp, Warning, TEXT("Number of snap colliders: %d"), BoxComponents.Num());
	return BoxComponents;
}

void ABuildable::SetObjectMesh(UStaticMesh* NewMesh)
{
	if (!ObjectMesh) return;

	ObjectMesh->SetStaticMesh(NewMesh);
}
