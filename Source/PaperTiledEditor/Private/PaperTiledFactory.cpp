// Fill out your copyright notice in the Description page of Project Settings.


#include "PaperTiledFactory.h"
#include "PT_Tileset.h"

UPaperTiledFactory::UPaperTiledFactory()
{
	SupportedClass = UPT_Tileset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPaperTiledFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	return NewObject<UPT_Tileset>(
		InParent,
		Class,
		Name,
		Flags,
		Context
	);
}