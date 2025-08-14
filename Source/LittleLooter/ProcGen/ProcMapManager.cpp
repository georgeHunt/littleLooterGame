#include "ProcMapManager.h"
#include "MapGenerator.h"
#include "NavigationSystem.h"

AProcMapManager::AProcMapManager()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

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

	int32 FloorCount = 0, WallCount = 0;

	auto IsFloor = [this](const FMapData& M, int32 X, int32 Y)
	{
		if (const FCell* C = M.Cells.Find(FIntPoint(X,Y)))
			return C->Type == ECellType::Floor || C->Type == ECellType::Door;
		return false;
	};


	// ---------- PASS 1: FLOORS ----------
	for (const auto& KV : Map.Cells)
	{
		const FCell& C = KV.Value;
		const FVector Pos = GridToWorld(C.X, C.Y);

		if (C.Type == ECellType::Floor && Tileset->FloorMesh)
		{
			FTransform FloorTransform(FRotator::ZeroRotator, Pos, FVector(1.f));
			FloorHISM->AddInstance(FloorTransform);
			++FloorCount;
		}
	}

	// ---------- PASS 2: WALLS ----------
	if (Tileset->WallMesh)
	{
		const float S = Tileset->TileSize;
		const float H = Tileset->WallHeight;
		const float T = -300.f;

		auto PlaceEdge = [&](int32 X, int32 Y, float YawDeg, const FVector& LocalStart)
			{
				FTransform T(FRotator(0.f, YawDeg, 0.f), GridToWorld(X, Y) + LocalStart);
				T.AddToTranslation(FVector(0, 0, H * 0.5f));     // raise to mid-height
				WallHISM->AddInstance(T);
				++WallCount;
			};

		// Iterate the grid once; only place edges around FLOOR cells to avoid duplicates
		for (int32 y = 0; y < Params.Height; ++y)
			for (int32 x = 0; x < Params.Width; ++x)
			{
				if (!IsFloor(Map, x, y)) continue;

				// South edge of (x,y): start at BL corner of the cell
				if (!IsFloor(Map, x, y - 1))  PlaceEdge(x, y, 0.f, FVector(0.f, 0.f, 0.f));

				// North edge: start at (x, y+1)
				if (!IsFloor(Map, x, y + 1))  PlaceEdge(x, y, 0.f, FVector(0.f, S - T, 0.f));

				// West edge: start at (x, y), wall runs north-south
				if (!IsFloor(Map, x - 1, y))  PlaceEdge(x, y, 90.f, FVector(0.f, 0.f, 0.f));

				// East edge: start at (x+1, y)
				if (!IsFloor(Map, x + 1, y))  PlaceEdge(x, y, 90.f, FVector(S - T, 0.f, 0.f));
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