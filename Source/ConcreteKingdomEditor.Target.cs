using UnrealBuildTool;

public class ConcreteKingdomEditorTarget : TargetRules
{
    public ConcreteKingdomEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
    }
}