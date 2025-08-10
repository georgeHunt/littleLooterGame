#include "ProcMapManager.h"
#include "MapGenerator.h"
#include "NavigationSystem.h"

AProcMapManager::AProcMapManager()
{
	PrimaryActorTick.bCanEverTick = false;

	FloorHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("FloorHISM"));
	WallHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("WallHISM"));
	FloorHISM->SetupAttachment(RootComponent);
	WallHISM->SetupAttachment(RootComponent);
}

void AProcMapManager::BeginPlay()
{
	Super::BeginPlay();
	if (Generator == nullptr)
	{
		Generator = NewObject<UMapGenerator>(this);
	}
}

void AProcMapManager::EnsureComponents()
{
	if (!Tileset) return;
	FloorHISM->SetStaticMesh(Tileset->FloorMesh);
	WallHISM->SetStaticMesh(Tileset->WallMesh);

	// Basic collision expectations
	if (Tileset->FloorMesh)
	{
		FloorHISM->SetCollisionProfileName(TEXT("BlockAll"));
		FloorHISM->SetCanEverAffectNavigation(true);
	}
	if (Tileset->WallMesh)
	{
		WallHISM->SetCollisionProfileName(TEXT("BlockAll"));
		WallHISM->SetCanEverAffectNavigation(true);
	}
}

void AProcMapManager::Clear()
{
	if (FloorHISM) FloorHISM->ClearInstances();
	if (WallHISM)  WallHISM->ClearInstances();
	Map = FMapData();
}

void AProcMapManager::Generate()
{
	if (!Tileset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcMapManager: Tileset not set."));
		return;
	}
	if (Generator == nullptr)
	{
		Generator = NewObject<UMapGenerator>(this);
	}

	Clear();
	EnsureComponents();

	Map = Generator->Run(Params, Seed);

	// Spawn floor/walls
	for (const auto& KV : Map.Cells)
	{
		const FCell& C = KV.Value;
		const FVector Pos = GridToWorld(C.X, C.Y);
		FTransform T(FRotator::ZeroRotator, Pos, FVector(1.f));
		if (C.Type == ECellType::Floor && Tileset->FloorMesh)
		{
			FloorHISM->AddInstance(T);
		}
		else if (C.Type == ECellType::Wall && Tileset->WallMesh)
		{
			// Raise wall mesh by half height if its pivot is at base
			T.AddToTranslation(FVector(0, 0, Tileset->WallHeight * 0.5f));
			WallHISM->AddInstance(T);
		}
	}

	// Rebuild navmesh for AI
	if (UWorld* World = GetWorld())
	{
		if (auto* NS = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			NS->Build();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ProcGen complete. Seed=%d, Cells=%d, Rooms=%d"), Seed, Map.Cells.Num(), Map.Rooms.Num());
}