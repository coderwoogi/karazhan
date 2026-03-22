using KarazhanAddonStudio.Models;
using KarazhanAddonStudio.Services;

namespace KarazhanAddonStudio;

public sealed class MainForm : Form
{
    private readonly LayoutDocument _document = new();
    private readonly ListBox _toolbox = new();
    private readonly DesignerCanvas _canvas = new();
    private readonly PropertyGrid _propertyGrid = new();
    private readonly TextBox _outputBox = new();
    private readonly Label _statusLabel = new();
    private readonly NumericUpDown _documentWidth = new();
    private readonly NumericUpDown _documentHeight = new();
    private readonly TextBox _documentName = new();

    public MainForm()
    {
        InitializeLayout();
        LoadDefaultWidgets();
    }

    private void InitializeLayout()
    {
        Text = "Karazhan Addon Studio";
        Width = 1380;
        Height = 860;
        StartPosition = FormStartPosition.CenterScreen;
        KeyPreview = true;

        var toolStrip = new ToolStrip();
        var newButton = new ToolStripButton("새 문서");
        var saveJsonButton = new ToolStripButton("JSON 저장");
        var loadJsonButton = new ToolStripButton("JSON 열기");
        var exportLuaButton = new ToolStripButton("Lua 내보내기");
        var copyLuaButton = new ToolStripButton("Lua 복사");
        var deleteButton = new ToolStripButton("선택 삭제");

        newButton.Click += (_, _) => ResetDocument();
        saveJsonButton.Click += (_, _) => SaveJson();
        loadJsonButton.Click += (_, _) => LoadJson();
        exportLuaButton.Click += (_, _) => ExportLuaFile();
        copyLuaButton.Click += (_, _) => CopyLua();
        deleteButton.Click += (_, _) => DeleteSelectedWidget();

        toolStrip.Items.AddRange(
        [
            newButton,
            saveJsonButton,
            loadJsonButton,
            exportLuaButton,
            copyLuaButton,
            new ToolStripSeparator(),
            deleteButton
        ]);

        Controls.Add(toolStrip);

        var split = new SplitContainer
        {
            Dock = DockStyle.Fill,
            SplitterDistance = 180
        };
        Controls.Add(split);
        split.BringToFront();

        var rightSplit = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Vertical,
            SplitterDistance = 760
        };
        split.Panel2.Controls.Add(rightSplit);

