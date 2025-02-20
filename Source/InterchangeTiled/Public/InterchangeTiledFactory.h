// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "InterchangeTiledFactory.generated.h"

/**
 * A baseline Interchange Factory with customized logging.
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTiledFactory : public UInterchangeFactoryBase
{
	GENERATED_BODY()

protected:
	// standardized error logging for use in BeginImportAsset
	virtual void LogAssetCreationError(
		const FImportAssetObjectParams& Arguments,
		const FText& Info, 
		FImportAssetResult& ImportAssetResult
	);

	// standardized error logging for use in SetupObject
	virtual void LogAssetSetupError(
		const FSetupObjectParams& Arguments,
		const FText& Info,
		FImportAssetResult& ImportAssetResult
	);
};
