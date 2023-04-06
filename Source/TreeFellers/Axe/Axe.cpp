// Fill out your copyright notice in the Description page of Project Settings.


#include "Axe.h"
#include "Components/StaticMeshComponent.h"

AAxe::AAxe()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Axe Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

}

void AAxe::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

