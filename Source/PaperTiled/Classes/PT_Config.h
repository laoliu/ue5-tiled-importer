// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PT_Config.generated.h"

/**
 * 
 */
UCLASS()
class PAPERTILED_API UPT_Config : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// the path where Tiled assets live for this particular configuration
	UPROPERTY(EditAnywhere)
	FDirectoryPath path;
};

