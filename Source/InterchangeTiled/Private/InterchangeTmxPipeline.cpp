#include "InterchangeTmxPipeline.h"

#include "InterchangeManager.h"
#include "InterchangeTileMapFactoryNode.h"
#include "InterchangeTileMapNode.h"
#include "Misc/SecureHash.h"
#include "PaperTileMap.h"

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

		// need to init the hash since it's not done automatically
		SourceData->GetFileContentHash();

		FImportAssetParameters ImportAssetParameters;
		ImportAssetParameters.bReplaceExisting = false;
		ImportAssetParameters.bIsAutomated = true;

		InterchangeManager.ImportAsset(
			ContentImportPath,
			SourceData,
			ImportAssetParameters
		);

		BaseNodeContainer->AddNode(TileMapFactoryNode);
	}
}
