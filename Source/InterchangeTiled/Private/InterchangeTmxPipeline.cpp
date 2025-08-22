#include "InterchangeTmxPipeline.h"
#include "InterchangeTiledUtils.h"

#include "InterchangeManager.h"
#include "InterchangeTileMapFactoryNode.h"
#include "InterchangeTileMapNode.h"
#include "Misc/SecureHash.h"
#include "PaperTileMap.h"
#include "Misc/Paths.h"

// Helper struct for tileset info
struct FInterchangeParsedTilesetInfo {
    FString FirstGid;
    FString Name;
    FString TileWidth;
    FString TileHeight;
    FString ImageSource;
};

// Helper: Parse tilesets json array string to array of FInterchangeParsedTilesetInfo
static void ParseTilesetsJson(const FString& TilesetsStr, TArray<FInterchangeParsedTilesetInfo>& OutTilesets)
{
    TArray<FString> JsonItems;
    TilesetsStr.ParseIntoArray(JsonItems, TEXT("},{"), true);
    for (FString& Item : JsonItems)
    {
        // Clean up braces
        Item = Item.Replace(TEXT("{"), TEXT(""), ESearchCase::IgnoreCase);
        Item = Item.Replace(TEXT("}"), TEXT(""), ESearchCase::IgnoreCase);
        TArray<FString> Fields;
        Item.ParseIntoArray(Fields, TEXT(","), true);
        FInterchangeParsedTilesetInfo Info;
        for (const FString& Field : Fields)
        {
            FString Key, Value;
            if (Field.Split(TEXT(":"), &Key, &Value))
            {
                Key = Key.Replace(TEXT("\""), TEXT(""), ESearchCase::IgnoreCase);
                Value = Value.Replace(TEXT("\""), TEXT(""), ESearchCase::IgnoreCase);
                if (Key == "firstgid") Info.FirstGid = Value;
                else if (Key == "name") Info.Name = Value;
                else if (Key == "tilewidth") Info.TileWidth = Value;
                else if (Key == "tileheight") Info.TileHeight = Value;
                else if (Key == "image") Info.ImageSource = Value;
            }
        }
        OutTilesets.Add(Info);
    }
}

FString UInterchangeTmxPipeline::GetPipelineCategory(UClass* AssetClass)
{
	return TEXT("Assets");
}

void UInterchangeTmxPipeline::GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const
{
	PipelineSupportAssetClasses.Add(UPaperTileMap::StaticClass());
}

void UInterchangeTmxPipeline::ExecutePipeline(
	UInterchangeBaseNodeContainer* BaseNodeContainer,
	const TArray<UInterchangeSourceData*>& InSourceDatas,
	const FString& ContentBasePath
)
{
	if (!BaseNodeContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInterchangeTmxPipeline: Cannot execute pre-import pipeline because BaseNodeContainer is null"));
		return;
	}

	ensure(Results);

	TArray<FString> TileMapNodeUids;
	BaseNodeContainer->GetNodes(
		UInterchangeTileMapNode::StaticClass(), TileMapNodeUids
	);

	UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();

    FString TmxFilePath;
    if (InSourceDatas.Num() > 0 && InSourceDatas[0])
    {
        TmxFilePath = InSourceDatas[0]->GetFilename();
    }
    else
    {
        TmxFilePath = TEXT("");
    }

	for (FString TileMapNodeUid : TileMapNodeUids)
	{
		const UInterchangeBaseNode* Node = BaseNodeContainer->GetNode(TileMapNodeUid);
		const UInterchangeTileMapNode* TileMapNode = Cast<UInterchangeTileMapNode>(Node);

		FString TileSetFilename;
		TileMapNode->GetAttribute("TileSetFilename", TileSetFilename);

		UInterchangeTileMapFactoryNode* TileMapFactoryNode = NewObject<UInterchangeTileMapFactoryNode>(
			BaseNodeContainer,
			UInterchangeTileMapFactoryNode::StaticClass()
		);
		TileMapFactoryNode->InitializeNode(
			UInterchangeFactoryBaseNode::BuildFactoryNodeUid(TileMapNodeUid),
			TileMapNode->GetDisplayLabel() + "_tile_map",
			EInterchangeNodeContainerType::FactoryData
		);
		TileMapFactoryNode->SetAttribute("TileSetFilename", TileSetFilename);

		UInterchangeSourceData* SourceData = InterchangeManager.CreateSourceData(TileSetFilename);
		SourceData->GetFileContentHash();

		FImportAssetParameters ImportAssetParameters;
		ImportAssetParameters.bReplaceExisting = false;
		ImportAssetParameters.bIsAutomated = true;

		InterchangeManager.ImportAsset(
			ContentImportPath,
			SourceData,
			ImportAssetParameters
		);

		FString TilesetsStr;
		TileMapNode->GetAttribute("Tilesets", TilesetsStr);
		TArray<FInterchangeParsedTilesetInfo> Tilesets;
		ParseTilesetsJson(TilesetsStr, Tilesets);
		TArray<FString> TilesetFactoryNodeUids;
		for (const auto& Tileset : Tilesets)
		{
			auto* FactoryNode = NewObject<UInterchangeTileSetFactoryNode>(
				BaseNodeContainer,
				UInterchangeTileSetFactoryNode::StaticClass()
			);
			FactoryNode->InitializeNode(
				FGuid::NewGuid().ToString(),
				Tileset.Name + "_tileset",
				EInterchangeNodeContainerType::FactoryData
			);
			FactoryNode->SetAttribute("TextureFilename", InterchangeTiled::GetAbsoluteTexturePath(Tileset.ImageSource, TmxFilePath));
			FactoryNode->SetAttribute("FirstGid", Tileset.FirstGid);
			FactoryNode->SetAttribute("AssetName", Tileset.Name);
			FactoryNode->SetAttribute("TileWidth", Tileset.TileWidth);
			FactoryNode->SetAttribute("TileHeight", Tileset.TileHeight);
			TilesetFactoryNodeUids.Add(FactoryNode->GetUniqueID());
			BaseNodeContainer->AddNode(FactoryNode);
		}
		TileMapFactoryNode->SetAttribute("TilesetFactoryNodeUids", FString::Join(TilesetFactoryNodeUids, TEXT(",")));

		BaseNodeContainer->AddNode(TileMapFactoryNode);
	}
}
