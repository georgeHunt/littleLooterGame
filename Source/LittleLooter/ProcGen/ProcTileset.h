#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProcTileset.generated.h"

UCLASS(BlueprintType)
class UProcTileset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* FloorMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* WallMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WallHeight = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize = 100.f;
};
