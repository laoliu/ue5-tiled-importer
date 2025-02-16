// Fill out your copyright notice in the Description page of Project Settings.


#include "PT_Tileset.h"
#include "Logging/StructuredLog.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Factories/TextureFactory.h"
#include "UObject/SavePackage.h"
#include "IContentBrowserSingleton.h"
#include "ImageUtils.h"
#include "ObjectTools.h"
#include "Rendering/Texture2DResource.h"
#include "AutomatedAssetImportData.h"

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
		const FXmlNode* ImageNode = RootNode->FindChildNode("image");

		if (RootNode->GetTag() != "tileset" || !ImageNode) {
			return;
		}

		FString TileSetName = RootNode->GetAttribute("name");
		FString TileWidth = RootNode->GetAttribute("tilewidth");
		FString TileHeight = RootNode->GetAttribute("tileheight");
		FString TileSpacing = RootNode->GetAttribute("spacing");
		FString ImageMargin = RootNode->GetAttribute("margin");
		FString TileCount = RootNode->GetAttribute("tilecount");
		FString ColumnCount = RootNode->GetAttribute("columns");
		FString ImageSource = ImageNode->GetAttribute("source");


		SetTileSize(FIntPoint(FCString::Atoi(*TileWidth), FCString::Atoi(*TileHeight)));
		SetMargin(FCString::Atoi(*ImageMargin));
		SetPerTileSpacing(FCString::Atoi(*TileSpacing));

		UTexture2D* Texture = FPaths::IsRelative(ImageSource) ?
			ImportTexture(ImageSource, TiledTileset.FilePath) :
			ImportTexture(ImageSource);

		SetTileSheetTexture(Texture);
		Super::PostEditChangeProperty(PropertyChangedEvent);

		PopulateTileMetadata(RootNode->GetChildrenNodes());
	}
}
#endif

// determine whether a 2D texture asset exists with the given name
// at the given package path
UTexture2D* UPT_Tileset::GetExistingTextureAsset(
	FAssetRegistryModule& AssetRegistryModule,
	FString AssetPath,
	FString ImageFileName
)
{

	const UClass* Class = UTexture2D::StaticClass();

	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.PackagePaths.Add(FName(*AssetPath));
	Filter.ClassPaths.Add(Class->GetClassPathName());

	AssetRegistryModule.Get().GetAssets(
		Filter,
		AssetData
	);

	for (int i = 0; i < AssetData.Num(); i++)
	{
		FAssetData Asset = AssetData[i];

		if (Asset.AssetName == ImageFileName)
		{
			UObject* AssetObject = Asset.GetAsset();
			UTexture2D* Texture = Cast<UTexture2D>(AssetObject);

			return Texture;
		}
	}

	return NULL;
}

// Create a new texture asset in the same directory as this tile set.
// Calculate the path to the image file using the relative path from
// this asset to the Tiled tile set file and the relative path of the
// image source provided in the Tiled tile set file.
UTexture2D* UPT_Tileset::ImportTexture(FString RelativeImageSource, FString InTileSetPath)
{
	const FString FullTileSetPath = FPaths::ConvertRelativePathToFull(InTileSetPath);
		
	UE_LOGFMT(LogTemp, Display, "Importing texture for tile set located at {Path}", FullTileSetPath);

	FString WorkingImageSource = RelativeImageSource;
	int LevelsAbove = 0, NextParentIndex = WorkingImageSource.Find("../");

	while (NextParentIndex != -1)
	{
		LevelsAbove++;
		WorkingImageSource = WorkingImageSource.RightChop(3);
		NextParentIndex = WorkingImageSource.Find("../");
	}

	FString TileSetPath = FPaths::GetPath(FullTileSetPath);

	for (int Level = 0; Level < LevelsAbove; Level++)
	{
		int YoungestDescendentIndex;
		TileSetPath.FindLastChar('/', YoungestDescendentIndex);

		if (YoungestDescendentIndex == -1)
		{
			UE_LOG(LogTemp, Error, TEXT("Couldn't process relative paths."));
			break;
		}

		TileSetPath = TileSetPath.Left(YoungestDescendentIndex);
	}

	FString ImagePath = TileSetPath + "/" + WorkingImageSource;

	UE_LOGFMT(LogTemp, Display, "Image file is {ImagePath}", ImagePath);

	return ImportTexture(ImagePath);
}

// Create a new texture asset 
UTexture2D* UPT_Tileset::ImportTexture(FString ImageSource)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	FString AssetPath = FPaths::GetPath(GetPathName());
	FString ImageFileName = FPaths::GetBaseFilename(ImageSource);
	FString PackageName = FPaths::Combine(AssetPath, ImageFileName);

	UE_LOGFMT(LogTemp, Display, "Creating 2D Texture asset {PackageName}", PackageName);

	TArray<FString> Filenames;
	Filenames.Add(ImageSource);

	UAutomatedAssetImportData* ImportSettings = NewObject<UAutomatedAssetImportData>(UAutomatedAssetImportData::StaticClass());
	ImportSettings->bReplaceExisting = false;
	ImportSettings->DestinationPath = AssetPath;
	ImportSettings->Factory = NewObject<UTextureFactory>(UTextureFactory::StaticClass());
	ImportSettings->FactoryName = UTextureFactory::StaticClass()->GetName();
	ImportSettings->Filenames = Filenames;

	TArray<UObject*> NewAssets = AssetToolsModule.Get().ImportAssetsAutomated(ImportSettings);
	UObject* NewAsset = NewAssets[0];
	UTexture2D* NewTexture = Cast<UTexture2D>(NewAsset);

	return NewTexture;
}

void UPT_Tileset::PopulateTileMetadata(TArray<FXmlNode*> TilesetNodes)
{

	for (const FXmlNode* Node : TilesetNodes)
	{
		if (Node->GetTag() == "tile")
		{
			const uint32 TileId = FCString::Atoi(*Node->GetAttribute("id"));
			const FXmlNode* ObjectGroupNode = Node->FindChildNode("objectgroup");

			if (!ObjectGroupNode)
			{
				continue;
			}

			const FXmlNode* ObjectNode = ObjectGroupNode->FindChildNode("object");

			if (!ObjectNode)
			{
				continue;
			}

			FVector2D RectanglePosition(
				FCString::Atoi(*ObjectNode->GetAttribute("x")),
				FCString::Atoi(*ObjectNode->GetAttribute("y"))
			);

			FVector2D RectangleSize(
				FCString::Atod(*ObjectNode->GetAttribute("width")),
				FCString::Atod(*ObjectNode->GetAttribute("height"))
			);

			FSpriteGeometryCollection CollisionData;
			CollisionData.AddRectangleShape(
				RectanglePosition,
				RectangleSize
			);
			
			FPaperTileMetadata TileMetadata;
			TileMetadata.CollisionData = CollisionData;

			FPaperTileMetadata* CurrentTileMetadata = GetMutableTileMetadata(TileId);
			*CurrentTileMetadata = TileMetadata;
		}
	}
}