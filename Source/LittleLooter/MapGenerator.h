#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ProcTypes.h"
#include "MapGenerator.generated.h"

UCLASS()
class UMapGenerator : public UObject
{
	GENERATED_BODY()
public:
	FMapData Run(const FProcGenParams& Params, int32 Seed);

private:
	void StampRoom(FMapData& Out, const FIntRect& Rect);
	void CarveCorridor(FMapData& Out, const FIntPoint& A, const FIntPoint& B);
	bool IntersectsExisting(const FMapData& Map, const FIntRect& Rect) const;
};
