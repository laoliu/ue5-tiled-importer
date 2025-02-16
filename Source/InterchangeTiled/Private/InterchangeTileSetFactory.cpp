#include "InterchangeTileSetFactory.h"

#include "InterchangeSourceData.h"
#include "InterchangeTextureFactoryNode.h"
#include "InterchangeTileSetFactoryNode.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "XmlFile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTileSetFactory)

#define LOCTEXT_NAMESPACE "InterchangeTileSetFactory"

UInterchangeFactoryBase::FImportAssetResult UInterchangeTileSetFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInterchangeTileSetFactory::BeginImportAsset_GameThread);

	FImportAssetResult ImportAssetResult;
	UPaperTileSet* TileSet = nullptr;

	// lambda to standardize log message when asset creation fails
	auto CouldNotCreateTileSetLog = [this, &Arguments, &ImportAssetResult](const FText& Info)
	{
		UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();
		Message->SourceAssetName = Arguments.SourceData->GetFilename();
		Message->DestinationAssetName = Arguments.AssetName;
		Message->AssetType = GetFactoryClass();
		Message->Text = FText::Format(LOCTEXT("TileSetFact_CouldNotCreateMat", "UInterchangeTileSetFactory: Could not create tile set asset %s. Reason: %s"), FText::FromString(Arguments.AssetName), Info);
		ImportAssetResult.bIsFactorySkipAsset = true;
	};

	if (!Arguments.AssetNode)
	{
		CouldNotCreateTileSetLog(LOCTEXT("TileSetFactory_AssetNodeNull", "Asset node parameter is null."));
		return ImportAssetResult;
	}

	const UClass* TileSetClass = Arguments.AssetNode->GetObjectClass();
	if (!TileSetClass || !TileSetClass->IsChildOf(UPaperTileSet::StaticClass()))
	{
		CouldNotCreateTileSetLog(LOCTEXT("TileSetFactory_NodeClassMissmatch", "Asset node parameter class doesn't derive from UPaperTileSet."));
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
		TileSet = NewObject<UPaperTileSet>(
			Arguments.Parent,
			TileSetClass,
			*Arguments.AssetName,
			RF_Public | RF_Standalone
		);
	}

	if (!TileSet)
	{
		CouldNotCreateTileSetLog(LOCTEXT("TileSetFactory_TileSetCreateFail", "Tile Set creation failed."));
		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = TileSet;

	return ImportAssetResult;
}

void UInterchangeTileSetFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	if (Arguments.FactoryNode->GetClass() != UInterchangeTileSetFactoryNode::StaticClass())
	{
		return;
	}

	// load the dependency texture
	FString TextureFactoryUid;
	Arguments.FactoryNode->GetFactoryDependency(0, TextureFactoryUid);

	UInterchangeFactoryBaseNode* TextureFactoryNode = Arguments.NodeContainer->GetFactoryNode(TextureFactoryUid);

	if (TextureFactoryNode->GetClass() != UInterchangeTextureFactoryNode::StaticClass())
	{
		return;
	}

	FSoftObjectPath TextureObjectPath;
	TextureFactoryNode->GetCustomReferenceObject(TextureObjectPath);

	UTexture2D* Texture = Cast<UTexture2D>(TextureObjectPath.TryLoad());

	if (!Texture)
	{
		return;
	}

	FXmlFile TileSetFile(Arguments.SourceData->GetFilename());
	FXmlNode* RootNode = TileSetFile.GetRootNode();

	FString TileSetName = RootNode->GetAttribute("name");
	FString TileWidth = RootNode->GetAttribute("tilewidth");
	FString TileHeight = RootNode->GetAttribute("tileheight");
	FString TileSpacing = RootNode->GetAttribute("spacing");
	FString ImageMargin = RootNode->GetAttribute("margin");
	FString TileCount = RootNode->GetAttribute("tilecount");
	FString ColumnCount = RootNode->GetAttribute("columns");

	UPaperTileSet* TileSet = Cast<UPaperTileSet>(Arguments.ImportedObject);

	TileSet->SetTileSize(FIntPoint(FCString::Atoi(*TileWidth), FCString::Atoi(*TileHeight)));
	TileSet->SetMargin(FCString::Atoi(*ImageMargin));
	TileSet->SetPerTileSpacing(FCString::Atoi(*TileSpacing));
	TileSet->SetTileSheetTexture(Texture);

	UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();

	const FPaperTileMetadata* TileMetadata = TileSet->GetTileMetadata(0);

	Message->Text = FText::Format(FText::FromString("Paper tile metadata exists: %s"), FText::FromString(TileMetadata ? "Yes" : "No"));
}
