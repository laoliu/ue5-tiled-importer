// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperTileSet.h"
#include "PT_Tileset.generated.h"

// A Paper 2D Tile Set extended to handle tile set data from
// Tiled
UCLASS(ClassGroup = PaperTiled, Category = "Paper Tiled", BlueprintType, Blueprintable)
class PAPERTILED_API UPT_Tileset : public UPaperTileSet
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Paper Tiled")
	FFilePath TiledTileset;

	void ImportTexture(FString TileSetName);
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
};
