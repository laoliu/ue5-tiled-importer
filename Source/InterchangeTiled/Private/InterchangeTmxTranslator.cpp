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
	TileMapNode->SetAttribute("TileSetFilename", GetTileSetFilenameFromSourceFilename(Filename));

	FXmlFile TileMapFile(Filename);
	FXmlNode* RootNode = TileMapFile.GetRootNode();
	TArray<FXmlNode*> ChildrenNodes = RootNode->GetChildrenNodes();

	TArray<FString> TilesetJsonArray;

	for (FXmlNode* ChildNode : ChildrenNodes)
	{
		if (ChildNode->GetTag() == "tileset")
		{
			FString FirstGid = ChildNode->GetAttribute("firstgid");
			FString Name = ChildNode->GetAttribute("name");
			FString TileWidth = ChildNode->GetAttribute("tilewidth");
			FString TileHeight = ChildNode->GetAttribute("tileheight");
			FXmlNode* ImageNode = ChildNode->FindChildNode("image");
			FString ImageSource = ImageNode ? ImageNode->GetAttribute("source") : "";

			// 组装为json字符串或其他格式
			FString TilesetJson = FString::Printf(
				TEXT("{\"firstgid\":%s,\"name\":\"%s\",\"tilewidth\":%s,\"tileheight\":%s,\"image\":\"%s\"}"),
				*FirstGid, *Name, *TileWidth, *TileHeight, *ImageSource
			);
			TilesetJsonArray.Add(TilesetJson);
		}
	}

	// 存到Node
	TileMapNode->SetAttribute("Tilesets", FString::Join(TilesetJsonArray, TEXT(",")));

	BaseNodeContainer.AddNode(TileMapNode);
	return true;
}

FString UInterchangeTmxTranslator::GetTileSetFilenameFromSourceFilename(FString Filename)
{
	FXmlFile TileMapFile(Filename);
	FXmlNode* RootNode = TileMapFile.GetRootNode();
	const FXmlNode* TileSetNode = RootNode->FindChildNode("tileset");
	FString ImageSource = TileSetNode->GetAttribute("source");

	return InterchangeTiled::GetAbsolutePath(ImageSource, Filename);
}

