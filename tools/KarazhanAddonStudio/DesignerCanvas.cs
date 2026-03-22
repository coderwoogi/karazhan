using System.Drawing.Drawing2D;
using KarazhanAddonStudio.Models;

namespace KarazhanAddonStudio;

public sealed class DesignerCanvas : Panel
{
    public event EventHandler<WidgetModel?>? WidgetSelected;
    public event EventHandler? LayoutChanged;

    private readonly Dictionary<WidgetModel, Control> _bindings = [];
    private readonly ToolTip _tooltip = new();
    private WidgetModel? _selected;
    private Point _dragStart;
    private Point _controlStart;
    private bool _dragging;

    public DesignerCanvas()
    {
        AllowDrop = true;
        BackColor = Color.FromArgb(20, 23, 31);
        BorderStyle = BorderStyle.FixedSingle;
        DoubleBuffered = true;
        AutoScroll = true;
        DragEnter += OnDragEnterCanvas;
        DragDrop += OnDragDropCanvas;
        MouseClick += (_, _) => ClearSelection();
    }

    public WidgetModel? SelectedWidget => _selected;

    public void LoadDocument(LayoutDocument document)
    {
        Controls.Clear();
        _bindings.Clear();
        _selected = null;
        Width = document.Width;
        Height = document.Height;

        foreach (var widget in document.Widgets)
            AddWidget(widget);

        Invalidate();
    }

    public void RefreshWidget(WidgetModel widget)
    {
        if (!_bindings.TryGetValue(widget, out var control))
            return;

        ApplyWidget(control, widget);
        control.Invalidate();
        Invalidate();
    }

    public WidgetModel AddNewWidget(WidgetKind kind, Point location)
    {
        var widget = new WidgetModel
        {
            Kind = kind,
            Name = GetDefaultName(kind),
            Text = GetDefaultText(kind),
            Left = Math.Max(8, location.X),
            Top = Math.Max(8, location.Y),
            Width = GetDefaultWidth(kind),
            Height = GetDefaultHeight(kind)
        };

        ConfigureDefaults(widget);
        AddWidget(widget);
        SelectWidget(widget);
        LayoutChanged?.Invoke(this, EventArgs.Empty);
        return widget;
    }

    public void RemoveWidget(WidgetModel widget)
    {
        if (!_bindings.TryGetValue(widget, out var control))
            return;

        Controls.Remove(control);
        _bindings.Remove(widget);
        _selected = null;
        WidgetSelected?.Invoke(this, null);
        Invalidate();
        LayoutChanged?.Invoke(this, EventArgs.Empty);
    }

    public void RemoveSelectedWidget()
    {
        if (_selected is null)
            return;

        RemoveWidget(_selected);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);

        using var pen = new Pen(Color.FromArgb(34, 52, 64));
        pen.DashStyle = DashStyle.Dot;

        for (var x = 0; x < Width; x += 16)
            e.Graphics.DrawLine(pen, x, 0, x, Height);

        for (var y = 0; y < Height; y += 16)
            e.Graphics.DrawLine(pen, 0, y, Width, y);

        if (_selected is null || !_bindings.TryGetValue(_selected, out var control))
            return;

