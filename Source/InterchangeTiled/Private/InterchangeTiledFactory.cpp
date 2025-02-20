#include "InterchangeTiledFactory.h"

#include "InterchangeSourceData.h"
#include "InterchangeTiledModule.h"

void UInterchangeTiledFactory::LogAssetCreationError(
	const FImportAssetObjectParams& Arguments,
	const FText& Info, 
	FImportAssetResult& ImportAssetResult
)
{
	UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();
	Message->SourceAssetName = Arguments.SourceData->GetFilename();
	Message->DestinationAssetName = Arguments.AssetName;
	Message->AssetType = GetFactoryClass();
	Message->Text = FText::Format(
		LOCTEXT("TiledFact_CouldNotCreateAsset", "%s: Could not create asset %s. Reason: %s"),
		FText::FromString(GetFactoryClass()->GetName()),
		FText::FromString(Arguments.AssetName),
		Info
	);
	ImportAssetResult.bIsFactorySkipAsset = true;
}

void UInterchangeTiledFactory::LogAssetSetupError(
	const FSetupObjectParams& Arguments,
	const FText& Info, 
	FImportAssetResult& ImportAssetResult
)
{
	UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();
	Message->SourceAssetName = Arguments.SourceData->GetFilename();
	Message->DestinationAssetName = Arguments.ImportedObject->GetPathName();
	Message->AssetType = GetFactoryClass();
	Message->Text = FText::Format(
		LOCTEXT("TiledFact_CouldNotCreateAsset", "%s: Could not create asset %s. Reason: %s"),
		FText::FromString(GetFactoryClass()->GetName()),
		FText::FromString(Arguments.ImportedObject->GetName()),
		Info
	);
	ImportAssetResult.bIsFactorySkipAsset = true;
}
