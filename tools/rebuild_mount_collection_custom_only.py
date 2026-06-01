import csv
import shutil
import subprocess
from pathlib import Path


ROOT = Path(r"E:\server\azerothcore-wotlk")
DBC_DIR = Path(r"E:\server\data\karazhan\20260106")
SERVER_DBC_DIR = Path(r"E:\server\operate\data\dbc")
BUILD_DIR = Path(r"E:\server\tmp_mount_collection_custom_only")
DBCUTIL = DBC_DIR / "DBCUtil.exe"

SQL_OUT = (
    ROOT
    / "modules"
    / "mod-item-karazhan"
    / "data"
    / "sql"
    / "db-world"
    / "updates"
    / "2026_05_31_02_karazhan_mount_collection_custom_only.sql"
)
VENDOR_SQL_OUT = (
    ROOT
    / "modules"
    / "mod-item-karazhan"
    / "data"
    / "sql"
    / "db-world"
    / "updates"
    / "2026_05_31_01_karazhan_mount_collection_vendor.sql"
)
REPORT_OUT = ROOT / "doc" / "karazhan_mount_collection_custom_only_ko.md"

SPELL_START = 960001
ITEM_START = 970001
VENDOR_ENTRY = 190059
TEMPLATE_ITEM = 19872
GROUND_TEMPLATE_SPELL = 24242
FLYING_TEMPLATE_SPELL = 59567


GROUND_KEYWORDS = [
    "crane",
    "fox",
    "scorpion",
    "monkmount",
    "moose",
    "mushan",
    "ravenlord",
    "saber",
    "tiger",
    "raptor",
    "horse",
    "wolf",
    "bear",
    "kodo",
    "ram",
    "elekk",
    "strider",
    "mammoth",
    "chopper",
    "hog",
    "turtle",
    "seahorse",
]


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


def sql_quote(value):
    return "'" + str(value).replace("\\", "\\\\").replace("'", "''") + "'"


def load_columns(table):
    rows = run_mysql(f"SHOW COLUMNS FROM `{table}`;")
    return [line.split("\t", 1)[0] for line in rows.splitlines() if line.strip()]


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


def classify_mount_type(model_path):
    low = model_path.replace("\\", "/").lower()
    if any(keyword in low for keyword in GROUND_KEYWORDS):
        return "지상탈"
    return "날탈"


def load_custom_mounts():
    display_paths = load_display_model_paths()
    rows = run_mysql(
        """
        SELECT t.entry,t.name,m.CreatureDisplayID
        FROM creature_template t
        JOIN creature_template_model m ON m.CreatureID=t.entry
        WHERE m.CreatureDisplayID BETWEEN 900000 AND 999999
        ORDER BY t.entry,m.CreatureDisplayID;
        """
    )
    mounts = []
    seen = set()
    for line in rows.splitlines():
        if not line.strip():
            continue
        entry, name, display_id = line.split("\t")
        display_id = int(display_id)
        if display_id in seen:
            continue
        seen.add(display_id)
        model_path = display_paths.get(display_id, "")
        mounts.append(
            {
                "creature_entry": int(entry),
                "display_id": display_id,
                "name": name,
                "model_path": model_path,
                "mount_type": classify_mount_type(model_path),
            }
        )

    for index, mount in enumerate(mounts):
        mount["spell_id"] = SPELL_START + index
        mount["item_id"] = ITEM_START + index
        mount["slot"] = index + 1
    return mounts


