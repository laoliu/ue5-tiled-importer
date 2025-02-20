#include "InterchangeTileMapFactory.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "InterchangeSourceData.h"
#include "InterchangeTiledModule.h"
#include "PaperTileLayer.h"
#include "PaperTileSet.h"
#include "XmlFile.h"

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

void UInterchangeTileMapFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	FString TileSetFilename;
	Arguments.FactoryNode->GetAttribute("TileSetFilename", TileSetFilename);

	FString TileSetAssetName = FPaths::GetBaseFilename(TileSetFilename) + "_tile_set";

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FString TileMapPathName = FPaths::GetPath(Arguments.ImportedObject->GetPathName());
	FString ObjectPath = FPaths::Combine(TileMapPathName, TileSetAssetName);

	TSet<FTopLevelAssetPath> ClassPaths{ UPaperTileSet::StaticClass()->GetClassPathName() };
	TSet<FName> PackageNames{ FName(*ObjectPath) };

	TArray<FAssetData> AssetData;

	FARCompiledFilter Filter;
	Filter.ClassPaths = ClassPaths;
	Filter.PackageNames = PackageNames;

	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	FAssetData* TileSetAssetData = AssetData.GetData();
	UObject* TileSetObject = TileSetAssetData->GetAsset();
	UPaperTileSet* TileSet = Cast<UPaperTileSet>(TileSetObject);

	FXmlFile TileMapFile(Arguments.SourceData->GetFilename());
	FXmlNode* RootNode = TileMapFile.GetRootNode();

	FString MapHeight = RootNode->GetAttribute("width");
	FString MapWidth = RootNode->GetAttribute("height");
	FString TileHeight = RootNode->GetAttribute("tileheight");
	FString TileWidth = RootNode->GetAttribute("tilewidth");

	UPaperTileMap* TileMap = Cast<UPaperTileMap>(Arguments.ImportedObject);
	TileMap->InitializeNewEmptyTileMap(TileSet);
	TileMap->MapHeight = FCString::Atoi(*MapHeight);
	TileMap->MapWidth = FCString::Atoi(*MapWidth);
	TileMap->TileHeight = FCString::Atoi(*TileHeight);
	TileMap->TileWidth = FCString::Atoi(*TileWidth);

	TArray<FXmlNode*> ChildrenNodes = RootNode->GetChildrenNodes();

	FXmlNode* LayerNode = nullptr;
	for (FXmlNode* ChildNode : ChildrenNodes)
	{
		if (ChildNode->GetTag() == "layer")
		{
			LayerNode = ChildNode;
			break;
		}
	}

	if (!LayerNode)
	{
		return;
	}

	FString LayerName = LayerNode->GetAttribute("name");
	int LayerHeight = FCString::Atoi(*LayerNode->GetAttribute("height"));
	int LayerWidth = FCString::Atoi(*LayerNode->GetAttribute("width"));;

	UPaperTileLayer* Layer = NewObject<UPaperTileLayer>(
		TileMap,
		UPaperTileLayer::StaticClass()
	);
	TileMap->TileLayers[0] = Layer;

	Layer->DestructiveAllocateMap(LayerWidth, LayerHeight);
	Layer->LayerName = FText::FromString(LayerName);

	const FXmlNode* LayerDataNode = LayerNode->GetFirstChildNode();
	// encoded as CSV w/ no newlines
	FString LayerData = LayerDataNode->GetContent();

	TArray<FString> TileUids;
	LayerData.ParseIntoArray(TileUids, TEXT(","));

	for (int TileIndex = 0; TileIndex < LayerWidth * LayerHeight; TileIndex++)
	{
		int32 TileUid = FCString::Atoi(*TileUids[TileIndex]) - 1;
		int TileX = TileIndex % 16;
		int TileY = TileIndex / 16;

		FPaperTileInfo TileInfo;
		TileInfo.TileSet = TileSet;
		TileInfo.PackedTileIndex = TileUid;

		Layer->SetCell(
			TileX,
			TileY,
			TileInfo
		);
	}
}
