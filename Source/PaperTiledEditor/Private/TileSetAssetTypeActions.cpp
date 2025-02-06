// Fill out your copyright notice in the Description page of Project Settings.


#include "TileSetAssetTypeActions.h"
#include "PT_Tileset.h"

#define LOCTEXT_NAMESPACE "PaperTiledActions"

FTileSetAssetTypeActions::FTileSetAssetTypeActions(
	EAssetTypeCategories::Type InCategoryType
) : MyAssetCategory(InCategoryType)
{
}

FTileSetAssetTypeActions::~FTileSetAssetTypeActions()
{
}

UClass* FTileSetAssetTypeActions::GetSupportedClass() const
{
	return UPT_Tileset::StaticClass();
}

FText FTileSetAssetTypeActions::GetName() const 
{
	return LOCTEXT("PaperTiledAssetCategory", "Tile Set");
}

FColor FTileSetAssetTypeActions::GetTypeColor() const
{
	return FColor::Cyan;
}

uint32 FTileSetAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}