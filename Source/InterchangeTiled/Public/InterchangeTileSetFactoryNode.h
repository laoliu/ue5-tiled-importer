// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/InterchangeFactoryBaseNode.h"
#include "PaperTileSet.h"
#include "InterchangeTileSetFactoryNode.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileSetFactoryNode : public UInterchangeFactoryBaseNode
{
	GENERATED_BODY()

public:

	virtual FString GetTypeName() const override
	{
		return TEXT("TileSetFactoryNode");
	}

	virtual UClass* GetObjectClass() const override
	{
		return UPaperTileSet::StaticClass();
	}
};
