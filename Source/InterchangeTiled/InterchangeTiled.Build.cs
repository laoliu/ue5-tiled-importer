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
                "InterchangeNodes"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "InterchangeCommonParser",
                "InterchangeMessages",
                "Paper2D",
            }
        );
    }
}
