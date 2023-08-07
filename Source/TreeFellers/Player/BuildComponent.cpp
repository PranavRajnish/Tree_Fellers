// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildComponent.h"
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMeshActor.h"


UBuildComponent::UBuildComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UBuildComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());

	if (BuildObjects)
	{
		BuildObjectRowNames = BuildObjects->GetRowNames();
	}
}

void UBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsInBuildMode)
	{
		if (!CurrentObjectComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawning preview..."));
			SpawnObjectPreview();

			bCanBuildHere = true;
			SetObjectMaterial(bCanBuildHere);
			PreviewObjectLocation();
		}

		if (!GetWorld()->GetTimerManager().IsTimerActive(TraceTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(TraceTimerHandle, this, &ThisClass::PreviewObjectLocation, TraceBuffer, true);
		}
	}
	else
	{
		if (CurrentObjectComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("Destroying preview..."));
			CurrentObjectComponent->UnregisterComponent();
			CurrentObjectComponent->DestroyComponent();
			CurrentObjectComponent = nullptr;
		}

		if (GetWorld()->GetTimerManager().IsTimerActive(TraceTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
		}
	}
	

}

void UBuildComponent::NextBuildObject()
{
	BuildObjectIndex = (BuildObjectIndex + 1) % BuildObjectRowNames.Num();
	ChangeMesh();
}

void UBuildComponent::PreviousBuildObject()
{
	BuildObjectIndex--;
	if (BuildObjectIndex < 0)
		BuildObjectIndex = BuildObjectRowNames.Num() - 1;

	ChangeMesh();
}

void UBuildComponent::PlaceBuilding()
{
	if (bIsInBuildMode && bCanBuildHere)
	{
		SpawnBuildActor();
	}
}

void UBuildComponent::PreviewObjectLocation()
{
	if (!CameraComponent || !PlayerCharacter) return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

	FHitResult HitResult;
	FVector StartVector = PlayerCharacter->GetActorLocation();
	FVector EndVector = PlayerCharacter->GetActorLocation() + (CameraComponent->GetForwardVector() * PreviewTraceDistance);
	FVector NewVector = (EndVector - StartVector).RotateAngleAxis(TraceAngle, CameraComponent->GetRightVector());

	//DrawDebugLine(GetWorld(), StartVector, StartVector + NewVector, FColor::Orange, false, -1.f, (uint8)0U, 5.f);
	
	bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartVector, StartVector + NewVector, TraceObjectTypes);
	
	if (bCanBuildHere != bHit)
	{
		bCanBuildHere = bHit;
		SetObjectMaterial(bCanBuildHere);
	}

	ObjectTransform.SetLocation(bHit ? HitResult.ImpactPoint : HitResult.TraceEnd);
	CurrentObjectComponent->SetWorldTransform(ObjectTransform);

}

void UBuildComponent::SpawnObjectPreview()
{
	if (!PlayerCharacter || !CurrentObject) return;

	CurrentObjectComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("Static Mesh"));
	if (CurrentObjectComponent)
	{
		CurrentObjectComponent->CreationMethod = EComponentCreationMethod::Instance;

		CurrentObjectComponent->SetupAttachment(this);
		CurrentObjectComponent->RegisterComponent();

		//CurrentObjectComponent->AttachToComponent(GetRootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		
		if (BuildObjects)
		{
			static const FString ContextString(TEXT("Build Objects Context"));
			FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

			if (BuildObject)
			{
				CurrentObjectComponent->SetStaticMesh(BuildObject->ObjectMesh);
			}
		}
	}
}

void UBuildComponent::SetObjectMaterial(bool bIsGreen)
{
	if (!CurrentObjectComponent) return;

	int32 NumMaterials = CurrentObjectComponent->GetNumMaterials();

	for (int32 i = 0; i < NumMaterials; i++)
	{
		CurrentObjectComponent->SetMaterial(i, bIsGreen? GreenPreview: RedPreview);
	}
	
}

void UBuildComponent::ChangeMesh()
{
	if (!BuildObjects) return;

	static const FString ContextString(TEXT("Build Objects Context"));
	FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

	if (BuildObject)
	{
		CurrentObjectComponent->SetStaticMesh(BuildObject->ObjectMesh);
	}
}

void UBuildComponent::SpawnBuildActor()
{
	if (!BuildObjects) return;

	static const FString ContextString(TEXT("Build Objects Context"));
	FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

	if (BuildObject)
	{
		GetWorld()->SpawnActor<AStaticMeshActor>(BuildObject->ObjectActorClass, ObjectTransform);
	} 
}

