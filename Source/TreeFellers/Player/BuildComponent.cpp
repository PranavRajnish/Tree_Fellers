// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildComponent.h"
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TreeFellers/Buildings/Buildable.h"
#include "Components/BoxComponent.h"

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
	if (!CameraComponent || !PlayerCharacter || !CurrentObjectComponent) return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

	//FHitResult HitResult;
	TArray<FHitResult> HitResults;
	FVector StartVector = PlayerCharacter->GetActorLocation() + FVector(0.f, 0.f, TraceZOffset) + (PlayerCharacter->GetActorForwardVector() * 50.f);
	FVector EndVector = StartVector + (CameraComponent->GetForwardVector() * PreviewTraceDistance);
	FVector NewVector = (EndVector - StartVector).RotateAngleAxis(TraceAngle, CameraComponent->GetRightVector());

	//DrawDebugLine(GetWorld(), StartVector, StartVector + NewVector, FColor::Orange, false, -1.f, (uint8)0U, 5.f);
	
	//bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartVector, StartVector + NewVector, ECollisionChannel::ECC_GameTraceChannel3);
	bool bHit = GetWorld()->LineTraceMultiByChannel(HitResults, StartVector, StartVector + NewVector, ECollisionChannel::ECC_GameTraceChannel3);

	static const FString ContextString(TEXT("Build Objects Context"));
	FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

	// First checking if there are any overlapping hits with a snap collider.
	bool bCanSnap = false;
	if (BuildObject)
	{
		// Looping through all hits to see if the corresponding snap collider has been hit.
		for (auto& HitResult : HitResults)
		{
			ABuildable* Buildable = Cast<ABuildable>(HitResult.GetActor());
			if (Buildable && HitResult.GetComponent()->ComponentHasTag(BuildObject->TagName))
			{
				bool isSnapCollider = DetectSnapColliders(Buildable, HitResult.GetComponent());
				if (isSnapCollider)
				{
					bCanSnap = true;
					ObjectTransform = HitResult.GetComponent()->GetComponentTransform();
					break;
				}
			}
		}
	}

	// If no snapping, checkin to see if there is a blocking hit.
	if (!bCanSnap)
	{
		if (bHit)
		{
			ObjectTransform.SetLocation(HitResults[HitResults.Num() - 1].ImpactPoint);
		}
		else
		{
			ObjectTransform.SetLocation(StartVector + NewVector);
		}
	}
	
	// Changing of preview mesh color based on if change in blocking hit.
	if (bCanBuildHere != bHit)
	{
		bCanBuildHere = bHit;
		SetObjectMaterial(bCanBuildHere);
	}

	CurrentObjectComponent->SetWorldTransform(ObjectTransform);

}

// Check if the hit component is the a snap collider of the same type of object.
bool UBuildComponent::DetectSnapColliders(ABuildable* Buildable, UPrimitiveComponent* HitComponent)
{
	if (!Buildable || !BuildObjects) return false;

	//UE_LOG(LogTemp, Warning, TEXT("Hit Snap Collider"));
	UBoxComponent* HitCollider = Cast<UBoxComponent>(HitComponent);
	if (!HitCollider) return false;

	static const FString ContextString(TEXT("Build Objects Context"));
	FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

	if (BuildObject)
	{
		CurrentObjectComponent->SetStaticMesh(BuildObject->ObjectMesh);

		TArray<UBoxComponent*> SnapColliders = Buildable->GetSnapColliders(BuildObject->TagName);

		for (UBoxComponent* SnapCollider : SnapColliders)
		{
			if (SnapCollider == HitCollider)
			{
				return true;
			}
		}
	}

	

	return false;
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

		CurrentObjectComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
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
	if (!BuildObjects || !CurrentObjectComponent) return;

	static const FString ContextString(TEXT("Build Objects Context"));
	FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

	if (BuildObject && BuildObject->ObjectMesh)
	{
		CurrentObjectComponent->SetStaticMesh(BuildObject->ObjectMesh);
	}
}

void UBuildComponent::SpawnBuildActor()
{
	if (!BuildObjects) return;

	static const FString ContextString(TEXT("Build Objects Context"));
	FBuildObject* BuildObject = BuildObjects->FindRow<FBuildObject>(BuildObjectRowNames[BuildObjectIndex], ContextString, true);

	if (BuildObject && BuildObject->ObjectActorClass)
	{
		ABuildable* NewBuildable = GetWorld()->SpawnActor<ABuildable>(BuildObject->ObjectActorClass, ObjectTransform);
		NewBuildable->SetObjectMesh(BuildObject->ObjectMesh);
	} 
}

