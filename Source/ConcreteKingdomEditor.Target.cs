using UnrealBuildTool;
public class ConcreteKingdomEditorTarget : TargetRules {
    public ConcreteKingdomEditorTarget(TargetInfo Target) : base(Target) {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("ConcreteKingdom");
    }
}