        var bounds = control.Bounds;
        bounds.Inflate(3, 3);
        using var selectedPen = new Pen(Color.FromArgb(255, 209, 102), 2);
        e.Graphics.DrawRectangle(selectedPen, bounds);
    }

    private void AddWidget(WidgetModel widget)
    {
        var control = CreateControlFor(widget);
        _bindings[widget] = control;
        Controls.Add(control);
        ApplyWidget(control, widget);
        HookWidgetControl(control, widget);
    }

    private static string GetDefaultName(WidgetKind kind)
    {
        return kind switch
        {
            WidgetKind.Label => "LabelWidget",
            WidgetKind.Panel => "PanelWidget",
            WidgetKind.Frame => "FrameWidget",
            WidgetKind.Button => "ButtonWidget",
            WidgetKind.ProgressBar => "ProgressWidget",
            WidgetKind.StatusBar => "StatusBarWidget",
            WidgetKind.Texture => "TextureWidget",
            WidgetKind.Icon => "IconWidget",
            WidgetKind.EditBox => "EditBoxWidget",
            WidgetKind.CheckBox => "CheckBoxWidget",
            WidgetKind.Slider => "SliderWidget",
            _ => "Widget"
        };
    }

    private static string GetDefaultText(WidgetKind kind)
    {
        return kind switch
        {
            WidgetKind.Label => "새 텍스트",
            WidgetKind.Panel => string.Empty,
            WidgetKind.Frame => "프레임 제목",
            WidgetKind.Button => "새 버튼",
            WidgetKind.ProgressBar => string.Empty,
            WidgetKind.StatusBar => string.Empty,
            WidgetKind.Texture => string.Empty,
            WidgetKind.Icon => string.Empty,
            WidgetKind.EditBox => "입력 상자",
            WidgetKind.CheckBox => "옵션 사용",
            WidgetKind.Slider => "슬라이더",
            _ => "새 위젯"
        };
    }

    private static int GetDefaultWidth(WidgetKind kind)
    {
        return kind switch
        {
            WidgetKind.Label => 140,
            WidgetKind.Panel => 180,
            WidgetKind.Frame => 220,
            WidgetKind.Button => 120,
            WidgetKind.ProgressBar => 180,
            WidgetKind.StatusBar => 180,
            WidgetKind.Texture => 180,
            WidgetKind.Icon => 48,
            WidgetKind.EditBox => 180,
            WidgetKind.CheckBox => 140,
            WidgetKind.Slider => 200,
            _ => 120
        };
    }

    private static int GetDefaultHeight(WidgetKind kind)
    {
        return kind switch
        {
            WidgetKind.Label => 24,
            WidgetKind.Panel => 90,
            WidgetKind.Frame => 120,
            WidgetKind.Button => 28,
            WidgetKind.ProgressBar => 18,
            WidgetKind.StatusBar => 18,
            WidgetKind.Texture => 96,
            WidgetKind.Icon => 48,
            WidgetKind.EditBox => 28,
            WidgetKind.CheckBox => 24,
            WidgetKind.Slider => 36,
            _ => 28
        };
    }

    private static void ConfigureDefaults(WidgetModel widget)
    {
        switch (widget.Kind)
        {
        case WidgetKind.Panel:
            widget.Text = string.Empty;
            widget.BackColor = "#202634";
            break;
        case WidgetKind.Frame:
            widget.BackColor = "#191D28";
            widget.BorderColor = "#92734B";
            break;
        case WidgetKind.ProgressBar:
        case WidgetKind.StatusBar:
            widget.BackColor = "#0E1118";
            widget.ForeColor = "#4F8BFF";
            break;
        case WidgetKind.Texture:
            widget.BackColor = "#2D3444";
            widget.ForeColor = "#DDDDDD";
            break;
        case WidgetKind.Icon:
            widget.TexturePath = "Interface\\ICONS\\INV_Misc_QuestionMark";
            widget.BackColor = "#141820";
            break;
        case WidgetKind.EditBox:
            widget.BackColor = "#0F1320";
            break;
        case WidgetKind.CheckBox:
            widget.BackColor = "#1E1E28";
            break;
        case WidgetKind.Slider:
            widget.Value = 35;
            break;
        }
    }

    private static Control CreateControlFor(WidgetModel widget)
    {
        return widget.Kind switch
        {
            WidgetKind.Panel => new Panel(),
            WidgetKind.Frame => new GroupBox(),
            WidgetKind.Button => new Button { FlatStyle = FlatStyle.Flat },
            WidgetKind.ProgressBar => new Panel(),
            WidgetKind.StatusBar => new Panel(),
            WidgetKind.Texture => new Panel(),
            WidgetKind.Icon => new Panel(),
            WidgetKind.EditBox => new TextBox(),
            WidgetKind.CheckBox => new CheckBox(),
            WidgetKind.Slider => new TrackBar
            {
                TickStyle = TickStyle.None,
                Minimum = 0,
                Maximum = 100
            },
            _ => new Label()
        };
    }

    private void ApplyWidget(Control control, WidgetModel widget)
    {
        control.Left = widget.Left;
        control.Top = widget.Top;
        control.Width = widget.Width;
        control.Height = widget.Height;
        control.Visible = widget.Visible;
        control.BackColor = ApplyAlpha(
            ParseColor(widget.BackColor, Color.FromArgb(30, 30, 40)),
            widget.Alpha);
        control.ForeColor = ParseColor(widget.ForeColor, Color.Gainsboro);
        control.Tag = widget;
        control.Font = new Font(
            Font.FontFamily,
            Math.Max(8, widget.FontSize),
            FontStyle.Regular);

        switch (control)
        {
        case Label label:
            label.Text = widget.Text;
            label.TextAlign = widget.TextAlign;
            break;
        case Button button:
            button.Text = widget.Text;
            button.FlatAppearance.BorderColor = ParseColor(
                widget.BorderColor,
                Color.Gray);
            break;
        case GroupBox groupBox:
            groupBox.Text = widget.Text;
            groupBox.ForeColor = control.ForeColor;
            break;
        case TextBox textBox:
            textBox.Text = widget.Text;
            textBox.BorderStyle = BorderStyle.FixedSingle;
            break;
        case CheckBox checkBox:
            checkBox.Text = widget.Text;
            checkBox.Checked = widget.Checked;
            break;
        case TrackBar trackBar:
            trackBar.Value = Math.Max(
                trackBar.Minimum,
                Math.Min(trackBar.Maximum, widget.Value));
            break;
        case Panel panel when widget.Kind == WidgetKind.ProgressBar ||
                              widget.Kind == WidgetKind.StatusBar ||
                              widget.Kind == WidgetKind.Texture ||
                              widget.Kind == WidgetKind.Icon:
            panel.Paint -= PaintWidgetPanel;
            panel.Paint += PaintWidgetPanel;
            break;
        }

        if (string.IsNullOrWhiteSpace(widget.Tooltip))
            _tooltip.SetToolTip(control, null);
        else
            _tooltip.SetToolTip(control, widget.Tooltip);
    }

    private void HookWidgetControl(Control control, WidgetModel widget)
    {
        control.MouseDown += (_, e) =>
        {
            if (e.Button != MouseButtons.Left)
                return;

            SelectWidget(widget);
            _dragging = true;
            _dragStart = Cursor.Position;
            _controlStart = new Point(widget.Left, widget.Top);
        };

        control.MouseMove += (_, _) =>
        {
            if (!_dragging || _selected != widget)
                return;

            var current = Cursor.Position;
            var deltaX = current.X - _dragStart.X;
            var deltaY = current.Y - _dragStart.Y;
            widget.Left = Math.Max(0, _controlStart.X + deltaX);
            widget.Top = Math.Max(0, _controlStart.Y + deltaY);
            ApplyWidget(control, widget);
            LayoutChanged?.Invoke(this, EventArgs.Empty);
        };

        control.MouseUp += (_, _) =>
        {
            _dragging = false;
        };

        control.Click += (_, _) => SelectWidget(widget);
    }

    private void SelectWidget(WidgetModel widget)
    {
        _selected = widget;
        WidgetSelected?.Invoke(this, widget);
        Invalidate();
    }

    private void ClearSelection()
    {
        _selected = null;
        WidgetSelected?.Invoke(this, null);
        Invalidate();
    }

    private void OnDragEnterCanvas(object? sender, DragEventArgs e)
    {
        if (e.Data?.GetDataPresent(typeof(string)) == true)
            e.Effect = DragDropEffects.Copy;
    }

    private void OnDragDropCanvas(object? sender, DragEventArgs e)
    {
        if (e.Data?.GetData(typeof(string)) is not string text)
            return;

        if (!Enum.TryParse<WidgetKind>(text, out var kind))
            return;

        var point = PointToClient(new Point(e.X, e.Y));
        AddNewWidget(kind, point);
    }

    private static Color ParseColor(string value, Color fallback)
    {
        try
        {
            return ColorTranslator.FromHtml(value);
        }
        catch
        {
            return fallback;
        }
    }

    private static Color ApplyAlpha(Color color, int alpha)
    {
        return Color.FromArgb(Math.Clamp(alpha, 0, 255), color);
    }

    private void PaintWidgetPanel(object? sender, PaintEventArgs e)
    {
        if (sender is not Panel panel || panel.Tag is not WidgetModel widget)
            return;

        var backColor = ParseColor(widget.BackColor, Color.FromArgb(30, 30, 40));
        var foreColor = ParseColor(widget.ForeColor, Color.FromArgb(48, 132, 214));
        var borderColor = ParseColor(widget.BorderColor, Color.DimGray);

        using var back = new SolidBrush(backColor);
        e.Graphics.FillRectangle(back, panel.ClientRectangle);

        if (widget.Kind == WidgetKind.Texture || widget.Kind == WidgetKind.Icon)
        {
            using var placeholderBrush = new SolidBrush(Color.FromArgb(58, 73, 97));
            var inner = panel.ClientRectangle;
            inner.Inflate(-2, -2);
            e.Graphics.FillRectangle(placeholderBrush, inner);
            TextRenderer.DrawText(
                e.Graphics,
                Path.GetFileName(widget.TexturePath),
                panel.Font,
                inner,
                panel.ForeColor,
                TextFormatFlags.HorizontalCenter |
                TextFormatFlags.VerticalCenter |
                TextFormatFlags.WordBreak);
        }
        else
        {
            var ratio = widget.MaxValue <= 0 ? 0.0 :
                Math.Clamp((double)widget.Value / widget.MaxValue, 0.0, 1.0);
            var fillWidth = Math.Max(0, (int)(panel.Width * ratio));
            if (fillWidth > 0)
            {
                using var fill = new SolidBrush(foreColor);
                var fillRect = new Rectangle(0, 0, fillWidth, panel.Height);
                e.Graphics.FillRectangle(fill, fillRect);
            }
        }

        ControlPaint.DrawBorder(
            e.Graphics,
            panel.ClientRectangle,
            borderColor,
            ButtonBorderStyle.Solid);
    }
}
