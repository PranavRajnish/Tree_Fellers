// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "Components/SphereComponent.h"
#include "TreeFellers/Player/PlayerCharacter.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"

ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetupAttachment(RootComponent);
	ObjectMesh->SetupAttachment(Hinge);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractionSphere->SetupAttachment(RootComponent);

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnInteractionOverlapBegin);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnInteractionOverlapEnd);

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

}

void ADoor::BeginPlay()
{
	Super::BeginPlay();

	if (!DoorCurve) return;

	FOnTimelineFloat InterpFunction;
	FOnTimelineEvent TimelineFinished;

	InterpFunction.BindUFunction(this, FName("DoorTimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnDoorTimelineFinished"));

	DoorTimeline->AddInterpFloat(DoorCurve, InterpFunction, FName("Alpha"));
	DoorTimeline->SetTimelineFinishedFunc(TimelineFinished);

	DoorTimeline->SetLooping(false);
	DoorTimeline->SetIgnoreTimeDilation(true);
}


void ADoor::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetInteractionObject(this);
	}
}

void ADoor::OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetInteractionObject(nullptr);
	}
}


void ADoor::DoorTimelineFloatReturn(float value)
{
	Hinge->SetRelativeRotation(FMath::Lerp(ClosedRotation, OpenRotation, value));
}

void ADoor::OnDoorTimelineFinished()
{
	bIsInUse = false;
	bIsOpen = !bIsOpen;
}

void ADoor::Interact()
{
	if (bIsInUse) return;

	bIsInUse = true;

	if (!bIsOpen)
	{
		DoorTimeline->PlayFromStart();
	}
	else
	{
		DoorTimeline->ReverseFromEnd();
	}
}
