// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PT_Config.generated.h"

// Information to assist in auto-importing Tiled assets into Paper2D assets
UCLASS()
class PAPERTILED_API UPT_Config : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// A directory with Tiled assets
	UPROPERTY(EditAnywhere)
	FDirectoryPath TiledImportFolder;

	// A directory within the project where Paper2D assets will be generated
	UPROPERTY(EditAnywhere)
	FDirectoryPath DestinationFolder;
};

