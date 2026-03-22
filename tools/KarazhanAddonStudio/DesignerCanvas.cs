using System.Drawing.Drawing2D;
using KarazhanAddonStudio.Models;

namespace KarazhanAddonStudio;

public sealed class DesignerCanvas : Panel
{
    public event EventHandler<WidgetModel>? WidgetSelected;
    public event EventHandler? LayoutChanged;

    private readonly Dictionary<WidgetModel, Control> _bindings = [];
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
    }

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
            Width = kind == WidgetKind.ProgressBar ? 180 : 120,
            Height = kind == WidgetKind.Label ? 22 : 28
        };

        if (kind == WidgetKind.Panel)
        {
            widget.Width = 180;
            widget.Height = 90;
            widget.Text = string.Empty;
        }

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
        LayoutChanged?.Invoke(this, EventArgs.Empty);
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
            WidgetKind.Button => "ButtonWidget",
            WidgetKind.ProgressBar => "ProgressWidget",
            _ => "Widget"
        };
    }

    private static string GetDefaultText(WidgetKind kind)
    {
        return kind switch
        {
            WidgetKind.Label => "새 텍스트",
            WidgetKind.Panel => string.Empty,
            WidgetKind.Button => "새 버튼",
            WidgetKind.ProgressBar => string.Empty,
            _ => "새 위젯"
        };
    }

    private static Control CreateControlFor(WidgetModel widget)
    {
        return widget.Kind switch
        {
            WidgetKind.Panel => new Panel(),
            WidgetKind.Button => new Button
            {
                FlatStyle = FlatStyle.Flat
            },
            WidgetKind.ProgressBar => new Panel(),
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
        control.BackColor = ParseColor(widget.BackColor, Color.FromArgb(30, 30, 40));
        control.ForeColor = ParseColor(widget.ForeColor, Color.Gainsboro);
        control.Tag = widget;

        switch (control)
        {
        case Label label:
            label.Text = widget.Text;
            label.TextAlign = ContentAlignment.MiddleLeft;
            break;
        case Button button:
            button.Text = widget.Text;
            button.FlatAppearance.BorderColor = Color.Gray;
            break;
        case Panel panel when widget.Kind == WidgetKind.ProgressBar:
            panel.Paint -= PaintProgress;
            panel.Paint += PaintProgress;
            break;
        }
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

        control.MouseMove += (_, e) =>
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

    private void PaintProgress(object? sender, PaintEventArgs e)
    {
        if (sender is not Panel panel || panel.Tag is not WidgetModel widget)
            return;

        using var fill = new SolidBrush(Color.FromArgb(48, 132, 214));
        using var back = new SolidBrush(ParseColor(widget.BackColor, Color.FromArgb(30, 30, 40)));
        e.Graphics.FillRectangle(back, panel.ClientRectangle);
        var fillRect = new Rectangle(0, 0, Math.Max(12, panel.Width / 2), panel.Height);
        e.Graphics.FillRectangle(fill, fillRect);
        ControlPaint.DrawBorder(
            e.Graphics,
            panel.ClientRectangle,
            Color.DimGray,
            ButtonBorderStyle.Solid);
    }
}
