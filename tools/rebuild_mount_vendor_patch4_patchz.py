import csv
import re
import shutil
import subprocess
from pathlib import Path


ROOT = Path(r"E:\server\azerothcore-wotlk")
DBC_DIR = Path(r"E:\server\data\karazhan\20260106")
SERVER_DBC_DIR = Path(r"E:\server\operate\data\dbc")
BUILD_DIR = Path(r"E:\server\tmp_mount_vendor_patch4_patchz")
DBCUTIL = DBC_DIR / "DBCUtil.exe"
MPQCLI = Path(r"E:\server\tools\mpqcli\mpqcli.exe")
PATCH_Z = Path(r"E:\server\3.3.5\Data\patch-Z.MPQ")
PATCH_KOKR4 = Path(r"E:\server\3.3.5\Data\koKR\patch-koKR-4.MPQ")

SQL_OUT = (
    ROOT
    / "modules"
    / "mod-item-karazhan"
    / "data"
    / "sql"
    / "db-world"
    / "updates"
    / "2026_05_31_03_mount_vendor_patch4_patchz.sql"
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
REPORT_OUT = ROOT / "doc" / "mount_vendor_patch4_patchz_ko.md"
REPORT_CSV = ROOT / "doc" / "mount_vendor_patch4_patchz_ko.csv"

VENDOR_ENTRY = 190059
TEMPLATE_ITEM = 19872
CREATURE_START_Z = 940001
SPELL_START = 960001
ITEM_START = 970001
MODEL_START_Z = 900050
DISPLAY_START_Z = 900185
GROUND_TEMPLATE_SPELL = 24242
FLYING_TEMPLATE_SPELL = 59567


STRICT_INCLUDE = (
    "mount",
    "riding",
    "pvp",
    "rocketmount",
    "carpetmount",
    "chopper",
    "hog",
    "gryphon_skeletal_mount",
)
STRICT_EXCLUDE = (
    "bearcub",
    "dragonspawn",
    "mountaingiant",
    "doodads",
    "goober",
    "spells\\",
    "serpent_totem",
    "dragonegg",
    "flow",
    "chainmounting",
    "nightdragon",
    "rocketlauncher",
    "vulpera",
    "druid",
    "sealion",
    "stonetreelog",
    "mounteddeathknight",
    "mountedknight",
    "mounteddemon",
    "terongorefiend",
)

GROUND_HINTS = (
    "bear",
    "direwolf",
    "wolf",
    "raptor",
    "horse",
    "warhorse",
    "paladin",
    "ram",
    "kodo",
    "elekk",
    "hawkstrider",
    "mechastrider",
    "saber",
    "sabre",
    "tiger",
    "frostsabre",
    "scorpion",
    "fox",
    "crane",
    "mushan",
    "monkmount",
    "moose",
    "ravenlord",
    "mammoth",
    "chopper",
    "hog",
    "seahorse",
    "turtle",
    "waterstrider",
    "silithid",
    "talbuk",
    "zebra",
    "shredder",
)

NAME_RULES = [
    ("alliancelionmount", "얼라이언스 사자"),
    ("alliancepvpmount", "사나운 전투사자"),
    ("allianceshipmount", "스톰윈드 하늘추적선"),
    ("celestialserpent", "천공의 운룡"),
    ("cranemount", "학 탈것"),
    ("darkphoenix", "암흑 불사조"),
    ("dragondeepholm", "심연 바위 비룡"),
    ("dragonhawkarmormountalliance", "얼라이언스 장갑 용매"),
    ("dragonhawkarmormounthorde", "호드 장갑 용매"),
    ("dragonhawkmountelite", "정예 용매"),
    ("dragonhawkmount", "용매"),
    ("faeriedragoncreature", "요정용"),
    ("faeriedragonmount", "마력 깃든 요정용"),
    ("felhound3_fire", "일리다리 지옥추적자"),
    ("felhound3_shadow", "일리다리 공포추적자"),
    ("felstalkermount", "지옥추적자"),
    ("firecatmount", "불꽃호랑이"),
    ("foxmount", "여우 탈것"),
    ("hordepvpmount", "사나운 전투여우"),
    ("hordescorpionmount", "사나운 전투전갈"),
    ("hordezeppelinmount", "호드 비행선"),
    ("magemount_arcane", "대마법사의 비전 원반"),
    ("magemount_fire", "대마법사의 화염 원반"),
    ("magemount_frost", "대마법사의 냉기 원반"),
    ("monkmount", "반루"),
    ("moosemount", "에체로의 영혼"),
    ("mushanbeast", "무산야수"),
    ("pandarenphoenix", "판다렌 불사조"),
    ("pandarenserpent", "운룡"),
    ("korkronprotodrake", "코르크론 원시비룡"),
    ("mdprotodrake", "원시비룡"),
    ("quilinflyingmount", "제국의 기렌"),
    ("ravenlord", "까마귀 군주"),
    ("reddrakemount", "붉은 비룡"),
    ("rocketmount4", "X-54 순회 로켓"),
    ("rocketmount3", "X-53 순회 로켓"),
    ("saber2mountsimple", "호랑이 탈것"),
    ("saber2mount", "호랑이 탈것"),
    ("scaleddrakemount", "비룡"),
    ("seahorsemount", "해마"),
    ("shadowstalkerpanther", "공허 수정 표범"),
    ("siberiantigermount", "겨울빙호"),
    ("skeletalraptor", "해골 랩터"),
    ("stormcrowmount_arcane", "비전 폭풍까마귀"),
    ("stormcrowmount_solar_low", "작은 태양 폭풍까마귀"),
    ("stormcrowmount_solar", "태양 폭풍까마귀"),
    ("stormcrowmount_low", "작은 폭풍까마귀"),
    ("stormcrowmount", "폭풍까마귀"),
    ("suramarmount", "수라마르 마나호랑이"),
    ("turtlemount", "바다거북"),
    ("tyraelmount", "티리엘의 군마"),
    ("voidelfhawkstrider", "공허타조"),
    ("waterstrider", "물꼬리"),
    ("wingedlionmount", "날개 달린 수호자"),
    ("amanibearmount", "아마니 전투곰"),
    ("batmounttaxi", "박쥐 탈것"),
    ("bearmountzulaman", "줄아만 전투곰"),
    ("bearmountaltnew", "전투곰"),
    ("cosmicraynew", "황천 가오리"),
    ("ridingdirewolf", "전투 늑대"),
    ("drakemount2grandalex", "알렉스트라자 거대 비룡"),
    ("drakemount2grandhalion", "할리온 거대 비룡"),
    ("drakemount2grandmalygos", "말리고스 거대 비룡"),
    ("drakemount2grandnefarian", "네파리안 거대 비룡"),
    ("drakemount2grandnozdormu", "노즈도르무 거대 비룡"),
    ("drakemount2grandonyxia", "오닉시아 거대 비룡"),
    ("drakemount2grandterac", "테라제인 거대 비룡"),
    ("drakemount2grandysera", "이세라 거대 비룡"),
    ("drakemount2grand", "거대 비룡"),
    ("drakemount2armored", "장갑 비룡"),
    ("drakemount2azure", "하늘빛 비룡"),
    ("drakemount2elite", "정예 비룡"),
    ("drakemount2", "비룡"),
    ("elekkdraenormount", "드레나이 엘레크"),
    ("pvpridingraptor", "전투 랩터"),
    ("nightsaber2mountarmored", "장갑 밤호랑이"),
    ("spectraltiger2mountarmored", "장갑 유령호랑이"),
    ("ridingfrostsabre2", "겨울빙호"),
    ("pvpridingfrostsabre", "전투 겨울빙호"),
    ("ridingfrostsabrenaked", "안장 없는 겨울빙호"),
    ("ridingfrostsabre", "겨울빙호"),
    ("goblinshreddermount", "고블린 벌목기"),
    ("gryphon_armoredmount", "장갑 그리핀"),
    ("gryphonmount", "그리핀"),
    ("gryphon_mount", "그리핀"),
    ("hawkstrider2_mount_noarmor", "날쌘 매타조"),
    ("hawkstrider2_mount", "매타조"),
    ("headlesshorsemanmount", "저주받은 기사의 군마"),
    ("hippogryph2mountarmored", "장갑 히포그리프"),
    ("hippogryph2mount", "히포그리프"),
    ("horse2_zebramount2", "얼룩말"),
    ("humanridinghorse2", "인간 군마"),
    ("kodobeastpvpt2", "전투 코도"),
    ("kodobeast2pack", "짐 싣는 코도"),
    ("kodobeast2mount", "코도"),
    ("pvpmechastrider", "전투 기계타조"),
    ("northrendbearmount2", "노스렌드 장갑곰"),
    ("northrendbearmountblizzcon", "블리즈컨 전투곰"),
    ("northrendbearmountarmored", "노스렌드 전투곰"),
    ("stormdragonmount2", "폭풍 비룡"),
    ("olddrakemount", "고대 비룡"),
    ("paladinwarhorse_slow2", "성기사 군마"),
    ("paladinwarhorse_slow", "성기사 군마"),
    ("paladinwarhorse", "성기사 전투마"),
    ("ridingrambrew", "가을축제 산양"),
    ("ridingram", "산양"),
    ("horse_sattel1", "안장 군마"),
    ("horse_sattel2", "안장 군마"),
    ("packmule", "짐노새"),
    ("ridinghorsepvpt2_noshield", "전투 군마"),
    ("ridinghorsepvpt2", "전투 군마"),
    ("ridinghorse", "군마"),
    ("zulgurubraptor", "줄구룹 랩터"),
    ("ridingraptor", "랩터"),
    ("ridingsilithid", "실리시드"),
    ("ridingtalbukepic", "날쌘 탈부크"),
    ("ridingtalbuk", "탈부크"),
    ("ridingwyvernarmored", "장갑 와이번"),
    ("wyvern_armored_vehicle", "장갑 와이번"),
    ("ridingwyvern", "와이번"),
    ("wyvern_mount", "와이번"),
    ("wyvern.m2", "와이번"),
    ("turkeymount", "칠면조 탈것"),
    ("ridingundeadwarhorse", "언데드 전투마"),
    ("ridingundeadhorse", "언데드 군마"),
    ("pvpwarhorse2", "전투 군마"),
    ("pvpwarhorse", "전투 군마"),
    ("zebramount", "얼룩말 탈것"),
]


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


def run_mysql(sql):
    return run(
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
        ]
    )


