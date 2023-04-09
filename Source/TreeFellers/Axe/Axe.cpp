// Fill out your copyright notice in the Description page of Project Settings.


#include "Axe.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TreeFellers/Tree/ChoppableTree.h"

AAxe::AAxe()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Axe Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Collision Point"));
	CollisionPoint->SetupAttachment(Mesh);

}

void AAxe::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AAxe::CalculateAxeCollision()
{
	if (!bHasHitThisSwing && HasAuthority())
	{
		//FVector SocketLocation = Mesh->GetSocketLocation(FName("CollisionSocket"));
		FVector CollisionLocation = CollisionPoint->GetComponentLocation();

		TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));
		FHitResult HitResult;

		bHasHitThisSwing = UKismetSystemLibrary::SphereTraceSingleForObjects(this, CollisionLocation, CollisionLocation, CollisionRadius,
			TraceObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, HitResult, true);

		if (bHasHitThisSwing)
		{
			AChoppableTree* Tree = Cast<AChoppableTree>(HitResult.GetActor());
			if (Tree)
			{
				Tree->AxeImpact(HitResult);
			}
		}	
	}
	
}
