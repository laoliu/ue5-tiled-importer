// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace InterchangeTiled
{
	/**
	* Generate the absolute path of a file path that's referenced
	* in an external file.
	* 
	* @param Path - the file path as it's written in the reference file
	* @param RelativeTo - the file path to the reference file
	* 
	* @return the absolute path corresponding to the one referenced in the external file
	*/
	FString GetAbsolutePath(FString Path, FString RelativeTo);

	/**
	* Get absolute texture path from relative or absolute path and reference file path.
	*/
	inline FString GetAbsoluteTexturePath(const FString& TextureFilename, const FString& TmxOrTsxFilePath)
	{
		if (FPaths::IsRelative(TextureFilename))
		{
			FString Dir = FPaths::GetPath(TmxOrTsxFilePath);
			return FPaths::Combine(Dir, TextureFilename);
		}
		return TextureFilename;
	}
}