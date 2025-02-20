#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInterchangeTiledImport, Log, All);

#define LOCTEXT_NAMESPACE "InterchangeTiled"

class FInterchangeTiled : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
