#include "InterchangeTileMapFactory.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "InterchangePaperImporterSettings.h"
#include "InterchangeSourceData.h"
#include "InterchangeTiledModule.h"
#include "PaperTileLayer.h"
#include "PaperTileSet.h"
#include "XmlFile.h"
#include "Misc/Base64.h"
#include "Misc/Compression.h"

UInterchangeFactoryBase::FImportAssetResult UInterchangeTileMapFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInterchangeTileMapFactory::BeginImportAsset_GameThread);

	FImportAssetResult ImportAssetResult;
	UPaperTileMap* TileMap = nullptr;

	if (!Arguments.AssetNode)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_AssetNodeNull", "Asset node parameter is null."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	const UClass* TileMapClass = Arguments.AssetNode->GetObjectClass();

	if (!TileMapClass || !TileMapClass->IsChildOf(UPaperTileMap::StaticClass()))
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_NodeClassMissmatch", "Asset node parameter class doesn't derive from UPaperTileMap."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	UObject* ExistingAsset = Arguments.ReimportObject;

	// even if not flagged as a re-import, try to load the asset in case
	// it already exists
	if (!ExistingAsset)
	{
		FSoftObjectPath ReferenceObject;
		if (Arguments.AssetNode->GetCustomReferenceObject(ReferenceObject))
		{
			ExistingAsset = ReferenceObject.TryLoad();
		}
	}

	if (!ExistingAsset)
	{
		TileMap = NewObject<UPaperTileMap>(
			Arguments.Parent,
			TileMapClass,
			*Arguments.AssetName,
			RF_Public | RF_Standalone
		);
	}

	if (!TileMap)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_TileMapCreateFail", "Tile Map creation failed."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = TileMap;

	return ImportAssetResult;
}

void UInterchangeTileMapFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	// 1. 获取所有TilesetFactoryNodeUids
	FString UidsStr;
	TArray<FString> TilesetFactoryNodeUids;
	Arguments.FactoryNode->GetAttribute("TilesetFactoryNodeUids", UidsStr);
	UidsStr.ParseIntoArray(TilesetFactoryNodeUids, TEXT(","), true);

	// 2. 查找/创建所有UPaperTileSet对象，并保存firstgid
	struct FParsedTileSet {
		UPaperTileSet* TileSet;
		int32 FirstGid;
	};
	TArray<FParsedTileSet> ParsedTileSets;
	for (const FString& Uid : TilesetFactoryNodeUids)
	{
		UInterchangeFactoryBaseNode* FactoryNode = FindFactoryNodeByUid(Uid, Arguments.NodeContainer);
		FString FirstGidStr;
		FactoryNode->GetAttribute("FirstGid", FirstGidStr);
		int32 FirstGid = FCString::Atoi(*FirstGidStr);
		// 查找/创建UPaperTileSet对象（此处假设已实现FindOrCreateTileSet）
		UPaperTileSet* TileSet = FindOrCreateTileSet(FactoryNode);
		ParsedTileSets.Add({TileSet, FirstGid});
	}

	// 3. 解析TMX文件，获取地图和layer信息
	FXmlFile TileMapFile(Arguments.SourceData->GetFilename());
	FXmlNode* RootNode = TileMapFile.GetRootNode();
	FString MapHeight = RootNode->GetAttribute("height");
	FString MapWidth = RootNode->GetAttribute("width");
	FString TileHeight = RootNode->GetAttribute("tileheight");
	FString TileWidth = RootNode->GetAttribute("tilewidth");

	UPaperTileMap* TileMap = Cast<UPaperTileMap>(Arguments.ImportedObject);
	TileMap->MapHeight = FCString::Atoi(*MapHeight);
	TileMap->MapWidth = FCString::Atoi(*MapWidth);
	TileMap->TileHeight = FCString::Atoi(*TileHeight);
	TileMap->TileWidth = FCString::Atoi(*TileWidth);

	TArray<FXmlNode*> ChildrenNodes = RootNode->GetChildrenNodes();
	for (int k = ChildrenNodes.Num() - 1; k >= 0; --k)
	{
		FXmlNode* ChildNode = ChildrenNodes[k];
		if (ChildNode->GetTag() == "layer")
		{
			FString LayerName = ChildNode->GetAttribute("name");
			int LayerHeight = FCString::Atoi(*ChildNode->GetAttribute("height"));
			int LayerWidth = FCString::Atoi(*ChildNode->GetAttribute("width"));
			const FXmlNode* LayerDataNode = ChildNode->FindChildNode("data");
			FString LayerData = LayerDataNode ? LayerDataNode->GetContent() : "";
			FString Encoding = LayerDataNode ? LayerDataNode->GetAttribute("encoding") : "";
			FString Compression = LayerDataNode ? LayerDataNode->GetAttribute("compression") : "";
			FString OpacityStr = ChildNode->GetAttribute("opacity");
			float Opacity = OpacityStr.IsEmpty() ? 1.0f : FCString::Atof(*OpacityStr);

			TArray<int32> TileGids;

			if (Encoding == "base64")
			{
				FString CleanLayerData = LayerData.Replace(TEXT("\"\\n\""), TEXT("\"")).Replace(TEXT(" "), TEXT("\"")).Replace(TEXT("\"\\r\""), TEXT("\"")).TrimStartAndEnd();
				TArray<uint8> DecodedData;
				FBase64::Decode(CleanLayerData, DecodedData);

				TArray<uint8> UncompressedData;
				int32 UncompressedSize = LayerWidth * LayerHeight * 4;
				UncompressedData.SetNumUninitialized(UncompressedSize);

				bool bSuccess = FCompression::UncompressMemory(
					NAME_Zlib,
					UncompressedData.GetData(),
					UncompressedSize,
					DecodedData.GetData(),
					DecodedData.Num()
				);
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("Zlib decompression failed!"));
				}

				// 打印解压后的数据（前32字节，十六进制）
				FString DebugHex;
				for (int i = 0; i < FMath::Min(UncompressedData.Num(), 32); ++i)
				{
					DebugHex += FString::Printf(TEXT("%02X "), UncompressedData[i]);
				}
				UE_LOG(LogTemp, Warning, TEXT("UncompressedData (hex): %s"), *DebugHex);

				// 解析为int32数组
				TileGids.SetNum(LayerWidth * LayerHeight);
				FMemory::Memcpy(TileGids.GetData(), UncompressedData.GetData(), UncompressedSize);

				// 打印TileGids内容（前32个int32）
				FString DebugTileGids;
				for (int i = 0; i < FMath::Min(TileGids.Num(), 32); ++i)
				{
					DebugTileGids += FString::Printf(TEXT("%d "), TileGids[i]);
				}
				UE_LOG(LogTemp, Warning, TEXT("TileGids (int32): %s"), *DebugTileGids);
			}
			else
			{
				// 其他编码方式，保持原有逻辑
				TArray<FString> TileGidStrs;
				LayerData.ParseIntoArray(TileGidStrs, TEXT(","), true);
				for (const FString& GidStr : TileGidStrs)
				{
					TileGids.Add(FCString::Atoi(*GidStr));
				}
			}

			UPaperTileLayer* Layer = NewObject<UPaperTileLayer>(TileMap, UPaperTileLayer::StaticClass());
			Layer->DestructiveAllocateMap(LayerWidth, LayerHeight);
			Layer->LayerName = FText::FromString(LayerName);
			//FLinearColor LayerColor = Layer->GetLayerColor();
			//LayerColor.A = Opacity;
			FLinearColor LayerColor = FLinearColor::White;
			LayerColor.A = FMath::Clamp<float>(Opacity, 0.0f, 1.0f);
			Layer->SetLayerColor(LayerColor);

			// 访问前检查
			for (int TileIndex = 0; TileIndex < LayerWidth * LayerHeight; TileIndex++)
			{
				if (TileIndex >= TileGids.Num()) break; // 防止越界
				int32 Gid = TileGids[TileIndex];
				if (Gid <= 0) continue; // 空白tile
				// 找到对应的tileset
				FParsedTileSet* UsedTileSet = nullptr;
				for (int i = ParsedTileSets.Num() - 1; i >= 0; --i)
				{
					if (Gid >= ParsedTileSets[i].FirstGid)
					{
						UsedTileSet = &ParsedTileSets[i];
						break;
					}
				}
				if (!UsedTileSet || !UsedTileSet->TileSet) continue;
				int32 LocalTileIndex = Gid - UsedTileSet->FirstGid;
				int TileX = TileIndex % LayerWidth;
				int TileY = TileIndex / LayerWidth;
				FPaperTileInfo TileInfo;
				TileInfo.TileSet = UsedTileSet->TileSet;
				TileInfo.PackedTileIndex = LocalTileIndex;
				Layer->SetCell(TileX, TileY, TileInfo);
			}
			TileMap->TileLayers.Add(Layer);
		}
	}
}

