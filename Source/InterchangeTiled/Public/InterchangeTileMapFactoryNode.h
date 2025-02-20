// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/InterchangeFactoryBaseNode.h"
#include "PaperTileMap.h"

#include "InterchangeTileMapFactoryNode.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileMapFactoryNode : public UInterchangeFactoryBaseNode
{
	GENERATED_BODY()

public:

	virtual FString GetTypeName() const override
	{
		return TEXT("TileMapFactoryNode");
	}

	virtual UClass* GetObjectClass() const override
	{
		return UPaperTileMap::StaticClass();
	}
	
};
