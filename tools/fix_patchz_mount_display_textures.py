import csv
import re
import shutil
import subprocess
from pathlib import Path


DBC_DIR = Path(r"E:\server\data\karazhan\20260106")
SERVER_DBC_DIR = Path(r"E:\server\operate\data\dbc")
BUILD_DIR = Path(r"E:\server\tmp_patchz_mount_texture_fix")
DBCUTIL = DBC_DIR / "DBCUtil.exe"
MPQCLI = Path(r"E:\server\tools\mpqcli\mpqcli.exe")
PATCH_Z = Path(r"E:\server\3.3.5\Data\patch-Z.MPQ")


def run(command, **kwargs):
    result = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
        **kwargs,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def norm(path):
    return path.replace("/", "\\").lower()


def load_mpq_files():
    files = []
    output = run([str(MPQCLI), "list", str(PATCH_Z)])
    for line in output.splitlines():
        path = line.strip()
        if path:
            files.append(path)
    return files


def extract_model(model_path):
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    BUILD_DIR.mkdir(parents=True)
    run(
        [
            str(MPQCLI),
            "extract",
            "-o",
            str(BUILD_DIR),
            "-f",
            model_path,
            str(PATCH_Z),
        ]
    )
    return BUILD_DIR / Path(model_path.replace("\\", "/")).name


def embedded_blp_stems(model_path):
    try:
        extracted = extract_model(model_path)
        data = extracted.read_bytes()
    except Exception:
        return []

    found = []
    for match in re.findall(rb"[A-Za-z0-9_\\/\\.-]+\\.blp", data, re.IGNORECASE):
        text = match.decode("latin1", "ignore").replace("/", "\\")
        stem = Path(text).stem
        if stem and stem.lower() not in [x.lower() for x in found]:
            found.append(stem)
    return found


def folder_blp_stems(model_path, mpq_files):
    folder = norm(str(Path(model_path.replace("\\", "/")).parent)).rstrip("\\") + "\\"
    stems = []
    for path in mpq_files:
        low = norm(path)
        if not low.startswith(folder) or not low.endswith(".blp"):
            continue
        stem = Path(path.replace("\\", "/")).stem
        if stem and stem.lower() not in [x.lower() for x in stems]:
            stems.append(stem)
    return stems


def score_texture(stem, model_stem):
    low = stem.lower()
    score = 0
    if model_stem.lower() in low:
        score += 100
    if "skin" in low:
        score += 80
    if "mount" in low:
        score += 50
    if any(word in low for word in ["brown", "black", "white", "red", "blue", "green", "purple", "gold", "gray", "grey"]):
        score += 20
    if "armor" in low:
        score += 10
    if any(word in low for word in ["reflect", "glow", "pulse", "light", "spark", "cloud", "smoke", "fire", "fx_"]):
        score -= 40
    if low.startswith("armorreflect") or low.startswith("orbreflect") or low.startswith("chrome"):
        score -= 80
    return score


def choose_textures(model_path, mpq_files):
    model_stem = Path(model_path.replace("\\", "/")).stem
    embedded = embedded_blp_stems(model_path)
    folder = folder_blp_stems(model_path, mpq_files)
    candidates = []
    for stem in embedded + folder:
        if stem.lower() not in [x.lower() for x in candidates]:
            candidates.append(stem)

    candidates.sort(key=lambda stem: (-score_texture(stem, model_stem), stem.lower()))
    chosen = candidates[:4]
    while len(chosen) < 4:
        chosen.append("")
    return chosen[:4]


def main():
    mpq_files = load_mpq_files()

    model_rows = list(csv.reader((DBC_DIR / "CreatureModelData.dbc.csv").open("r", encoding="utf-8-sig", newline="")))
    model_path_by_id = {}
    for row in model_rows[1:]:
        if row and row[0].isdigit() and len(row) > 2:
            model_path_by_id[int(row[0])] = row[2]

    display_path = DBC_DIR / "CreatureDisplayInfo.dbc.csv"
    display_rows = list(csv.reader(display_path.open("r", encoding="utf-8-sig", newline="")))

    fixed = []
    for row in display_rows[1:]:
        if not row or not row[0].isdigit() or not row[1].isdigit():
            continue
        display_id = int(row[0])
        model_id = int(row[1])
        if not (900185 <= display_id <= 900273):
            continue
        model_path = model_path_by_id.get(model_id, "")
        if not model_path:
            continue
        textures = choose_textures(model_path, mpq_files)
        row[6:10] = textures
        fixed.append((display_id, model_path, textures))

    with display_path.open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerows(display_rows)

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    BUILD_DIR.mkdir(parents=True)
    temp_csv = BUILD_DIR / "CreatureDisplayInfo.dbc.csv"
    shutil.copy2(display_path, temp_csv)
    run([str(DBCUTIL), str(temp_csv)], cwd=str(BUILD_DIR))
    generated = BUILD_DIR / "CreatureDisplayInfo.dbc"
    shutil.copy2(generated, DBC_DIR / "CreatureDisplayInfo.dbc")
    shutil.copy2(generated, SERVER_DBC_DIR / "CreatureDisplayInfo.dbc")

    report = Path(r"E:\server\azerothcore-wotlk\doc\patchz_mount_texture_fix_ko.md")
    lines = [
        "# patch-Z 탈것 텍스처 연결 보정",
        "",
        f"- 보정 display 수: {len(fixed)}",
        "",
        "| display | 모델 | TextureVariation 1 | 2 | 3 | 4 |",
        "|---:|---|---|---|---|---|",
    ]
    for display_id, model_path, textures in fixed:
        lines.append(f"| {display_id} | {model_path} | {textures[0]} | {textures[1]} | {textures[2]} | {textures[3]} |")
    report.write_text("\n".join(lines) + "\n", encoding="utf-8")

    text = display_path.read_text(encoding="utf-8-sig", errors="replace") + report.read_text(encoding="utf-8", errors="replace")
    if "\ufffd" in text:
        raise RuntimeError("replacement character found after texture fix")

    print(f"fixed={len(fixed)}")
    print(report)


if __name__ == "__main__":
    main()
