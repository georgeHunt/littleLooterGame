#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ProcTypes.h"
#include "ProcTileset.h"
#include "ProcMapManager.generated.h"

UCLASS()
class AProcMapManager : public AActor
{
	GENERATED_BODY()
public:
	AProcMapManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProcGen") int32 Seed = 1337;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProcGen") FProcGenParams Params;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProcGen") TObjectPtr<UProcTileset> Tileset;
	UPROPERTY(EditAnywhere, Category="ProcGen") float TileSize = 400.f; // cm per tile

	UFUNCTION(CallInEditor, BlueprintCallable) void Generate();
	UFUNCTION(CallInEditor, BlueprintCallable) void Clear();

protected:
	virtual void BeginPlay() override;

private:
	UHierarchicalInstancedStaticMeshComponent* FloorHISM = nullptr;
	UHierarchicalInstancedStaticMeshComponent* WallHISM = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<class UMapGenerator> Generator;

	FMapData Map;

	void EnsureComponents();
	FVector GridToWorld(int32 X, int32 Y) const { return GetActorLocation() + FVector(X*TileSize, Y*TileSize, 0.f); }
};
