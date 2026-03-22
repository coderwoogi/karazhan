namespace KarazhanAddonStudio.Models;

public sealed class LayoutDocument
{
    public string Name { get; set; } = "KarazhanBonusMission";
    public int Width { get; set; } = 344;
    public int Height { get; set; } = 246;
    public List<WidgetModel> Widgets { get; set; } = [];
}
