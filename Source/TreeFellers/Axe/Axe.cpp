// Fill out your copyright notice in the Description page of Project Settings.


#include "Axe.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TreeFellers/Tree/ChoppableTree.h"
#include "TreeFellers/Player/PlayerCharacter.h"
#include "Sound/SoundCue.h"
#include "TreeFellers/Buildings/Buildable.h"

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
		//UE_LOG(LogTemp, Warning, TEXT("Axe point : %s"), *CollisionLocation.ToString());
		TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel4));
		FHitResult HitResult;

		bHasHitThisSwing = UKismetSystemLibrary::SphereTraceSingleForObjects(this, CollisionLocation, CollisionLocation, CollisionRadius,
			TraceObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None , HitResult, true);

		if (bHasHitThisSwing)
		{
			AChoppableTree* Tree = Cast<AChoppableTree>(HitResult.GetActor());
			if (Tree)
			{
				Tree->AxeImpact(CollisionLocation, HitResult.ImpactNormal, this);
			}

			ABuildable* Buildable = Cast<ABuildable>(HitResult.GetActor());
			if (Buildable)
			{
				UE_LOG(LogTemp, Warning, TEXT("Axe point : %s"), *CollisionLocation.ToString());
				Buildable->Impacted(CollisionLocation);

			}

			Player = Player? Player : Cast<APlayerCharacter>(GetOwner());
			if (Player)
			{
				Player->AxeImpact();
			}

		}	
	}
	
}

void AAxe::PlaySwingSound()
{
	if (!WeaponSwingSFX) return;

	UGameplayStatics::PlaySoundAtLocation(this, WeaponSwingSFX, GetActorLocation());
}


