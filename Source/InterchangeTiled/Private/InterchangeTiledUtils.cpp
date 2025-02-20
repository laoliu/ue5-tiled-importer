#include "InterchangeTiledUtils.h"

FString InterchangeTiled::GetAbsolutePath(FString Path, FString RelativeTo)
{
	const FString FullPathToRelative = FPaths::ConvertRelativePathToFull(RelativeTo);
	FString WorkingPath = Path;
	int LevelsAbove = 0, NextParentIndex = WorkingPath.Find("../");

	while (NextParentIndex != -1)
	{
		LevelsAbove++;
		WorkingPath = WorkingPath.RightChop(3);
		NextParentIndex = WorkingPath.Find("../");
	}

	FString AbsolutePath = FPaths::GetPath(FullPathToRelative);

	for (int Level = 0; Level < LevelsAbove; Level++)
	{
		int YoungestDescendentIndex;
		AbsolutePath.FindLastChar('/', YoungestDescendentIndex);
		AbsolutePath = AbsolutePath.Left(YoungestDescendentIndex);
	}

	return AbsolutePath + "/" + WorkingPath;
}
