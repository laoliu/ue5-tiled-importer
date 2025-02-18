using UnrealBuildTool;

public class InterchangeTiled: ModuleRules
{
    public InterchangeTiled(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InterchangeCore",
                "InterchangeCommon",
                "InterchangeDispatcher",
                "InterchangeEngine",
                "InterchangeFactoryNodes",
                "InterchangeNodes",
                "InterchangePipelines",
                "UnrealEd",
                "XmlParser",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AssetTools",
                "InterchangeCommonParser",
                "InterchangeMessages",
                "Paper2D",
            }
        );
    }
}
