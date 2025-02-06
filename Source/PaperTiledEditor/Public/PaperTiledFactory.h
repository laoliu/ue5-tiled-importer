// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PaperTiledFactory.generated.h"

// asset factory to generate Paper Tiled tile set assets
UCLASS()
class PAPERTILEDEDITOR_API UTileSetAssetFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UTileSetAssetFactory();
	UObject* FactoryCreateNew(
		UClass* Class,
		UObject* InParent,
		FName Name,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn
	);
};