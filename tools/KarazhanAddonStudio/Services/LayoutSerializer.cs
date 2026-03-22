using System.Text.Json;
using KarazhanAddonStudio.Models;

namespace KarazhanAddonStudio.Services;

public static class LayoutSerializer
{
    private static readonly JsonSerializerOptions Options = new()
    {
        WriteIndented = true
    };

    public static void Save(string path, LayoutDocument document)
    {
        var json = JsonSerializer.Serialize(document, Options);
        File.WriteAllText(path, json);
    }

    public static LayoutDocument Load(string path)
    {
        var json = File.ReadAllText(path);
        return JsonSerializer.Deserialize<LayoutDocument>(json, Options)
            ?? new LayoutDocument();
    }
}
