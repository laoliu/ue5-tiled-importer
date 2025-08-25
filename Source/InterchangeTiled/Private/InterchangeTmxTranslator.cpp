#include "InterchangeTmxTranslator.h"

#include "InterchangeTiledModule.h"
#include "InterchangeTiledUtils.h"
#include "InterchangeTileMapNode.h"
#include "Logging/StructuredLog.h"
#include "XmlFile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTmxTranslator)

// global variable to enable/disable TMX import from the Unreal console
static bool bGInterchangeEnableTmxImport = true;
static FAutoConsoleVariableRef CCvarInterchangeEnableTmxImport(
	TEXT("Interchange.FeatureFlags.Import.TMX"),
	bGInterchangeEnableTmxImport,
	TEXT("Whether TMX support is enabled."),
	ECVF_Default
);

EInterchangeTranslatorAssetType UInterchangeTmxTranslator::GetSupportedAssetTypes() const
{
	return EInterchangeTranslatorAssetType::None;
}

TArray<FString> UInterchangeTmxTranslator::GetSupportedFormats() const
{
#if WITH_EDITOR
	if (bGInterchangeEnableTmxImport)
	{
		TArray<FString> Formats{ TEXT("tmx;Tiled Tile Map") };
		return Formats;
	}
#endif
	return TArray<FString>{};
}

EInterchangeTranslatorType UInterchangeTmxTranslator::GetTranslatorType() const
{
	return EInterchangeTranslatorType::Assets;
}

bool UInterchangeTmxTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{

	FString Filename = SourceData->GetFilename();
	FPaths::NormalizeFilename(Filename);

	// 只处理 .tmx 文件
	if (!Filename.EndsWith(TEXT(".tmx"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	if (!FPaths::FileExists(Filename))
	{
		return false;
	}

	return TranslateTileMap(Filename, BaseNodeContainer);
}

bool UInterchangeTmxTranslator::TranslateTileMap(FString Filename, UInterchangeBaseNodeContainer& BaseNodeContainer) const
{

	UClass* TileSetClass = UInterchangeTileMapNode::StaticClass();

	if (!ensure(TileSetClass))
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Error importing TSX tile set: UInterchangeTileSetNode is unsupported."))

			return false;
	}

	FString DisplayLabel = FPaths::GetBaseFilename(Filename);
	FString NodeUid("tmx:" + DisplayLabel);

	UInterchangeTileMapNode* TileMapNode = NewObject<UInterchangeTileMapNode>(&BaseNodeContainer, TileSetClass);

	if (!ensure(TileMapNode))
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Error importing TSX tile set: Failed to create UInterchangeTileMapNode."));

		return false;
	}

	TileMapNode->InitializeNode(
		NodeUid,
		DisplayLabel,
		EInterchangeNodeContainerType::TranslatedAsset
	);

	FXmlFile TileMapFile(Filename);
	FXmlNode* RootNode = TileMapFile.GetRootNode();
	TArray<FXmlNode*> ChildrenNodes = RootNode->GetChildrenNodes();

	TArray<FString> TilesetJsonArray;

	for (FXmlNode* ChildNode : ChildrenNodes)
	{
		if (ChildNode->GetTag() == "tileset")
		{
			FString FirstGid = ChildNode->GetAttribute("firstgid");
			FString Source = ChildNode->GetAttribute("source");
			if (!Source.IsEmpty())
			{
				// 解析 TSX 文件，获取详细信息
				FString TsxPath = FPaths::Combine(FPaths::GetPath(Filename), Source);
				FXmlFile TsxFile(TsxPath);
				FXmlNode* TsxRoot = TsxFile.GetRootNode();
				FString Name = TsxRoot->GetAttribute("name");
				FString TileWidth = TsxRoot->GetAttribute("tilewidth");
				FString TileHeight = TsxRoot->GetAttribute("tileheight");
				FXmlNode* ImageNode = TsxRoot->FindChildNode("image");
				FString ImageSource = ImageNode ? ImageNode->GetAttribute("source") : "";
				FString TilesetJson = FString::Printf(
					TEXT("{\"firstgid\":%s,\"source\":\"%s\",\"name\":\"%s\",\"tilewidth\":%s,\"tileheight\":%s,\"image\":\"%s\"}"),
					*FirstGid, *Source, *Name, *TileWidth, *TileHeight, *ImageSource
				);
				TilesetJsonArray.Add(TilesetJson);
			}
			else
			{
				// 内嵌 tileset，记录详细信息
				FString Name = ChildNode->GetAttribute("name");
				FString TileWidth = ChildNode->GetAttribute("tilewidth");
				FString TileHeight = ChildNode->GetAttribute("tileheight");
				FXmlNode* ImageNode = ChildNode->FindChildNode("image");
				FString ImageSource = ImageNode ? ImageNode->GetAttribute("source") : "";

				FString TilesetJson = FString::Printf(
					TEXT("{\"firstgid\":%s,\"name\":\"%s\",\"tilewidth\":%s,\"tileheight\":%s,\"image\":\"%s\"}"),
					*FirstGid, *Name, *TileWidth, *TileHeight, *ImageSource
				);
				TilesetJsonArray.Add(TilesetJson);
			}
		}
	}

	// 存到Node，组装为json数组格式
	TileMapNode->SetAttribute("Tilesets", FString::Printf(TEXT("[%s]"), *FString::Join(TilesetJsonArray, TEXT(","))));

	BaseNodeContainer.AddNode(TileMapNode);
	return true;
}
