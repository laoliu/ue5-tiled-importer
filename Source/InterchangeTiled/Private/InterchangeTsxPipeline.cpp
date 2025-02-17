#include "InterchangeTsxPipeline.h"

#include "PaperTileSet.h"
#include "InterchangeGenericTexturePipeline.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTsxPipeline)

#define LOCTEXT_NAMESPACE "InterchangeTsxPipeline"

UInterchangeTsxPipeline::UInterchangeTsxPipeline()
{
	TexturePipeline = CreateDefaultSubobject<UInterchangeGenericTexturePipeline>("TexturePipeline");
}

FString UInterchangeTsxPipeline::GetPipelineCategory(UClass* AssetClass)
{
	return TEXT("Assets");
}

void UInterchangeTsxPipeline::GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const
{
	if (TexturePipeline)
	{
		TexturePipeline->GetSupportAssetClasses(PipelineSupportAssetClasses);
	}

	PipelineSupportAssetClasses.Add(UPaperTileSet::StaticClass());
}

void UInterchangeTsxPipeline::ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, const TArray<UInterchangeSourceData*>& InSourceDatas, const FString& ContentBasePath)
{
	if (!InBaseNodeContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInterchangeTsxPipeline: Cannot execute pre-import pipeline because InBaseNodeContrainer is null"));
		return;
	}

	ensure(Results);
	{
		if (TexturePipeline)
		{
			TexturePipeline->SetResultsContainer(Results);
		}
	}

	BaseNodeContainer = InBaseNodeContainer;

	if (TexturePipeline)
	{
		TexturePipeline->ScriptedExecutePipeline(InBaseNodeContainer, InSourceDatas, ContentBasePath);
	}
}
