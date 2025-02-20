#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "InterchangeTiledFactory.h"
#include "PaperTileSet.h"
#include "XmlFile.h"

#include "InterchangeTileSetFactory.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileSetFactory : public UInterchangeTiledFactory
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

	virtual void BuildObject_GameThread(const FSetupObjectParams& Arguments, bool& OutPostEditChangeCalled) override;

	virtual UTexture2D* LoadOrCreateTextureAsset(
		UInterchangeFactoryBaseNode* FactoryNode,
		FString PackagePath
	);

	virtual void PopulateTileMetadata(TArray<FXmlNode*> TilesetNodes, UPaperTileSet* TileSet);
};
