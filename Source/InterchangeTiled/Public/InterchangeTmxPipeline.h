#pragma once

#include "CoreMinimal.h"
#include "InterchangePipelineBase.h"
#include "InterchangeTileMapFactoryNode.h"
#include "InterchangeTsxPipeline.h"

#include "InterchangeTmxPipeline.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTmxPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()

public:
	static FString GetPipelineCategory(UClass* AssetClass);

	virtual void GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const override;

protected:

	virtual void ExecutePipeline(
		UInterchangeBaseNodeContainer* BaseNodeContainer, 
		const TArray<UInterchangeSourceData*>& InSourceDatas, 
		const FString& ContentBasePath
	) override;

};
