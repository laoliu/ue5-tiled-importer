// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions/AssetTypeActions_ClassTypeBase.h"

/**
 * 
 */
class FTileSetAssetTypeActions : public FAssetTypeActions_Base
{
protected:
	virtual bool IsMacroFactory() const { return false; }

public:
	FTileSetAssetTypeActions(
		EAssetTypeCategories::Type InAssetCategory = EAssetTypeCategories::Misc
	);
	~FTileSetAssetTypeActions();

	UClass* GetSupportedClass() const override;
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	uint32 GetCategories() override;

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
