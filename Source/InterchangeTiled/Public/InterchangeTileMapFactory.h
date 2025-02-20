#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "InterchangeTiledFactory.h"
#include "PaperTileMap.h"

#include "InterchangeTileMapFactory.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileMapFactory : public UInterchangeTiledFactory
{
	GENERATED_BODY()


public:

	virtual UClass* GetFactoryClass() const override
	{
		return UPaperTileMap::StaticClass();
	}

private:

	virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;

	virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;
};
