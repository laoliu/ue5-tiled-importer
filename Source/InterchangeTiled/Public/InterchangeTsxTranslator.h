// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterchangeDispatcher.h"
#include "InterchangeTranslatorBase.h"
#include "InterchangeTsxTranslator.generated.h"

UCLASS(BlueprintType, EditInlineNew, MinimalAPI)
class UInterchangeTsxTranslatorSettings : public UInterchangeTranslatorSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "TSX Translator")
	bool bKeepTsxNamespace = false;
};

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTsxTranslator : public UInterchangeTranslatorBase
{
	GENERATED_BODY()
	
public:

	// inherited methods
	virtual EInterchangeTranslatorAssetType GetSupportedAssetTypes() const override;
	virtual TArray<FString> GetSupportedFormats() const override;
	virtual EInterchangeTranslatorType GetTranslatorType() const override;
	virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;

private:
	bool TranslateTexture(UInterchangeBaseNodeContainer& BaseNodeContainer) const;
};
