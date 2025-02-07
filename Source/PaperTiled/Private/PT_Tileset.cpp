// Fill out your copyright notice in the Description page of Project Settings.


#include "PT_Tileset.h"
#include "Logging/StructuredLog.h"
#include "Modules/ModuleManager.h"
#include "XmlFile.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "AssetRegistry/AssetRegistryModule.h"

#if WITH_EDITOR
void UPT_Tileset::PostEditChangeProperty(
	struct FPropertyChangedEvent& PropertyChangedEvent
)
{
	FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UPT_Tileset, TiledTileset) && !TiledTileset.FilePath.IsEmpty())
	{
		FXmlFile TileSetFile(TiledTileset.FilePath);
		FXmlNode* RootNode = TileSetFile.GetRootNode();

		if (RootNode->GetTag() != "tileset") {
			return;
		}

		FString TileSetName = RootNode->GetAttribute("name");
		FString TileWidth = RootNode->GetAttribute("tilewidth");
		FString TileHeight = RootNode->GetAttribute("tileheight");
		FString TileSpacing = RootNode->GetAttribute("spacing");
		FString ImageMargin = RootNode->GetAttribute("margin");
		FString TileCount = RootNode->GetAttribute("tilecount");
		FString ColumnCount = RootNode->GetAttribute("columns");

		SetTileSize(FIntPoint(FCString::Atoi(*TileWidth), FCString::Atoi(*TileHeight)));
		SetMargin(FCString::Atoi(*ImageMargin));
		SetPerTileSpacing(FCString::Atoi(*TileSpacing));

		ImportTexture(TileSetName);
	}
}
#endif

void UPT_Tileset::ImportTexture(FString TileSetName)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString MyPath = GetPathName();
	int LastSlashIndex;

	MyPath.FindLastChar('/', LastSlashIndex);
	FString AssetPath = MyPath.Left(LastSlashIndex + 1);
	FString TexturePath = AssetPath + TileSetName;

	UE_LOGFMT(LogTemp, Warning, "Texture Path: {Path}", TexturePath);

	FString Name, PackageName;

	AssetToolsModule.Get().CreateUniqueAssetName(
		TexturePath,
		TEXT(""),
		PackageName,
		Name
	);
}