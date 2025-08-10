#include "MapGenerator.h"

static FORCEINLINE FIntPoint RandPoint(FRandomStream& R, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY)
{
	return FIntPoint(R.RandRange(MinX, MaxX), R.RandRange(MinY, MaxY));
}

FMapData UMapGenerator::Run(const FProcGenParams& Params, int32 Seed)
{
	FRandomStream Rand(Seed);
	FMapData Map;

	// 1) Rooms: rejection sampling rectangles
	for (int32 i = 0; i < Params.RoomAttempts; ++i)
	{
		const int32 W = Rand.RandRange(Params.MinRoomSize, Params.MaxRoomSize);
		const int32 H = Rand.RandRange(Params.MinRoomSize, Params.MaxRoomSize);
		const int32 X = Rand.RandRange(1, Params.Width - W - 2);
		const int32 Y = Rand.RandRange(1, Params.Height - H - 2);
		FIntRect Rect(X, Y, X + W, Y + H);
		if (IntersectsExisting(Map, Rect)) continue;
		StampRoom(Map, Rect);
		FRoom Rm; Rm.Bounds = Rect; Map.Rooms.Add(Rm);
	}

	if (Map.Rooms.Num() == 0) return Map; // nothing to do

	// 2) Connect rooms in sequence (MVP). Then add a few extra corridors.
	for (int32 i = 1; i < Map.Rooms.Num(); ++i)
	{
		const FIntPoint A((Map.Rooms[i - 1].Bounds.Min.X + Map.Rooms[i - 1].Bounds.Max.X) / 2,
			(Map.Rooms[i - 1].Bounds.Min.Y + Map.Rooms[i - 1].Bounds.Max.Y) / 2);
		const FIntPoint B((Map.Rooms[i].Bounds.Min.X + Map.Rooms[i].Bounds.Max.X) / 2,
			(Map.Rooms[i].Bounds.Min.Y + Map.Rooms[i].Bounds.Max.Y) / 2);
		CarveCorridor(Map, A, B);
	}
	for (int32 k = 0; k < Params.ExtraCorridors && Map.Rooms.Num() > 1; ++k)
	{
		int32 I = Rand.RandRange(0, Map.Rooms.Num() - 1);
		int32 J = Rand.RandRange(0, Map.Rooms.Num() - 1);
		if (I == J) continue;
		const FIntPoint A((Map.Rooms[I].Bounds.Min.X + Map.Rooms[I].Bounds.Max.X) / 2,
			(Map.Rooms[I].Bounds.Min.Y + Map.Rooms[I].Bounds.Max.Y) / 2);
		const FIntPoint B((Map.Rooms[J].Bounds.Min.X + Map.Rooms[J].Bounds.Max.X) / 2,
			(Map.Rooms[J].Bounds.Min.Y + Map.Rooms[J].Bounds.Max.Y) / 2);
		CarveCorridor(Map, A, B);
	}

	// 3) Walls pass: empty cells adjacent to floor ? wall
	auto IsFloor = [&Map](int32 X, int32 Y) {
		const FCell* C = Map.Cells.Find(FIntPoint(X, Y));
		return C && (C->Type == ECellType::Floor || C->Type == ECellType::Door);
		};
	for (int32 y = 0; y < Params.Height; ++y)
		for (int32 x = 0; x < Params.Width; ++x)
		{
			FIntPoint Key(x, y);
			const FCell* C = Map.Cells.Find(Key);
			if (C && C->Type != ECellType::Empty) continue;
			bool NearFloor = IsFloor(x + 1, y) || IsFloor(x - 1, y) || IsFloor(x, y + 1) || IsFloor(x, y - 1);
			if (NearFloor)
			{
				Map.Cells.Add(Key, FCell(x, y, ECellType::Wall));
			}
		}

	return Map;
}

void UMapGenerator::StampRoom(FMapData& Out, const FIntRect& Rect)
{
	for (int32 y = Rect.Min.Y; y < Rect.Max.Y; ++y)
		for (int32 x = Rect.Min.X; x < Rect.Max.X; ++x)
		{
			Out.Cells.Add(FIntPoint(x, y), FCell(x, y, ECellType::Floor));
		}
}

void UMapGenerator::CarveCorridor(FMapData& Out, const FIntPoint& A, const FIntPoint& B)
{
	// L-shaped (Manhattan). Randomize horizontal-first or vertical-first could be added.
	int32 x = A.X, y = A.Y;
	const auto Carve = [&Out](int32 CX, int32 CY) { Out.Cells.Add(FIntPoint(CX, CY), FCell(CX, CY, ECellType::Floor)); };

	while (x != B.X) { x += (B.X > x) ? 1 : -1; Carve(x, y); }
	while (y != B.Y) { y += (B.Y > y) ? 1 : -1; Carve(x, y); }
}

bool UMapGenerator::IntersectsExisting(const FMapData& Map, const FIntRect& Rect) const
{
	// one tile padding
	FIntRect Padded(Rect.Min.X - 1, Rect.Min.Y - 1, Rect.Max.X + 1, Rect.Max.Y + 1);
	for (int32 y = Padded.Min.Y; y < Padded.Max.Y; ++y)
		for (int32 x = Padded.Min.X; x < Padded.Max.X; ++x)
		{
			if (Map.Cells.Contains(FIntPoint(x, y))) return true;
		}
	return false;
}
