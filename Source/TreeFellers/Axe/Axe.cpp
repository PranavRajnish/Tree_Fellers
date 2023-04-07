// Fill out your copyright notice in the Description page of Project Settings.


#include "Axe.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AAxe::AAxe()
{
	PrimaryActorTick.bCanEverTick = true;

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
	//FVector SocketLocation = Mesh->GetSocketLocation(FName("CollisionSocket"));
	FVector CollisionLocation = CollisionPoint->GetComponentLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));
	FHitResult HitResult;

	bool bHasHit = UKismetSystemLibrary::SphereTraceSingleForObjects(this, CollisionLocation, CollisionLocation, CollisionRadius,
				   TraceObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, HitResult, true);
}
