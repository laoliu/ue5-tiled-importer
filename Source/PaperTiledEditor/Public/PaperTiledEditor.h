// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AssetTypeActions/AssetTypeActions_ClassTypeBase.h"
#include "Modules/ModuleManager.h"
#include "TileSetAssetTypeActions.h"

class FPaperTiledEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FTileSetAssetTypeActions> PaperTiledAssetTypeActions;
	EAssetTypeCategories::Type PaperTiledAssetCategory;
};