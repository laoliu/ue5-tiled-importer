// Copyright Epic Games, Inc. All Rights Reserved.

#include "PaperTiledEditor.h"

#define LOCTEXT_NAMESPACE "FPaperTiledEditorModule"

void FPaperTiledEditorModule::StartupModule()
{
	PaperTiledAssetTypeActions = MakeShared<FTileSetAssetTypeActions>();
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	PaperTiledAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("PaperTiled")), LOCTEXT("PaperTiledAssetCategory", "Paper Tiled"));

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FTileSetAssetTypeActions(PaperTiledAssetCategory)));
}

void FPaperTiledEditorModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	AssetTools.UnregisterAssetTypeActions(PaperTiledAssetTypeActions.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPaperTiledEditorModule, PaperTiledEditor)