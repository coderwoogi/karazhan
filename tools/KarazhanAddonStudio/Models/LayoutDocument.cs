namespace KarazhanAddonStudio.Models;

public sealed class LayoutDocument
{
    [System.ComponentModel.DisplayName("문서 이름")]
    public string Name { get; set; } = "KarazhanBonusMission";

    [System.ComponentModel.DisplayName("루트 너비")]
    public int Width { get; set; } = 344;

    [System.ComponentModel.DisplayName("루트 높이")]
    public int Height { get; set; } = 246;

    public List<WidgetModel> Widgets { get; set; } = [];
}
