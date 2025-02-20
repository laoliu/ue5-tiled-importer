#include "InterchangeTileMapFactory.h"

#include "InterchangeSourceData.h"
#include "InterchangeTiledModule.h"

UInterchangeFactoryBase::FImportAssetResult UInterchangeTileMapFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInterchangeTileMapFactory::BeginImportAsset_GameThread);

	FImportAssetResult ImportAssetResult;
	UPaperTileMap* TileMap = nullptr;

	if (!Arguments.AssetNode)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_AssetNodeNull", "Asset node parameter is null."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	const UClass* TileMapClass = Arguments.AssetNode->GetObjectClass();

	if (!TileMapClass || !TileMapClass->IsChildOf(UPaperTileMap::StaticClass()))
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_NodeClassMissmatch", "Asset node parameter class doesn't derive from UPaperTileMap."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	UObject* ExistingAsset = Arguments.ReimportObject;

	// even if not flagged as a re-import, try to load the asset in case
	// it already exists
	if (!ExistingAsset)
	{
		FSoftObjectPath ReferenceObject;
		if (Arguments.AssetNode->GetCustomReferenceObject(ReferenceObject))
		{
			ExistingAsset = ReferenceObject.TryLoad();
		}
	}

	if (!ExistingAsset)
	{
		TileMap = NewObject<UPaperTileMap>(
			Arguments.Parent,
			TileMapClass,
			*Arguments.AssetName,
			RF_Public | RF_Standalone
		);
	}

	if (!TileMap)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_TileMapCreateFail", "Tile Map creation failed."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = TileMap;

	return ImportAssetResult;
}
