#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProcTypes.generated.h"

UENUM(BlueprintType)
enum class ECellType : uint8 { Empty, Floor, Wall, Door };

USTRUCT(BlueprintType)
struct FProcGenParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Width = 80;   // in tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Height = 60;  // in tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RoomAttempts = 80;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinRoomSize = 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxRoomSize = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ExtraCorridors = 6; // extra connections
};

USTRUCT()
struct FCell
{
	GENERATED_BODY()
	FCell() {}
	FCell(int32 InX, int32 InY, ECellType InType) : X(InX), Y(InY), Type(InType) {}
	int32 X = 0;
	int32 Y = 0;
	ECellType Type = ECellType::Empty;
};

USTRUCT()
struct FRoom
{
	GENERATED_BODY()
	FIntRect Bounds; // inclusive min, exclusive max (like Rect: [Min, Max))
	TArray<FIntPoint> DoorCells;
};

USTRUCT()
struct FMapData
{
	GENERATED_BODY()
	TMap<FIntPoint, FCell> Cells; // key = (X,Y)
	TArray<FRoom> Rooms;
};
