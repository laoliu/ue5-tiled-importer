// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperTileSet.h"
#include "AssetRegistry/AssetRegistryModule.h"
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

	static UTexture2D* GetExistingTextureAsset(
		FAssetRegistryModule& AssetRegistryModule,
		FString AssetPath,
		FString ImageFileName
	);

private:

	UTexture2D* ImportTexture(FString ImageSource);
	UTexture2D* ImportTexture(FString RelativeImageSource, FString InTileSetPath);

	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
};
