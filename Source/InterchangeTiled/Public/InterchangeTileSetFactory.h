// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "PaperTileSet.h"
#include "InterchangeTileSetFactory.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileSetFactory : public UInterchangeFactoryBase
{
	GENERATED_BODY()
	

public:

	virtual UClass* GetFactoryClass() const override
	{
		return UPaperTileSet::StaticClass();
	}

private:

	virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;

	virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;
};
