import csv
import re
import shutil
import subprocess
from pathlib import Path


DBC_DIR = Path(r"E:\server\data\karazhan\20260106")
SERVER_DBC_DIR = Path(r"E:\server\operate\data\dbc")
BUILD_DIR = Path(r"E:\server\tmp_client_base_creature_dbc")
DBCUTIL = DBC_DIR / "DBCUtil.exe"
MPQCLI = Path(r"E:\server\tools\mpqcli\mpqcli.exe")
CLIENT_BASE_MPQ = Path(r"E:\server\3.3.5\Data\koKR\patch-koKR-3.MPQ")
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


def extract_dbc(mpq_path, archive_path, output_dir):
    output_dir.mkdir(parents=True, exist_ok=True)
    run([str(MPQCLI), "extract", "-o", str(output_dir), "-f", archive_path, str(mpq_path)])
    return output_dir / Path(archive_path.replace("\\", "/")).name


def convert_to_csv(dbc_path):
    work = dbc_path.parent
    csv_path = work / (dbc_path.name + ".csv")
    if csv_path.exists():
        csv_path.unlink()
    run([str(DBCUTIL), str(dbc_path)], cwd=str(work))
    return csv_path


def convert_to_dbc(csv_path):
    dbc_path = csv_path.with_suffix("")
    if dbc_path.exists():
        dbc_path.unlink()
    run([str(DBCUTIL), str(csv_path)], cwd=str(csv_path.parent))
    return dbc_path


def load_csv(path):
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        return list(csv.reader(f))


def write_csv(path, rows):
    with path.open("w", encoding="utf-8-sig", newline="") as f:
        csv.writer(f, lineterminator="\n").writerows(rows)


def mpq_file_list():
    output = run([str(MPQCLI), "list", str(PATCH_Z)])
    return [line.strip() for line in output.splitlines() if line.strip()]


def extract_model(model_path):
    model_dir = BUILD_DIR / "models"
    model_dir.mkdir(parents=True, exist_ok=True)
    run([str(MPQCLI), "extract", "-o", str(model_dir), "-f", model_path, str(PATCH_Z)])
    return model_dir / Path(model_path.replace("\\", "/")).name


def embedded_blp_stems(model_path):
    try:
        data = extract_model(model_path).read_bytes()
    except Exception:
        return []

    stems = []
    for match in re.findall(rb"[A-Za-z0-9_\\/\\.-]+\\.blp", data, re.IGNORECASE):
        stem = Path(match.decode("latin1", "ignore").replace("/", "\\")).stem
        if stem and stem.lower() not in [s.lower() for s in stems]:
            stems.append(stem)
    return stems


def folder_blp_stems(model_path, files):
    folder = norm(str(Path(model_path.replace("\\", "/")).parent)).rstrip("\\") + "\\"
    stems = []
    for path in files:
        low = norm(path)
        if not low.startswith(folder) or not low.endswith(".blp"):
            continue
        stem = Path(path.replace("\\", "/")).stem
        if stem and stem.lower() not in [s.lower() for s in stems]:
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
    if any(word in low for word in ["reflect", "glow", "pulse", "spark", "smoke", "cloud", "fx_"]):
        score -= 50
    if low.startswith("armorreflect") or low.startswith("orbreflect") or low.startswith("chrome"):
        score -= 100
    return score


def choose_textures(model_path, files):
    model_stem = Path(model_path.replace("\\", "/")).stem
    candidates = []
    for stem in embedded_blp_stems(model_path) + folder_blp_stems(model_path, files):
        if stem.lower() not in [s.lower() for s in candidates]:
            candidates.append(stem)
    candidates.sort(key=lambda stem: (-score_texture(stem, model_stem), stem.lower()))
    chosen = candidates[:4]
    while len(chosen) < 4:
        chosen.append("")
    return chosen


