#include "InterchangeTsxTranslator.h"

#include "InterchangeTexture2DNode.h"
#include "InterchangeTiledModule.h"
#include "InterchangeTiledUtils.h"
#include "InterchangeTileSetNode.h"
#include "InterchangeTileSetFactoryNode.h"
#include "ImageUtils.h"
#include "Logging/StructuredLog.h"
#include "Misc/Optional.h"
#include "Nodes/InterchangeSourceNode.h"
#include "XmlFile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTsxTranslator)

// global variable to enable/disable TSX import from the Unreal console
static bool bGInterchangeEnableTsxImport = true;
static FAutoConsoleVariableRef CCvarInterchangeEnableTsxImport(
	TEXT("Interchange.FeatureFlags.Import.TSX"),
	bGInterchangeEnableTsxImport,
	TEXT("Whether TSX support is enabled."),
	ECVF_Default
);

bool UInterchangeTsxTranslator::CanImportSourceData(const UInterchangeSourceData* InSourceData) const
{
	return true;
}

EInterchangeTranslatorAssetType UInterchangeTsxTranslator::GetSupportedAssetTypes() const
{
	return EInterchangeTranslatorAssetType::None;
}

TArray<FString> UInterchangeTsxTranslator::GetSupportedFormats() const
{
#if WITH_EDITOR
	if (bGInterchangeEnableTsxImport)
	{
		TArray<FString> Formats{ TEXT("tsx;Tiled Tile Set") };
		return Formats;
	}
#endif
	return TArray<FString>{};
}

EInterchangeTranslatorType UInterchangeTsxTranslator::GetTranslatorType() const
{
	return EInterchangeTranslatorType::Assets;
}

bool UInterchangeTsxTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
	FString Filename = SourceData->GetFilename();
	FPaths::NormalizeFilename(Filename);

	if (!FPaths::FileExists(Filename))
	{
		return false;
	}

	return TranslateTileSet(Filename, BaseNodeContainer);
}

bool UInterchangeTsxTranslator::TranslateTileSet(
	FString Filename,
	UInterchangeBaseNodeContainer& BaseNodeContainer
) const
{
	UClass* TileSetClass = UInterchangeTileSetNode::StaticClass();

	if (!ensure(TileSetClass))
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Error importing TSX tile set: UInterchangeTileSetNode is unsupported."))

		return false;
	}

	FString DisplayLabel = FPaths::GetBaseFilename(Filename);
	FString NodeUid("tsx:" + DisplayLabel);

	UInterchangeTileSetNode* TileSetNode = NewObject<UInterchangeTileSetNode>(&BaseNodeContainer, TileSetClass);

	if (!ensure(TileSetNode))
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Error importing TSX tile set: Failed to create UInterchangeTileSetNode."));

		return false;
	}

	TileSetNode->InitializeNode(
		NodeUid, 
		DisplayLabel, 
		EInterchangeNodeContainerType::TranslatedAsset
	);
	TileSetNode->SetAttribute("TextureFilename", GetTexturePathFromSourceFilename(Filename));

	BaseNodeContainer.AddNode(TileSetNode);

	return true;
}

FString UInterchangeTsxTranslator::GetTexturePathFromSourceFilename(FString Filename)
{
	FXmlFile TileSetFile(Filename);
	FXmlNode* RootNode = TileSetFile.GetRootNode();
	const FXmlNode* ImageNode = RootNode->FindChildNode("image");
	FString ImageSource = ImageNode->GetAttribute("source");

	return InterchangeTiled::GetAbsolutePath(ImageSource, Filename);
}