def sql_quote(value):
    return "'" + str(value).replace("\\", "\\\\").replace("'", "''") + "'"


def normalize_path(path):
    return path.replace("/", "\\").lower()


def is_strict_mount_model(path):
    low = normalize_path(path)
    if not (low.startswith("creature\\") and (low.endswith(".m2") or low.endswith(".mdx"))):
        return False
    if any(exclude in low for exclude in STRICT_EXCLUDE):
        return False
    return any(include in low for include in STRICT_INCLUDE)


def mount_type_for_path(path):
    low = normalize_path(path)
    if any(hint in low for hint in GROUND_HINTS):
        return "지상탈"
    return "날탈"


def base_name_from_path(path):
    low = normalize_path(path)
    for key, name in NAME_RULES:
        if key in low:
            return name

    stem = Path(path.replace("\\", "/")).stem
    stem = re.sub(r"(?<!^)([A-Z])", r" \1", stem)
    stem = re.sub(r"[_\-]+", " ", stem)
    stem = re.sub(r"\s+", " ", stem).strip()
    return f"카라잔 탈것 {stem}" if stem else "카라잔 탈것"


def unique_names(records):
    counts = {}
    for record in records:
        base = record["base_name"]
        counts[base] = counts.get(base, 0) + 1
        record["name"] = base if counts[base] == 1 else f"{base} #{counts[base]}"


