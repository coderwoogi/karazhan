import csv
import subprocess
from collections import Counter
from pathlib import Path


ROOT = Path(r"E:\server\azerothcore-wotlk")
DBC_DIR = Path(r"E:\server\data\karazhan\20260106")
REPORT_MD = ROOT / "doc" / "mount_vendor_190059_audit_ko.md"
REPORT_CSV = ROOT / "doc" / "mount_vendor_190059_audit_ko.csv"
VENDOR_ENTRY = 190059


def run_mysql(sql):
    result = subprocess.run(
        [
            "mysql",
            "--default-character-set=utf8mb4",
            "-uacore",
            "-pacore",
            "-N",
            "-B",
            "acore_world",
            "-e",
            sql,
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr)
    return result.stdout


def load_spell_names():
    names = {}
    with (DBC_DIR / "Spell.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            if not row or not row[0].isdigit():
                continue
            spell_id = int(row[0])
            names[spell_id] = row[137] or row[136] or ""
    return names


def load_display_model_paths():
    model_paths = {}
    with (DBC_DIR / "CreatureModelData.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            if row and row[0].isdigit():
                model_paths[int(row[0])] = row[2]

    display_paths = {}
    with (DBC_DIR / "CreatureDisplayInfo.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            if row and row[0].isdigit() and row[1].isdigit():
                display_paths[int(row[0])] = model_paths.get(int(row[1]), "")
    return display_paths


def yes_no(value):
    return "있음" if value else "없음"


def load_vendor_rows():
    sql = f"""
        SELECT
            v.slot,
            v.item,
            CASE WHEN it.entry IS NULL THEN 0 ELSE 1 END AS item_exists,
            COALESCE(it.name, '') AS item_name,
            COALESCE(it.spellid_2, 0) AS item_spell,
            CASE WHEN k.spell_id IS NULL THEN 0 ELSE 1 END AS collection_exists,
            COALESCE(k.spell_id, 0) AS collection_spell,
            COALESCE(k.creature_entry, 0) AS creature_entry,
            COALESCE(k.display_id, 0) AS display_id,
            COALESCE(k.mount_type, '') AS mount_type,
            COALESCE(k.name, '') AS collection_name,
            CASE WHEN c.entry IS NULL THEN 0 ELSE 1 END AS creature_exists,
            CASE WHEN cm.CreatureID IS NULL THEN 0 ELSE 1 END AS creature_model_exists
        FROM npc_vendor v
        LEFT JOIN item_template it ON it.entry = v.item
        LEFT JOIN karazhan_mount_collection_spell k ON k.spell_id = it.spellid_2
        LEFT JOIN creature_template c ON c.entry = k.creature_entry
        LEFT JOIN creature_template_model cm ON cm.CreatureID = k.creature_entry
          AND cm.CreatureDisplayID = k.display_id
        WHERE v.entry = {VENDOR_ENTRY}
        ORDER BY v.slot, v.item;
    """
    rows = []
    for line in run_mysql(sql).splitlines():
        if not line.strip():
            continue
        parts = line.split("\t")
        rows.append(
            {
                "slot": int(parts[0]),
                "item_id": int(parts[1]),
                "item_exists": parts[2] == "1",
                "item_name": parts[3],
                "item_spell": int(parts[4]),
                "collection_exists": parts[5] == "1",
                "collection_spell": int(parts[6]),
                "creature_entry": int(parts[7]),
                "display_id": int(parts[8]),
                "mount_type": parts[9],
                "collection_name": parts[10],
                "creature_exists": parts[11] == "1",
                "creature_model_exists": parts[12] == "1",
            }
        )
    return rows


def write_reports(rows):
    spell_names = load_spell_names()
    display_paths = load_display_model_paths()
    name_counts = Counter(row["item_name"] for row in rows)
    duplicate_names = {name for name, count in name_counts.items() if count > 1}

    enriched = []
    for row in rows:
        spell_id = row["item_spell"]
        spell_name = spell_names.get(spell_id, "")
        row = row.copy()
        row["spell_dbc_exists"] = spell_id in spell_names
        row["spell_name"] = spell_name
        row["name_duplicate"] = row["item_name"] in duplicate_names
        row["duplicate_count"] = name_counts[row["item_name"]]
        row["model_path"] = display_paths.get(row["display_id"], "")
        row["item_spell_matches_collection"] = (
            row["collection_exists"] and row["item_spell"] == row["collection_spell"]
        )
        enriched.append(row)

    csv_columns = [
        "slot",
        "item_id",
        "item_name",
        "name_duplicate",
        "duplicate_count",
        "mount_type",
        "item_exists",
        "item_spell",
        "spell_dbc_exists",
        "spell_name",
        "item_spell_matches_collection",
        "collection_exists",
        "creature_entry",
        "creature_exists",
        "display_id",
        "creature_model_exists",
        "model_path",
    ]
    with REPORT_CSV.open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=csv_columns, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(enriched)

    total = len(enriched)
    duplicate_row_count = sum(1 for row in enriched if row["name_duplicate"])
    duplicate_group_count = len(duplicate_names)
    missing_items = sum(1 for row in enriched if not row["item_exists"])
    missing_spells = sum(1 for row in enriched if not row["spell_dbc_exists"])
    missing_collection = sum(1 for row in enriched if not row["collection_exists"])
    mismatch = sum(1 for row in enriched if not row["item_spell_matches_collection"])
    missing_creatures = sum(1 for row in enriched if not row["creature_exists"])
    missing_models = sum(1 for row in enriched if not row["creature_model_exists"])

    lines = [
        "# 190059 탈것 판매 목록 점검",
        "",
        "## 요약",
        "",
        f"- 판매 등록 수: {total}",
        f"- 이름 중복 그룹: {duplicate_group_count}",
        f"- 이름 중복에 포함된 행: {duplicate_row_count}",
        f"- 아이템 누락: {missing_items}",
        f"- Spell.dbc 주문 누락: {missing_spells}",
        f"- 컬렉션 테이블 연결 누락: {missing_collection}",
        f"- 아이템 spellid_2와 컬렉션 spell_id 불일치: {mismatch}",
        f"- creature_template 누락: {missing_creatures}",
        f"- creature_template_model 누락: {missing_models}",
        "",
        "## 중복 이름 그룹",
        "",
        "| 이름 | 개수 | 아이템 |",
        "|---|---:|---|",
    ]

    for name in sorted(duplicate_names):
        items = ", ".join(str(row["item_id"]) for row in enriched if row["item_name"] == name)
        lines.append(f"| {name} | {name_counts[name]} | {items} |")

    lines.extend(
        [
            "",
            "## 전체 판매 탈것",
            "",
            "| 슬롯 | 아이템 | 이름(한글) | 중복 | 구분 | 아이템 | 주문 | 주문명 | 컬렉션 | 크리처 | display | 모델 |",
            "|---:|---:|---|---|---|---|---|---|---|---|---:|---|",
        ]
    )

    for row in enriched:
        duplicate = f"중복 {row['duplicate_count']}개" if row["name_duplicate"] else "아님"
        item_status = yes_no(row["item_exists"])
        spell_status = yes_no(row["spell_dbc_exists"])
        collection_status = "정상" if row["item_spell_matches_collection"] else "문제"
        creature_status = (
            "정상" if row["creature_exists"] and row["creature_model_exists"] else "문제"
        )
        lines.append(
            f"| {row['slot']} | {row['item_id']} | {row['item_name']} | {duplicate} | "
            f"{row['mount_type']} | {item_status} | {row['item_spell']} {spell_status} | "
            f"{row['spell_name']} | {collection_status} | {creature_status} | "
            f"{row['display_id']} | {row['model_path']} |"
        )

    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")

    for path in [REPORT_MD, REPORT_CSV]:
        text = path.read_text(encoding="utf-8-sig", errors="replace")
        if "\ufffd" in text:
            raise RuntimeError(f"replacement character found in {path}")

    print(f"rows={total}")
    print(f"duplicate_groups={duplicate_group_count}")
    print(f"missing_spells={missing_spells}")
    print(REPORT_MD)
    print(REPORT_CSV)


def main():
    write_reports(load_vendor_rows())


if __name__ == "__main__":
    main()