        var designSplit = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Horizontal,
            SplitterDistance = 520
        };
        rightSplit.Panel1.Controls.Add(designSplit);

        var documentBar = new FlowLayoutPanel
        {
            Dock = DockStyle.Top,
            Height = 34,
            Padding = new Padding(8, 4, 8, 4),
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false
        };
        rightSplit.Panel1.Controls.Add(documentBar);
        documentBar.BringToFront();

        documentBar.Controls.Add(new Label
        {
            AutoSize = true,
            Text = "문서명",
            TextAlign = ContentAlignment.MiddleLeft,
            Margin = new Padding(0, 6, 6, 0)
        });

        _documentName.Width = 180;
        _documentName.TextChanged += (_, _) =>
        {
            _document.Name = _documentName.Text.Trim().Length == 0
                ? "KarazhanBonusMission"
                : _documentName.Text.Trim();
            RefreshLuaPreview();
        };
        documentBar.Controls.Add(_documentName);

        documentBar.Controls.Add(new Label
        {
            AutoSize = true,
            Text = "너비",
            TextAlign = ContentAlignment.MiddleLeft,
            Margin = new Padding(12, 6, 6, 0)
        });

        _documentWidth.Minimum = 160;
        _documentWidth.Maximum = 4000;
        _documentWidth.Width = 80;
        _documentWidth.ValueChanged += (_, _) => ApplyDocumentSizeFromInputs();
        documentBar.Controls.Add(_documentWidth);

        documentBar.Controls.Add(new Label
        {
            AutoSize = true,
            Text = "높이",
            TextAlign = ContentAlignment.MiddleLeft,
            Margin = new Padding(12, 6, 6, 0)
        });

        _documentHeight.Minimum = 120;
        _documentHeight.Maximum = 4000;
        _documentHeight.Width = 80;
        _documentHeight.ValueChanged += (_, _) => ApplyDocumentSizeFromInputs();
        documentBar.Controls.Add(_documentHeight);

        var toolboxLabel = new Label
        {
            Dock = DockStyle.Top,
            Height = 28,
            Text = "툴박스",
            TextAlign = ContentAlignment.MiddleLeft
        };
        split.Panel1.Controls.Add(toolboxLabel);

        _toolbox.Dock = DockStyle.Fill;
        _toolbox.Items.AddRange(Enum.GetNames<WidgetKind>());
        _toolbox.MouseDown += (_, _) =>
        {
            if (_toolbox.SelectedItem is string item)
                _toolbox.DoDragDrop(item, DragDropEffects.Copy);
        };
        split.Panel1.Controls.Add(_toolbox);
        _toolbox.BringToFront();

        var canvasHost = new Panel
        {
            Dock = DockStyle.Fill,
            AutoScroll = true,
            BackColor = Color.FromArgb(42, 46, 57)
        };
        designSplit.Panel1.Controls.Add(canvasHost);

        _canvas.Location = new Point(16, 16);
        _canvas.WidgetSelected += (_, widget) =>
        {
            _propertyGrid.SelectedObject = widget is null
                ? _document
                : widget;
        };
        _canvas.LayoutChanged += (_, _) =>
        {
            SyncDocumentFromCanvas();
            SyncDocumentInputs();
            RefreshLuaPreview();
        };
        canvasHost.Controls.Add(_canvas);

        _outputBox.Dock = DockStyle.Fill;
        _outputBox.Multiline = true;
        _outputBox.ScrollBars = ScrollBars.Both;
        _outputBox.Font = new Font("Consolas", 10.0f);
        designSplit.Panel2.Controls.Add(_outputBox);

        _propertyGrid.Dock = DockStyle.Fill;
        _propertyGrid.PropertyValueChanged += (_, _) =>
        {
            if (_propertyGrid.SelectedObject is WidgetModel widget)
            {
                _canvas.RefreshWidget(widget);
                RefreshLuaPreview();
            }
        };
        rightSplit.Panel2.Controls.Add(_propertyGrid);

        _statusLabel.Dock = DockStyle.Bottom;
        _statusLabel.Height = 24;
        _statusLabel.Text = "툴박스에서 위젯을 끌어와 배치하세요. Delete 키로 삭제할 수 있습니다.";
        Controls.Add(_statusLabel);
        _statusLabel.BringToFront();

        KeyDown += OnMainFormKeyDown;
        SyncDocumentInputs();
    }

    private void LoadDefaultWidgets()
    {
        _document.Widgets.Clear();
        _document.Widgets.AddRange(
        [
            new WidgetModel
            {
                Name = "TitleLabel",
                Kind = WidgetKind.Label,
                Left = 18,
                Top = 20,
                Width = 180,
                Height = 22,
                Text = "추가 임무",
                ForeColor = "#FFD25A"
            },
            new WidgetModel
            {
                Name = "ThemeFrame",
                Kind = WidgetKind.Frame,
                Left = 214,
                Top = 18,
                Width = 120,
                Height = 52,
                BackColor = "#35204C",
                Text = "보스 집중형"
            },
            new WidgetModel
            {
                Name = "MissionProgress",
                Kind = WidgetKind.ProgressBar,
                Left = 18,
                Top = 92,
                Width = 260,
                Height = 16,
                Value = 64
            },
            new WidgetModel
            {
                Name = "MissionButton",
                Kind = WidgetKind.Button,
                Left = 18,
                Top = 132,
                Width = 120,
                Height = 28,
                Text = "임무 보기"
            }
        ]);

        _canvas.LoadDocument(_document);
        _propertyGrid.SelectedObject = _document;
        SyncDocumentInputs();
        RefreshLuaPreview();
    }

    private void ResetDocument()
    {
        _document.Name = "KarazhanBonusMission";
        _document.Width = 344;
        _document.Height = 246;
        LoadDefaultWidgets();
        _statusLabel.Text = "새 문서를 만들었습니다.";
    }

    private void SyncDocumentFromCanvas()
    {
        _document.Width = _canvas.Width;
        _document.Height = _canvas.Height;
        _document.Widgets.Clear();

        foreach (Control control in _canvas.Controls)
        {
            if (control.Tag is WidgetModel widget)
                _document.Widgets.Add(widget);
        }
    }

    private void RefreshLuaPreview()
    {
        SyncDocumentFromCanvas();
        _outputBox.Text = LuaExporter.Export(_document);
    }

    private void SaveJson()
    {
        using var dialog = new SaveFileDialog
        {
            Filter = "Layout JSON (*.json)|*.json",
            FileName = $"{_document.Name}.json"
        };

        if (dialog.ShowDialog(this) != DialogResult.OK)
            return;

        LayoutSerializer.Save(dialog.FileName, _document);
        _statusLabel.Text = $"JSON 저장 완료: {dialog.FileName}";
    }

    private void LoadJson()
    {
        using var dialog = new OpenFileDialog
        {
            Filter = "Layout JSON (*.json)|*.json"
        };

        if (dialog.ShowDialog(this) != DialogResult.OK)
            return;

        var loaded = LayoutSerializer.Load(dialog.FileName);
        _document.Name = loaded.Name;
        _document.Width = loaded.Width;
        _document.Height = loaded.Height;
        _document.Widgets = loaded.Widgets;
        _canvas.LoadDocument(_document);
        _propertyGrid.SelectedObject = _document;
        SyncDocumentInputs();
        RefreshLuaPreview();
        _statusLabel.Text = $"JSON 불러오기 완료: {dialog.FileName}";
    }

    private void ExportLuaFile()
    {
        using var dialog = new SaveFileDialog
        {
            Filter = "Lua Script (*.lua)|*.lua",
            FileName = $"{_document.Name}.lua"
        };

        if (dialog.ShowDialog(this) != DialogResult.OK)
            return;

        File.WriteAllText(dialog.FileName, _outputBox.Text);
        _statusLabel.Text = $"Lua 내보내기 완료: {dialog.FileName}";
    }

    private void CopyLua()
    {
        Clipboard.SetText(_outputBox.Text);
        _statusLabel.Text = "Lua 코드를 클립보드에 복사했습니다.";
    }

    private void DeleteSelectedWidget()
    {
        if (_canvas.SelectedWidget is null)
        {
            _statusLabel.Text = "삭제할 위젯을 먼저 선택하세요.";
            return;
        }

        var removedName = _canvas.SelectedWidget.Name;
        _canvas.RemoveSelectedWidget();
        _propertyGrid.SelectedObject = null;
        RefreshLuaPreview();
        _statusLabel.Text = $"{removedName} 위젯을 삭제했습니다.";
    }

    private void OnMainFormKeyDown(object? sender, KeyEventArgs e)
    {
        if (e.KeyCode != Keys.Delete)
            return;

        DeleteSelectedWidget();
        e.Handled = true;
    }

    private void ApplyDocumentSizeFromInputs()
    {
        _document.Width = (int)_documentWidth.Value;
        _document.Height = (int)_documentHeight.Value;
        _canvas.Width = _document.Width;
        _canvas.Height = _document.Height;
        RefreshLuaPreview();
        _canvas.Invalidate();
    }

    private void SyncDocumentInputs()
    {
        _documentName.Text = _document.Name;
        _documentWidth.Value = Math.Max(
            _documentWidth.Minimum,
            Math.Min(_documentWidth.Maximum, _document.Width));
        _documentHeight.Value = Math.Max(
            _documentHeight.Minimum,
            Math.Min(_documentHeight.Maximum, _document.Height));
    }
}
