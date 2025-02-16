// Fill out your copyright notice in the Description page of Project Settings.

#include "InterchangeTsxTranslator.h"

#include "InterchangeTexture2DNode.h"
#include "InterchangeTiledModule.h"
#include "InterchangeTileSetNode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTsxTranslator)

#define LOCTEXT_NAMESPACE "InterchangeTsxTranslator"

// global variable to enable/disable TSX import from the Unreal console
static bool bGInterchangeEnableTsxImport = true;
static FAutoConsoleVariableRef CCvarInterchangeEnableTsxImport(
	TEXT("Interchange.FeatureFlags.Import.TSX"),
	bGInterchangeEnableTsxImport,
	TEXT("Whether TSX support is enabled."),
	ECVF_Default
);

EInterchangeTranslatorAssetType UInterchangeTsxTranslator::GetSupportedAssetTypes() const
{
	return EInterchangeTranslatorAssetType::Textures;
}

TArray<FString> UInterchangeTsxTranslator::GetSupportedFormats() const
{
#if WITH_EDITOR
	if (bGInterchangeEnableTsxImport)
	{
		TArray<FString> Formats{ TEXT("tsx;Tiled Tileset") };
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
	UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Calling TSX Translator"));

	FString Filename = SourceData->GetFilename();
	FPaths::NormalizeFilename(Filename);

	if (!FPaths::FileExists(Filename))
	{
		return false;
	}

	return TranslateTexture(Filename, BaseNodeContainer);
}

bool UInterchangeTsxTranslator::TranslateTexture(
	FString Filename,
	UInterchangeBaseNodeContainer& BaseNodeContainer
) const
{
	UClass* Class = UInterchangeTexture2DNode::StaticClass();
	if (!ensure(Class))
	{
		return false;
	}

	FString DisplayLabel = FPaths::GetBaseFilename(Filename);
	FString NodeUid(Filename);

	UInterchangeTextureNode* TextureNode = NewObject<UInterchangeTextureNode>(&BaseNodeContainer, Class);
	if (!ensure(TextureNode))
	{
		return false;
	}

	TextureNode->InitializeNode(NodeUid, DisplayLabel, EInterchangeNodeContainerType::TranslatedAsset);
	TextureNode->SetPayLoadKey(Filename);

	BaseNodeContainer.AddNode(TextureNode);

	return true;
}

bool UInterchangeTsxTranslator::TranslateTileSet(
	FString Filename,
	UInterchangeBaseNodeContainer& BaseNodeContainer
) const
{
	UClass* Class = UInterchangeTileSetNode::StaticClass();
	if (!ensure(Class))
	{
		return false;
	}

	FString DisplayLabel = FPaths::GetBaseFilename(Filename) + "_tile_set";
	FString NodeUid(Filename);

	UInterchangeTextureNode* TileSetNode = NewObject<UInterchangeTextureNode>(&BaseNodeContainer, Class);
	if (!ensure(TileSetNode))
	{
		return false;
	}

	TileSetNode->InitializeNode(NodeUid, DisplayLabel, EInterchangeNodeContainerType::TranslatedAsset);
	TileSetNode->SetPayLoadKey(Filename);

	BaseNodeContainer.AddNode(TileSetNode);

	return true;
}
