// Fill out your copyright notice in the Description page of Project Settings.


#include "Hammer.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TreeFellers/Buildings/Buildable.h"
#include "TreeFellers/Player/PlayerCharacter.h"

// Sets default values
AHammer::AHammer()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Axe Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Collision Point"));
	CollisionPoint->SetupAttachment(Mesh);

}

// Called when the game starts or when spawned
void AHammer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHammer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHammer::CalculateCollision()
{

	if (!bHasHitThisSwing && HasAuthority())
	{
		//FVector SocketLocation = Mesh->GetSocketLocation(FName("CollisionSocket"));
		FVector CollisionLocation = CollisionPoint->GetComponentLocation();
		//UE_LOG(LogTemp, Warning, TEXT("Axe point : %s"), *CollisionLocation.ToString());
		TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel4));
		FHitResult HitResult;

		bHasHitThisSwing = UKismetSystemLibrary::SphereTraceSingleForObjects(this, CollisionLocation, CollisionLocation, CollisionRadius,
			TraceObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, HitResult, true);

		if (bHasHitThisSwing)
		{
			ABuildable* Buildable = Cast<ABuildable>(HitResult.GetActor());
			if (Buildable)
			{
				UE_LOG(LogTemp, Warning, TEXT("Axe point : %s"), *CollisionLocation.ToString());
				Buildable->Impacted(CollisionLocation);
				
			}

			Player = Player ? Player : Cast<APlayerCharacter>(GetOwner());
			if (Player)
			{
				Player->AxeImpact();
			}

		}
	}
}

void AHammer::PlaySwingSound()
{
	if (!WeaponSwingSFX) return;

	UGameplayStatics::PlaySoundAtLocation(this, WeaponSwingSFX, GetActorLocation());
}