UInterchangeFactoryBaseNode* UInterchangeTileMapFactory::FindFactoryNodeByUid(const FString& Uid, UInterchangeBaseNodeContainer* NodeContainer)
{
    if (!NodeContainer) return nullptr;
    const UInterchangeBaseNode* BaseNode = NodeContainer->GetNode(Uid);
    return const_cast<UInterchangeFactoryBaseNode*>(Cast<UInterchangeFactoryBaseNode>(BaseNode));
}

UPaperTileSet* UInterchangeTileMapFactory::FindOrCreateTileSet(UInterchangeFactoryBaseNode* FactoryNode)
{
    if (!FactoryNode) return nullptr;
    FString AssetName;
    FactoryNode->GetAttribute("AssetName", AssetName);
    FString TileWidthStr, TileHeightStr;
    FactoryNode->GetAttribute("TileWidth", TileWidthStr);
    FactoryNode->GetAttribute("TileHeight", TileHeightStr);

    // 查找已存在的TileSet（通过AssetRegistry）
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetDataArray;
    FARCompiledFilter Filter;
    Filter.ClassPaths.Add(UPaperTileSet::StaticClass()->GetClassPathName());
    Filter.PackageNames.Add(FName(*AssetName));
    AssetRegistryModule.Get().GetAssets(Filter, AssetDataArray);
    if (AssetDataArray.Num() > 0)
    {
        UObject* ExistingAsset = AssetDataArray[0].GetAsset();
        UPaperTileSet* ExistingTileSet = Cast<UPaperTileSet>(ExistingAsset);
        if (ExistingTileSet)
        {
            return ExistingTileSet;
        }
    }

    // 如果没有则新建
    UPaperTileSet* TileSet = NewObject<UPaperTileSet>(GetTransientPackage(), UPaperTileSet::StaticClass(), *AssetName, RF_Public | RF_Standalone);
    TileSet->SetTileSize(FIntPoint(FCString::Atoi(*TileWidthStr), FCString::Atoi(*TileHeightStr)));

    // 设置Texture等其他属性
    FString TextureFilename;
    FactoryNode->GetAttribute("TextureFilename", TextureFilename);
    if (!TextureFilename.IsEmpty())
    {
        UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TextureFilename));
        if (Texture)
        {
			GetDefault<UInterchangePaperImporterSettings>()->ApplyTextureSettings(Texture);
            TileSet->SetTileSheetTexture(Texture);
        }
    }
    FString TileSpacingStr, ImageMarginStr;
    FactoryNode->GetAttribute("TileSpacing", TileSpacingStr);
    FactoryNode->GetAttribute("ImageMargin", ImageMarginStr);
    if (!TileSpacingStr.IsEmpty())
    {
        TileSet->SetPerTileSpacing(FCString::Atoi(*TileSpacingStr));
    }
    if (!ImageMarginStr.IsEmpty())
    {
        TileSet->SetMargin(FCString::Atoi(*ImageMarginStr));
    }
    // 可扩展：设置其他属性，如TileCount、Columns等
    return TileSet;
}