def load_table_columns(table):
    rows = run_mysql(f"SHOW COLUMNS FROM `{table}`;")
    return [line.split("\t", 1)[0] for line in rows.splitlines() if line.strip()]


def load_model_rows():
    with (DBC_DIR / "CreatureModelData.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    return rows[0], rows[1:]


def load_display_rows():
    with (DBC_DIR / "CreatureDisplayInfo.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    return rows[0], rows[1:]


def load_patch_z_models():
    output = run([str(MPQCLI), "list", str(PATCH_Z)])
    models = set()
    for line in output.splitlines():
        path = line.strip()
        if is_strict_mount_model(path):
            models.add(path)
    return sorted(models, key=normalize_path)


def build_records():
    model_header, model_rows = load_model_rows()
    display_header, display_rows = load_display_rows()

    model_path_by_id = {}
    model_id_by_path = {}
    for row in model_rows:
        if row and row[0].isdigit() and len(row) > 2:
            model_id = int(row[0])
            path = row[2]
            model_path_by_id[model_id] = path
            model_id_by_path[normalize_path(path)] = model_id

    displays_by_model = {}
    for row in display_rows:
        if row and row[0].isdigit() and row[1].isdigit():
            display_id = int(row[0])
            model_id = int(row[1])
            if 900001 <= display_id <= 900184 and 900001 <= model_id <= 900049:
                displays_by_model.setdefault(model_id, []).append(display_id)

    creature_by_display = {}
    rows = run_mysql(
        """
        SELECT t.entry,t.name,m.CreatureDisplayID
        FROM creature_template t
        JOIN creature_template_model m ON m.CreatureID=t.entry
        WHERE m.CreatureDisplayID BETWEEN 900001 AND 900184
        ORDER BY t.entry;
        """
    )
    for line in rows.splitlines():
        if not line.strip():
            continue
        entry, _name, display = line.split("\t")
        creature_by_display.setdefault(int(display), int(entry))

    records = []
    for model_id in sorted(displays_by_model):
        display_id = min(displays_by_model[model_id])
        model_path = model_path_by_id[model_id]
        records.append(
            {
                "source": "patch-4",
                "model_id": model_id,
                "display_id": display_id,
                "creature_entry": creature_by_display.get(display_id, 910000 + len(records) + 1),
                "model_path": model_path,
                "mount_type": mount_type_for_path(model_path),
                "base_name": base_name_from_path(model_path),
            }
        )

    patch_z_models = load_patch_z_models()
    existing_paths = {normalize_path(record["model_path"]) for record in records}
    new_paths = [path for path in patch_z_models if normalize_path(path) not in existing_paths]

    for index, model_path in enumerate(new_paths):
        records.append(
            {
                "source": "patch-Z",
                "model_id": MODEL_START_Z + index,
                "display_id": DISPLAY_START_Z + index,
                "creature_entry": CREATURE_START_Z + index,
                "model_path": model_path,
                "mount_type": mount_type_for_path(model_path),
                "base_name": base_name_from_path(model_path),
            }
        )

    unique_names(records)
    for index, record in enumerate(records):
        record["spell_id"] = SPELL_START + index
        record["item_id"] = ITEM_START + index
        record["slot"] = index + 1
    return records, model_header, model_rows, display_header, display_rows


def rebuild_creature_dbcs(records, model_header, model_rows, display_header, display_rows):
    model_rows = [
        row
        for row in model_rows
        if not (row and row[0].isdigit() and MODEL_START_Z <= int(row[0]) <= MODEL_START_Z + 999)
    ]
    display_rows = [
        row
        for row in display_rows
        if not (row and row[0].isdigit() and DISPLAY_START_Z <= int(row[0]) <= DISPLAY_START_Z + 999)
    ]

    model_template = next(row for row in model_rows if row and row[0] == "900049")
    display_template = next(row for row in display_rows if row and row[0] == "900001")

    for record in records:
        if record["source"] != "patch-Z":
            continue
        model_row = model_template.copy()
        model_row[0] = str(record["model_id"])
        model_row[2] = record["model_path"]
        model_rows.append(model_row)

        display_row = display_template.copy()
        display_row[0] = str(record["display_id"])
        display_row[1] = str(record["model_id"])
        for idx in range(6, 10):
            display_row[idx] = ""
        display_rows.append(display_row)

    with (DBC_DIR / "CreatureModelData.dbc.csv").open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(model_header)
        writer.writerows(model_rows)

    with (DBC_DIR / "CreatureDisplayInfo.dbc.csv").open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(display_header)
        writer.writerows(display_rows)


def load_spell_rows():
    with (DBC_DIR / "Spell.dbc.csv").open("r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    header = rows[0]
    data = [
        row
        for row in rows[1:]
        if row and not (SPELL_START <= int(row[0]) <= SPELL_START + 999)
    ]
    return header, data


def set_localized_spell_text(row, name, description, aura):
    row[136] = ""
    row[137] = name
    for idx in range(138, 152):
        row[idx] = ""
    row[152] = "0xFF01FE"

    row[170] = ""
    row[171] = description
    for idx in range(172, 186):
        row[idx] = ""
    row[186] = "0xFF01FE"

    row[187] = ""
    row[188] = aura
    for idx in range(189, 203):
        row[idx] = ""
    row[203] = "0xFF01FE"


def rebuild_spell_dbc(records):
    header, data = load_spell_rows()
    templates = {int(row[0]): row for row in data if int(row[0]) in {GROUND_TEMPLATE_SPELL, FLYING_TEMPLATE_SPELL}}
    new_rows = []
    for record in records:
        is_flying = record["mount_type"] == "날탈"
        row = templates[FLYING_TEMPLATE_SPELL if is_flying else GROUND_TEMPLATE_SPELL].copy()
        row[0] = str(record["spell_id"])
        row[110] = str(record["creature_entry"])
        if is_flying:
            desc = f"{record['name']}에 올라타거나 내립니다. 지상 및 비행 속도가 증가합니다."
            aura = "지상 이동 속도 100%, 비행 이동 속도 280%만큼 증가"
        else:
            desc = f"{record['name']}에 올라타거나 내립니다. 지상 이동 속도가 증가합니다."
            aura = "지상 이동 속도 100%만큼 증가"
        set_localized_spell_text(row, record["name"], desc, aura)
        new_rows.append(row)

    with (DBC_DIR / "Spell.dbc.csv").open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(header)
        writer.writerows(data)
        writer.writerows(new_rows)


def convert_dbc(csv_name):
    temp = BUILD_DIR / csv_name
    temp.parent.mkdir(parents=True, exist_ok=True)
    if temp.exists():
        temp.unlink()
    dbc = temp.with_suffix("")
    if dbc.exists():
        dbc.unlink()
    shutil.copy2(DBC_DIR / csv_name, temp)
    run([str(DBCUTIL), str(temp)], cwd=str(BUILD_DIR))
    return BUILD_DIR / csv_name.replace(".csv", "")


def convert_and_copy_dbcs():
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    BUILD_DIR.mkdir(parents=True)
    for csv_name in ["CreatureModelData.dbc.csv", "CreatureDisplayInfo.dbc.csv", "Spell.dbc.csv"]:
        generated = convert_dbc(csv_name)
        target_name = csv_name.replace(".csv", "")
        shutil.copy2(generated, DBC_DIR / target_name)
        shutil.copy2(generated, SERVER_DBC_DIR / target_name)


def build_creature_insert(columns, record):
    values = []
    for column in columns:
        if column == "entry":
            values.append(str(record["creature_entry"]))
        elif column == "name":
            values.append(sql_quote(record["name"]))
        elif column == "subname":
            values.append(sql_quote("The Karazhan"))
        elif column == "VerifiedBuild":
            values.append("12340")
        else:
            values.append(f"`{column}`")
    return (
        f"INSERT INTO `creature_template` (`{'`,`'.join(columns)}`)\n"
        f"SELECT {', '.join(values)} FROM `creature_template` WHERE `entry`=910001;"
    )


def build_item_insert(columns, record):
    overrides = {
        "entry": str(record["item_id"]),
        "class": "15",
        "subclass": "5",
        "name": sql_quote(record["name"]),
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
        "spellid_2": str(record["spell_id"]),
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


def write_sql(records):
    creature_columns = load_table_columns("creature_template")
    item_columns = load_table_columns("item_template")
    lines = [
        "-- Rebuild mount vendor with deduplicated patch-4 mounts and patch-Z mounts.",
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
        "DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 940001 AND 940999;",
        "DELETE FROM `creature_template` WHERE `entry` BETWEEN 940001 AND 940999;",
        f"DELETE FROM `npc_vendor` WHERE `entry`={VENDOR_ENTRY} AND (`item`=960000 OR `item` BETWEEN 970001 AND 970999);",
        "DELETE FROM `item_template` WHERE `entry`=960000 OR `entry` BETWEEN 970001 AND 970999;",
        "",
    ]

    for record in records:
        if record["source"] != "patch-Z":
            continue
        lines.append(build_creature_insert(creature_columns, record))
        lines.append(
            "INSERT INTO `creature_template_model` "
            "(`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`) "
            f"VALUES ({record['creature_entry']},0,{record['display_id']},1,1,12340);"
        )
        lines.append("")

    values = [
        "("
        f"{r['spell_id']},{r['creature_entry']},{r['display_id']},"
        f"{sql_quote(r['mount_type'])},{sql_quote(r['source'])},{sql_quote(r['name'])}"
        ")"
        for r in records
    ]
    lines.append(
        "INSERT INTO `karazhan_mount_collection_spell` "
        "(`spell_id`,`creature_entry`,`display_id`,`mount_type`,`source`,`name`) VALUES\n"
        + ",\n".join(values)
        + ";"
    )
    lines.append("")

    for record in records:
        lines.append(build_item_insert(item_columns, record))
        lines.append("")

    vendor_values = [
        f"({VENDOR_ENTRY},{r['slot']},{r['item_id']},0,0,0,12340)" for r in records
    ]
    lines.append(
        "INSERT INTO `npc_vendor` "
        "(`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`VerifiedBuild`) VALUES\n"
        + ",\n".join(vendor_values)
        + ";"
    )
    SQL_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    shutil.copy2(SQL_OUT, VENDOR_SQL_OUT)


def write_reports(records):
    with REPORT_CSV.open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "slot",
                "source",
                "item_id",
                "spell_id",
                "creature_entry",
                "display_id",
                "model_id",
                "mount_type",
                "name",
                "model_path",
            ],
            extrasaction="ignore",
        )
        writer.writeheader()
        writer.writerows(records)

    patch4 = sum(1 for r in records if r["source"] == "patch-4")
    patchz = sum(1 for r in records if r["source"] == "patch-Z")
    flying = sum(1 for r in records if r["mount_type"] == "날탈")
    ground = sum(1 for r in records if r["mount_type"] == "지상탈")
    lines = [
        "# 190059 탈것 판매 최종 정리",
        "",
        f"- 총 탈것 아이템: {len(records)}개",
        f"- patch-4 중복 제거 후 유지: {patch4}개",
        f"- patch-Z 신규 추가: {patchz}개",
        f"- 날탈: {flying}개",
        f"- 지상탈: {ground}개",
        f"- 아이템 범위: {ITEM_START}~{ITEM_START + len(records) - 1}",
        f"- 주문 범위: {SPELL_START}~{SPELL_START + len(records) - 1}",
        "",
        "| 순서 | 출처 | 아이템 | 주문 | 크리처 | display | 구분 | 한글 이름 | 모델 |",
        "|---:|---|---:|---:|---:|---:|---|---|---|",
    ]
    for r in records:
        lines.append(
            f"| {r['slot']} | {r['source']} | {r['item_id']} | {r['spell_id']} | "
            f"{r['creature_entry']} | {r['display_id']} | {r['mount_type']} | {r['name']} | {r['model_path']} |"
        )
    REPORT_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def update_mpq(mpq_path, files):
    temp_mpq = BUILD_DIR / mpq_path.name
    shutil.copy2(mpq_path, temp_mpq)
    for source, archive_path in files:
        run(
            [
                str(MPQCLI),
                "add",
                "-w",
                "-g",
                "wow-wotlk",
                "-p",
                archive_path,
                str(source),
                str(temp_mpq),
            ]
        )
    shutil.copy2(temp_mpq, mpq_path)


def verify(records):
    for path in [SQL_OUT, VENDOR_SQL_OUT, REPORT_OUT, REPORT_CSV, DBC_DIR / "Spell.dbc.csv"]:
        text = path.read_text(encoding="utf-8-sig", errors="replace")
        if "\ufffd" in text:
            raise RuntimeError(f"replacement character found in {path}")
    if len(records) != 137:
        raise RuntimeError(f"expected 137 records, got {len(records)}")


def main():
    records, model_header, model_rows, display_header, display_rows = build_records()
    rebuild_creature_dbcs(records, model_header, model_rows, display_header, display_rows)
    rebuild_spell_dbc(records)
    convert_and_copy_dbcs()
    write_sql(records)
    write_reports(records)
    verify(records)
    print(f"total={len(records)}")
    print(f"patch4={sum(1 for r in records if r['source'] == 'patch-4')}")
    print(f"patchZ={sum(1 for r in records if r['source'] == 'patch-Z')}")
    print(SQL_OUT)
    print(REPORT_OUT)


if __name__ == "__main__":
    main()
