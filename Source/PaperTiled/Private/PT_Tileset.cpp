// Fill out your copyright notice in the Description page of Project Settings.


#include "PT_Tileset.h"
#include "Logging/StructuredLog.h"

#if WITH_EDITOR
void UPT_Tileset::PostEditChangeProperty(
	struct FPropertyChangedEvent& PropertyChangedEvent
)
{
	FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UPT_Tileset, TiledTileset))
	{
		UE_LOGFMT(LogTemp, Warning, "Property Changed: {Name}", PropertyName);
	}
}
#endif