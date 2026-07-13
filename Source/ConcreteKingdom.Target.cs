using UnrealBuildTool;
public class ConcreteKingdomTarget : TargetRules {
    public ConcreteKingdomTarget(TargetInfo Target) : base(Target) {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("ConcreteKingdom");
    }
}