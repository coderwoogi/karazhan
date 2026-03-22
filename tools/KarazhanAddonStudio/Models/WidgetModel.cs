using System.ComponentModel;
using System.Drawing;

namespace KarazhanAddonStudio.Models;

public sealed class WidgetModel
{
    [Browsable(false)]
    public Guid Id { get; set; } = Guid.NewGuid();

    [Category("기본")]
    [DisplayName("이름")]
    public string Name { get; set; } = "Widget";

    [Category("기본")]
    [DisplayName("종류")]
    public WidgetKind Kind { get; set; }

    [Category("위치")]
    [DisplayName("X")]
    public int Left { get; set; } = 16;

    [Category("위치")]
    [DisplayName("Y")]
    public int Top { get; set; } = 16;

    [Category("크기")]
    [DisplayName("너비")]
    public int Width { get; set; } = 120;

    [Category("크기")]
    [DisplayName("높이")]
    public int Height { get; set; } = 28;

    [Category("내용")]
    [DisplayName("텍스트")]
    public string Text { get; set; } = "새 위젯";

    [Category("색상")]
    [DisplayName("배경색")]
    public string BackColor { get; set; } = "#1E1E28";

    [Category("색상")]
    [DisplayName("글자색")]
    public string ForeColor { get; set; } = "#F0F0F0";

    [Category("색상")]
    [DisplayName("테두리색")]
    public string BorderColor { get; set; } = "#5A6578";

    [Category("표현")]
    [DisplayName("텍스처 경로")]
    public string TexturePath { get; set; } =
        "Interface\\DialogFrame\\UI-DialogBox-Background";

    [Category("표현")]
    [DisplayName("정렬")]
    public ContentAlignment TextAlign { get; set; } =
        ContentAlignment.MiddleLeft;

    [Category("표현")]
    [DisplayName("글꼴 크기")]
    public int FontSize { get; set; } = 11;

    [Category("표현")]
    [DisplayName("투명도")]
    public int Alpha { get; set; } = 255;

    [Category("값")]
    [DisplayName("현재값")]
    public int Value { get; set; } = 50;

    [Category("값")]
    [DisplayName("최대값")]
    public int MaxValue { get; set; } = 100;

    [Category("값")]
    [DisplayName("체크됨")]
    public bool Checked { get; set; }

    [Category("값")]
    [DisplayName("플레이스홀더")]
    public string Placeholder { get; set; } = string.Empty;

    [Category("동작")]
    [DisplayName("보이기")]
    public bool Visible { get; set; } = true;

    [Category("동작")]
    [DisplayName("툴팁")]
    public string Tooltip { get; set; } = string.Empty;

    [Category("데이터")]
    [DisplayName("바인딩 키")]
    public string BindingKey { get; set; } = string.Empty;

    public WidgetModel Clone()
    {
        return (WidgetModel)MemberwiseClone();
    }
}
