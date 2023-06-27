#include "ProceduralGround.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
AProceduralGround::AProceduralGround()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	ProceduralMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AProceduralGround::BeginPlay()
{
	Super::BeginPlay();
	

}

void AProceduralGround::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//DrawDebugBox(GetWorld(), Transform.GetLocation() + FVector(XSize * Scale / 2, YSize * Scale / 2, 0), FVector(XSize * Scale / 2, YSize * Scale / 2, 100.0f), FColor::Blue, true, -1.0f, 0U, 10.f);
	UE_LOG(LogTemp, Log, TEXT("Constructing Ground.."));
	ProceduralMesh->ClearMeshSection(0);
	Vertices.Empty();
	Triangles.Empty();
	UV0.Empty();

	CreateVertices();
	CreateTriangles();

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	ProceduralMesh->SetMaterial(0, GroundMaterial);
}

// Called every frame
void AProceduralGround::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralGround::CreateVertices()
{
	for (int32 X = 0; X <= XSize; X++)
	{
		for (int32 Y = 0; Y <= YSize; Y++)
		{
			float Z = FMath::PerlinNoise2D(FVector2D(X * NoiseScale + 0.1, Y * NoiseScale + 0.1)) * ZMultiplier;
			Vertices.Add(FVector(X * Scale, Y  * Scale, Z));
			UV0.Add(FVector2D(X * UVScale, Y * UVScale));

			//DrawDebugSphere(GetWorld(), FVector(X * Scale, Y * Scale, 0), 25.0f, 16, FColor::Red, true, -1.0f);
		}
	}
}

void AProceduralGround::CreateTriangles()
{
	int Vertex = 0;

	for (int X = 0; X < XSize; X++)
	{
		for (int Y = 0; Y < YSize; Y++)
		{
			Triangles.Add(Vertex);
			Triangles.Add(Vertex + 1);
			Triangles.Add(Vertex + YSize + 1);
			Triangles.Add(Vertex + 1);
			Triangles.Add(Vertex + YSize + 2);
			Triangles.Add(Vertex + YSize + 1);

			Vertex++;
		}
		Vertex++;
	}
}