def load_spell_rows():
    with (DBC_DIR / "Spell.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    header = rows[0]
    data = [
        row
        for row in rows[1:]
        if row and not (SPELL_START <= int(row[0]) <= 961000)
    ]
    return header, data


def set_localized_spell_text(row, name, description, aura):
    row[136] = ""
    row[137] = name
    for index in range(138, 152):
        row[index] = ""
    row[152] = "0xFF01FE"

    row[170] = ""
    row[171] = description
    for index in range(172, 186):
        row[index] = ""
    row[186] = "0xFF01FE"

    row[187] = ""
    row[188] = aura
    for index in range(189, 203):
        row[index] = ""
    row[203] = "0xFF01FE"


def rebuild_spell_dbc(mounts):
    header, data = load_spell_rows()
    templates = {int(row[0]): row for row in data if int(row[0]) in {GROUND_TEMPLATE_SPELL, FLYING_TEMPLATE_SPELL}}
    new_rows = []
    for mount in mounts:
        is_flying = mount["mount_type"] == "날탈"
        template = templates[FLYING_TEMPLATE_SPELL if is_flying else GROUND_TEMPLATE_SPELL].copy()
        template[0] = str(mount["spell_id"])
        template[110] = str(mount["creature_entry"])
        if is_flying:
            desc = f"{mount['name']}에 올라타거나 내립니다. 지상 및 비행 속도가 증가합니다."
            aura = "지상 이동 속도 100%, 비행 이동 속도 280%만큼 증가"
        else:
            desc = f"{mount['name']}에 올라타거나 내립니다. 지상 이동 속도가 증가합니다."
            aura = "지상 이동 속도 100%만큼 증가"
        set_localized_spell_text(template, mount["name"], desc, aura)
        new_rows.append(template)

    with (DBC_DIR / "Spell.dbc.csv").open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(header)
        writer.writerows(data)
        writer.writerows(new_rows)

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    BUILD_DIR.mkdir(parents=True)
    temp_csv = BUILD_DIR / "Spell.dbc.csv"
    shutil.copy2(DBC_DIR / "Spell.dbc.csv", temp_csv)
    subprocess.run([str(DBCUTIL), str(temp_csv)], cwd=str(BUILD_DIR), check=True)
    shutil.copy2(BUILD_DIR / "Spell.dbc", DBC_DIR / "Spell.dbc")
    shutil.copy2(BUILD_DIR / "Spell.dbc", SERVER_DBC_DIR / "Spell.dbc")


def build_item_insert(columns, mount):
    overrides = {
        "entry": str(mount["item_id"]),
        "class": "15",
        "subclass": "5",
        "name": sql_quote(mount["name"]),
        "Quality": "4",
        "BuyCount": "1",
        "BuyPrice": "0",
        "SellPrice": "0",
        "AllowableClass": "-1",
        "AllowableRace": "-1",
        "ItemLevel": "80",
        "RequiredLevel": "1",
        "RequiredSkill": "0",
        "RequiredSkillRank": "0",
        "requiredspell": "0",
        "maxcount": "1",
        "stackable": "1",
        "spellid_1": "55884",
        "spelltrigger_1": "0",
        "spellcharges_1": "-1",
        "spellppmRate_1": "0",
        "spellcooldown_1": "-1",
        "spellcategory_1": "330",
        "spellcategorycooldown_1": "3000",
        "spellid_2": str(mount["spell_id"]),
        "spelltrigger_2": "6",
        "spellcharges_2": "0",
        "spellppmRate_2": "0",
        "spellcooldown_2": "0",
        "spellcategory_2": "0",
        "spellcategorycooldown_2": "0",
        "spellid_3": "0",
        "spelltrigger_3": "0",
        "spellcharges_3": "0",
        "spellppmRate_3": "0",
        "spellcooldown_3": "0",
        "spellcategory_3": "0",
        "spellcategorycooldown_3": "0",
        "spellid_4": "0",
        "spelltrigger_4": "0",
        "spellcharges_4": "0",
        "spellppmRate_4": "0",
        "spellcooldown_4": "0",
        "spellcategory_4": "0",
        "spellcategorycooldown_4": "0",
        "spellid_5": "0",
        "spelltrigger_5": "0",
        "spellcharges_5": "0",
        "spellppmRate_5": "0",
        "spellcooldown_5": "0",
        "spellcategory_5": "0",
        "spellcategorycooldown_5": "0",
        "bonding": "1",
        "description": sql_quote("사용 시 해당 탈것 소환 주문을 배웁니다."),
        "ScriptName": "''",
        "VerifiedBuild": "12340",
    }
    values = [overrides.get(column, f"`{column}`") for column in columns]
    return (
        f"INSERT INTO `item_template` (`{'`,`'.join(columns)}`)\n"
        f"SELECT {', '.join(values)} FROM `item_template` WHERE `entry`={TEMPLATE_ITEM};"
    )


def write_sql(mounts):
    item_columns = load_columns("item_template")
    item_end = ITEM_START + len(mounts) - 1
    spell_end = SPELL_START + len(mounts) - 1
    lines = [
        "-- Rebuild mount collection to custom patch-Z/patch-4 display rows only.",
        "SET NAMES utf8mb4;",
        "",
        "CREATE TABLE IF NOT EXISTS `karazhan_mount_collection_spell` (",
        "  `spell_id` INT UNSIGNED NOT NULL,",
        "  `creature_entry` INT UNSIGNED NOT NULL,",
        "  `display_id` INT UNSIGNED NOT NULL,",
        "  `mount_type` VARCHAR(16) NOT NULL,",
        "  `source` VARCHAR(16) NOT NULL,",
        "  `name` VARCHAR(100) NOT NULL,",
        "  PRIMARY KEY (`spell_id`)",
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;",
        "",
        "DELETE FROM `karazhan_mount_collection_spell`;",
        "INSERT INTO `karazhan_mount_collection_spell`",
        "(`spell_id`,`creature_entry`,`display_id`,`mount_type`,`source`,`name`) VALUES",
        ",\n".join(
            f"({m['spell_id']},{m['creature_entry']},{m['display_id']},{sql_quote(m['mount_type'])},'custom',{sql_quote(m['name'])})"
            for m in mounts
        )
        + ";",
        "",
        "DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 940001 AND 940999;",
        "DELETE FROM `creature_template` WHERE `entry` BETWEEN 940001 AND 940999;",
        f"DELETE FROM `npc_vendor` WHERE `entry`={VENDOR_ENTRY} AND (`item`=960000 OR `item` BETWEEN 970001 AND 970999);",
        "DELETE FROM `item_template` WHERE `entry`=960000 OR `entry` BETWEEN 970001 AND 970999;",
        "",
    ]
    for mount in mounts:
        lines.append(build_item_insert(item_columns, mount))
        lines.append("")

    lines.extend(
        [
            "INSERT INTO `npc_vendor`",
            "(`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`VerifiedBuild`) VALUES",
            ",\n".join(
                f"({VENDOR_ENTRY},{mount['slot']},{mount['item_id']},0,0,0,12340)"
                for mount in mounts
            )
            + ";",
            "",
            f"-- Active custom spell range after rebuild: {SPELL_START}-{spell_end}",
            f"-- Active custom item range after rebuild: {ITEM_START}-{item_end}",
        ]
    )
    SQL_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    shutil.copy2(SQL_OUT, VENDOR_SQL_OUT)


def write_report(mounts):
    lines = [
        "# 카라잔 탈것 컬렉션 정정 결과",
        "",
        "- 기본 DBC display로 잘못 생성된 305개를 제외했습니다.",
        f"- custom display 기반 탈것만 남겼습니다: {len(mounts)}개",
        f"- 주문 범위: {SPELL_START}~{SPELL_START + len(mounts) - 1}",
        f"- 아이템 범위: {ITEM_START}~{ITEM_START + len(mounts) - 1}",
        "",
        "| 아이템 | 주문 | 크리처 | display | 구분 | 이름 | 모델 |",
        "|---:|---:|---:|---:|---|---|---|",
    ]
    for mount in mounts:
        lines.append(
            f"| {mount['item_id']} | {mount['spell_id']} | {mount['creature_entry']} | "
            f"{mount['display_id']} | {mount['mount_type']} | {mount['name']} | {mount['model_path']} |"
        )
    REPORT_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def verify_files():
    for path in [SQL_OUT, VENDOR_SQL_OUT, REPORT_OUT, DBC_DIR / "Spell.dbc.csv"]:
        text = path.read_text(encoding="utf-8-sig", errors="replace")
        if "\ufffd" in text:
            raise RuntimeError(f"replacement character found in {path}")


def main():
    mounts = load_custom_mounts()
    if len(mounts) != 168:
        raise RuntimeError(f"expected 168 custom mounts, got {len(mounts)}")
    rebuild_spell_dbc(mounts)
    write_sql(mounts)
    write_report(mounts)
    verify_files()
    print(f"custom_mounts={len(mounts)}")
    print(SQL_OUT)
    print(REPORT_OUT)


if __name__ == "__main__":
    main()
