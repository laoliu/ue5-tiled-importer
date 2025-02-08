// Fill out your copyright notice in the Description page of Project Settings.


#include "PT_Tileset.h"
#include "Logging/StructuredLog.h"
#include "Modules/ModuleManager.h"
#include "XmlFile.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Factories/TextureFactory.h"
#include "UObject/SavePackage.h"
#include "IContentBrowserSingleton.h"
#include "ImageUtils.h"
#include "ObjectTools.h"

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
		const FXmlNode* ImageNode = RootNode->GetFirstChildNode();

		if (RootNode->GetTag() != "tileset" || ImageNode->GetTag() != "image") {
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
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString AssetPath = FPaths::GetPath(GetPathName());
	FString ImageFileName = FPaths::GetBaseFilename(ImageSource);
	FString ImageFileExtension = FPaths::GetExtension(ImageSource, true);

	UTexture2D* ExistingTexture = GetExistingTextureAsset(AssetRegistryModule, AssetPath, ImageFileName);

	if (ExistingTexture)
	{
		UE_LOGFMT(LogTemp, Warning, "A 2D Texture asset already exists with the same name: {Name}", ImageFileName);
		return ExistingTexture;
	}

	const FString ObjectFileName = ImageFileName + '.' + ImageFileName;
	const FString ObjectPath = FPaths::Combine(AssetPath, ObjectFileName);
	const FString PackageName = FPackageName::ObjectPathToPackageName(ObjectPath);
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(
		PackageName,
		FPackageName::GetAssetPackageExtension()
	);

	UE_LOGFMT(LogTemp, Display, "Creating 2D Texture asset {PackageName}", PackageName);

	UPackage* const NewPackage = CreatePackage(*PackageName);
	NewPackage->FullyLoad();

	FImage ImageInfo;
	FImageUtils::LoadImage(*ImageSource, ImageInfo);
	UTexture2D* NewTexture = FImageUtils::CreateTexture2DFromImage(ImageInfo);
	NewTexture->Rename(nullptr, NewPackage, REN_DontCreateRedirectors);
	NewTexture->SetFlags(NewTexture->GetFlags() | RF_Public | RF_Standalone);
	NewTexture->AtomicallyClearFlags(RF_Transient);
	NewTexture->Rename(*ImageFileName, nullptr, REN_DontCreateRedirectors);
	
	FObjectThumbnail NewThumbnail;
	ThumbnailTools::RenderThumbnail(
		NewTexture,
		NewTexture->GetSizeX(),
		NewTexture->GetSizeY(),
		ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
		NULL,
		&NewThumbnail
	);
	ThumbnailTools::CacheThumbnail(NewTexture->GetFullName(), &NewThumbnail, NewPackage);

	NewPackage->ClearDirtyFlag();
	AssetRegistry.AssetCreated(NewTexture);


	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.bForceByteSwapping = true;
	SaveArgs.bWarnOfLongFilename = true;

	if (!UPackage::SavePackage(
		NewPackage,
		NewTexture,
		*PackageFileName,
		SaveArgs
	))
	{
		UE_LOGFMT(LogTemp, Error, "Failed to save Asset: {FileName}", *ImageFileName);
		return NULL;
	}

	TArray<UObject*> Objects;
	Objects.Add(NewTexture);
	ContentBrowserModule.Get().SyncBrowserToAssets(Objects);

	return NewTexture;
}