def main():
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    BUILD_DIR.mkdir(parents=True)

    base_dir = BUILD_DIR / "base"
    base_model_dbc = extract_dbc(CLIENT_BASE_MPQ, r"DBFilesClient\CreatureModelData.dbc", base_dir)
    base_display_dbc = extract_dbc(CLIENT_BASE_MPQ, r"DBFilesClient\CreatureDisplayInfo.dbc", base_dir)
    base_model_csv = convert_to_csv(base_model_dbc)
    base_display_csv = convert_to_csv(base_display_dbc)

    base_model_rows = load_csv(base_model_csv)
    base_display_rows = load_csv(base_display_csv)
    current_model_rows = load_csv(DBC_DIR / "CreatureModelData.dbc.csv")
    current_display_rows = load_csv(DBC_DIR / "CreatureDisplayInfo.dbc.csv")

    custom_model_rows = [
        row
        for row in current_model_rows[1:]
        if row and row[0].isdigit() and 900001 <= int(row[0]) <= 900999
    ]
    custom_display_rows = [
        row
        for row in current_display_rows[1:]
        if row and row[0].isdigit() and 900001 <= int(row[0]) <= 900999
    ]

    model_path_by_id = {
        int(row[0]): row[2]
        for row in custom_model_rows
        if row and row[0].isdigit() and len(row) > 2
    }
    patch_z_files = mpq_file_list()

    fixed_textures = 0
    for row in custom_display_rows:
        display_id = int(row[0])
        model_id = int(row[1])
        if 900185 <= display_id <= 900273:
            textures = choose_textures(model_path_by_id.get(model_id, ""), patch_z_files)
            row[6:10] = textures
            if any(textures):
                fixed_textures += 1

    merged_model = [base_model_rows[0]] + [
        row
        for row in base_model_rows[1:]
        if not (row and row[0].isdigit() and 900001 <= int(row[0]) <= 900999)
    ] + custom_model_rows

    merged_display = [base_display_rows[0]] + [
        row
        for row in base_display_rows[1:]
        if not (row and row[0].isdigit() and 900001 <= int(row[0]) <= 900999)
    ] + custom_display_rows

    out_model_csv = DBC_DIR / "CreatureModelData.dbc.csv"
    out_display_csv = DBC_DIR / "CreatureDisplayInfo.dbc.csv"
    write_csv(out_model_csv, merged_model)
    write_csv(out_display_csv, merged_display)

    model_dbc = convert_to_dbc(out_model_csv)
    display_dbc = convert_to_dbc(out_display_csv)
    shutil.copy2(model_dbc, DBC_DIR / "CreatureModelData.dbc")
    shutil.copy2(display_dbc, DBC_DIR / "CreatureDisplayInfo.dbc")
    server_copy_status = "완료"
    try:
        shutil.copy2(model_dbc, SERVER_DBC_DIR / "CreatureModelData.dbc")
        shutil.copy2(display_dbc, SERVER_DBC_DIR / "CreatureDisplayInfo.dbc")
    except PermissionError as exc:
        server_copy_status = f"건너뜀 - 서버 프로세스가 DBC 파일을 사용 중입니다: {exc}"

    report = Path(r"E:\server\azerothcore-wotlk\doc\client_base_creature_dbc_merge_ko.md")
    lines = [
        "# 클라이언트 기준 Creature DBC 병합",
        "",
        f"- 기준 MPQ: {CLIENT_BASE_MPQ}",
        f"- 커스텀 CreatureModelData 행: {len(custom_model_rows)}",
        f"- 커스텀 CreatureDisplayInfo 행: {len(custom_display_rows)}",
        f"- patch-Z 신규 display 텍스처 보정: {fixed_textures}",
        f"- 최종 CreatureModelData 행: {len(merged_model) - 1}",
        f"- 최종 CreatureDisplayInfo 행: {len(merged_display) - 1}",
        f"- 서버 DBC 복사: {server_copy_status}",
    ]
    report.write_text("\n".join(lines) + "\n", encoding="utf-8")

    for path in [out_model_csv, out_display_csv, report]:
        text = path.read_text(encoding="utf-8-sig", errors="replace")
        if "\ufffd" in text:
            raise RuntimeError(f"replacement character found in {path}")

    print(f"custom_models={len(custom_model_rows)}")
    print(f"custom_displays={len(custom_display_rows)}")
    print(f"fixed_textures={fixed_textures}")
    print(report)


if __name__ == "__main__":
    main()
