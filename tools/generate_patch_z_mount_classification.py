import csv
import struct
import subprocess
from collections import defaultdict
from pathlib import Path


ROOT = Path(r"E:\server\azerothcore-wotlk")
OUT_DIR = ROOT / "doc"
DBC_DIR = Path(r"E:\server\operate\data\dbc")


def read_dbc(path):
    data = path.read_bytes()
    magic, records, fields, rec_size, str_size = struct.unpack("<4sIIII", data[:20])
    if magic != b"WDBC":
        raise RuntimeError(f"not WDBC: {path}")

    rows = []
    for index in range(records):
        offset = 20 + index * rec_size
        rows.append(list(struct.unpack("<" + "I" * fields, data[offset : offset + rec_size])))

    string_base = 20 + records * rec_size
    strings = data[string_base : string_base + str_size]

    def get_string(offset):
        if offset == 0 or offset >= len(strings):
            return ""
        end = strings.find(b"\0", offset)
        if end < 0:
            end = len(strings)
        return strings[offset:end].decode("utf-8", "replace")

    return rows, get_string


def load_reflected_displays():
    sql = (
        "SELECT t.entry,t.name,m.CreatureDisplayID "
        "FROM acore_world.creature_template t "
        "JOIN acore_world.creature_template_model m ON m.CreatureID=t.entry "
        "WHERE t.entry BETWEEN 910001 AND 910084 OR t.entry BETWEEN 930001 AND 930084 "
        "ORDER BY t.entry;"
    )
    command = [
        "mysql",
        "--default-character-set=utf8mb4",
        "-uacore",
        "-pacore",
        "-N",
        "-B",
        "-e",
        sql,
    ]
    result = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )

    reflected = defaultdict(list)
    for line in result.stdout.splitlines():
        if not line.strip():
            continue
        parts = line.split("\t")
        if len(parts) < 3:
            continue
        entry, name, display_id = parts[0], parts[1], int(parts[2])
        reflected[display_id].append(f"{entry}:{name}")
    return reflected


MOUNT_KEYWORDS = [
    "mount",
    "riding",
    "gryphon",
    "wyvern",
    "drake",
    "dragon",
    "proto",
    "horse",
    "steed",
    "warhorse",
    "wolf",
    "ram",
    "kodo",
    "raptor",
    "sabre",
    "saber",
    "tiger",
    "lion",
    "bear",
    "mammoth",
    "elekk",
    "hawkstrider",
    "mechastrider",
    "chopper",
    "hog",
    "rocket",
    "carpet",
    "ray",
    "hippogryph",
    "phoenix",
    "serpent",
    "quilin",
    "crane",
    "turtle",
    "seahorse",
    "strider",
    "scorpion",
    "raven",
    "broom",
    "charger",
    "deathcharger",
    "frostwyrm",
    "windrider",
    "nether",
]

EXCLUDE_KEYWORDS = [
    "dragonspawn",
    "dragonwhelp",
    "bearcub",
    "druidbear",
    "druidowlbear",
    "sealion",
    "mountaingiant",
]

FLYING_KEYWORDS = [
    "alexstraszadragon",
    "dragon.mdx",
    "dragonazurgoz",
    "dragon2fixed",
    "drake",
    "drakemount",
    "protodragon",
    "proto",
    "netherdrake",
    "wyvern",
    "gryphon",
    "hippogryph",
    "dragonhawk",
    "phoenix",
    "frostwyrm",
    "frostbrood",
    "carpet",
    "rocket",
    "flying",
    "ray",
    "cosmicflyer",
    "celestialhorse",
    "winged",
    "tyrael",
    "hordezeppelin",
    "allianceship",
    "magemount",
    "stormcrow",
    "quilin",
    "serpentmount",
    "pandarenserpent",
    "faeriedragon",
    "darkphoenix",
    "redcrystaldragon",
    "invincible",
    "ebon",
    "windrider",
]

GROUND_KEYWORDS = [
    "bear",
    "horse",
    "deathknightmount",
    "direwolf",
    "wolf",
    "raptor",
    "ram",
    "kodo",
    "elekk",
    "hawkstrider",
    "mechastrider",
    "mammoth",
    "chopper",
    "hog",
    "frostsabre",
    "saber",
    "sabre",
    "tiger",
    "lion",
    "fox",
    "scorpion",
    "mushan",
    "monkmount",
    "moose",
    "ravenlord",
    "skeletalraptor",
    "siberiantiger",
    "crane",
    "paladinmount",
    "warhorse",
    "charger",
    "broommount",
    "chickenmount",
    "goblinshredder",
    "kodobeast",
    "viciouswarkodo",
]

AQUATIC_KEYWORDS = ["seahorse", "seaturtle", "turtlemount", "waterstrider"]

OVERRIDE = {
    r"creature\crane\cranemount.m2": ("지상탈", "모델명 CraneMount: 학 탈것 계열은 지상탈로 분류"),
    r"creature\monkmount\monkmount.m2": ("지상탈", "모델명 monkmount/Ban-Lu 계열"),
    r"creature\moosemount2nightmare\moosemount2nightmare.m2": ("지상탈", "모델명 moosemount 계열"),
    r"creature\mushanbeast\mushanbeastmount.m2": ("지상탈", "모델명 MushanBeast 계열"),
    r"creature\ravenlord\ravenlordmount.m2": ("지상탈", "까마귀 군주 계열은 WotLK 기준 지상탈"),
    r"creature\waterstrider\waterstridermount.m2": ("수상/지상", "Water Strider 계열"),
    r"creature\seahorse\seahorsemount.m2": ("수중탈", "Seahorse 계열"),
    r"creature\turtlemount\turtlemount.m2": ("수중/지상", "Sea Turtle/Turtle 계열"),
    r"creature\saber2\saber2mount.m2": ("지상탈", "Saber 계열"),
    r"creature\siberiantiger\siberiantigermount.m2": ("지상탈", "Tiger 계열"),
    r"creature\skeletalraptor\skeletalraptormount.m2": ("지상탈", "Raptor 계열"),
    r"creature\allianceshipmount\allianceshipmount.m2": ("날탈", "Skychaser/ship mount 계열"),
    r"creature\hordezeppelinmount\hordezeppelinmount.m2": ("날탈", "Zeppelin mount 계열"),
    r"creature\quilin\quilinflyingmount.m2": ("날탈", "모델명 QuilinFlyingMount"),
    r"creature\magemount_arcane\magemount_arcane.m2": ("날탈", "Mage disc mount 계열"),
    r"creature\magemount_fire\magemount_fire.m2": ("날탈", "Mage disc mount 계열"),
    r"creature\magemount_frost\magemount_frost.m2": ("날탈", "Mage disc mount 계열"),
}


def classify(path):
    low = path.replace("/", "\\").lower()
    if low in OVERRIDE:
        return OVERRIDE[low]
    if any(keyword in low for keyword in AQUATIC_KEYWORDS):
        return "수중/수상", "모델명 기준"
    if any(keyword in low for keyword in EXCLUDE_KEYWORDS):
        return "검토필요", "모델명은 탈것 후보이나 플레이어 탑승용 여부 불명확"
    if any(keyword in low for keyword in FLYING_KEYWORDS):
        return "날탈", "Wowhead WotLK 분류 및 비행 모델 계열명 기준"
    if any(keyword in low for keyword in GROUND_KEYWORDS):
        return "지상탈", "Wowhead WotLK 분류 및 지상 모델 계열명 기준"
    if any(keyword in low for keyword in MOUNT_KEYWORDS):
        return "검토필요", "탈것 키워드는 있으나 날탈/지상탈 확정 필요"
    return "비탈것/제외", "탈것 후보 키워드 부족"


def build_rows():
    model_rows, get_model_string = read_dbc(DBC_DIR / "CreatureModelData.dbc")
    display_rows, _ = read_dbc(DBC_DIR / "CreatureDisplayInfo.dbc")

    model_path = {row[0]: get_model_string(row[2]) for row in model_rows if get_model_string(row[2])}
    displays_by_model = defaultdict(list)
    for row in display_rows:
        displays_by_model[row[1]].append(row[0])

    reflected_by_display = load_reflected_displays()
    rows = []
    for model_id, path in sorted(model_path.items(), key=lambda item: (item[1].lower(), item[0])):
        low = path.replace("/", "\\").lower()
        displays = sorted(displays_by_model.get(model_id, []))
        if not displays:
            continue

        is_custom_model = 900001 <= model_id <= 900049
        is_mount_candidate = any(keyword in low for keyword in MOUNT_KEYWORDS) or is_custom_model
        if not is_mount_candidate:
            continue

        mount_type, reason = classify(path)
        reflected_entries = []
        for display_id in displays:
            reflected_entries.extend(reflected_by_display.get(display_id, []))

        reflected = "반영됨" if reflected_entries else "미반영"
        if reflected_entries:
            status_detail = "; ".join(reflected_entries[:6])
            if len(reflected_entries) > 6:
                status_detail += " ..."
        else:
            status_detail = "patch-Z DBC에는 있으나 910/930 탑승 크리처에는 미연결"

        display_ids = ",".join(map(str, displays[:20]))
        if len(displays) > 20:
            display_ids += " ..."

        rows.append(
            {
                "model_id": model_id,
                "model_path": path,
                "display_count": len(displays),
                "display_ids": display_ids,
                "reflected": reflected,
                "status_detail": status_detail,
                "mount_type": mount_type,
                "reason": reason,
            }
        )

    order = {"반영됨": 0, "미반영": 1}
    type_order = {
        "날탈": 0,
        "지상탈": 1,
        "수중탈": 2,
        "수상/지상": 2,
        "수중/지상": 2,
        "수중/수상": 2,
        "검토필요": 3,
        "비탈것/제외": 4,
    }
    rows.sort(
        key=lambda row: (
            order.get(row["reflected"], 9),
            type_order.get(row["mount_type"], 9),
            row["model_path"].lower(),
            row["model_id"],
        )
    )
    return rows


def write_outputs(rows):
    csv_path = OUT_DIR / "patch_z_mount_full_classification_ko.csv"
    with csv_path.open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "model_id",
                "model_path",
                "display_count",
                "display_ids",
                "reflected",
                "status_detail",
                "mount_type",
                "reason",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)

    summary = defaultdict(int)
    for row in rows:
        summary[(row["reflected"], row["mount_type"])] += 1

    order = {"반영됨": 0, "미반영": 1}
    type_order = {
        "날탈": 0,
        "지상탈": 1,
        "수중탈": 2,
        "수상/지상": 2,
        "수중/지상": 2,
        "수중/수상": 2,
        "검토필요": 3,
        "비탈것/제외": 4,
    }

    def escape(value):
        return str(value).replace("|", "/")

    lines = [
        "# patch-Z 전체 탈것 후보 분류표",
        "",
        "## 기준",
        "",
        "- 로컬 기준: `E:\\server\\operate\\data\\dbc\\CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`",
        "- 반영됨: `creature_template_model`에서 `910001~910084` 또는 `930001~930084`에 연결된 display ID",
        "- 미반영: `patch-Z` DBC에는 존재하지만 현재 910/930 탑승 크리처에 연결되지 않은 display/model",
        "- 날탈/지상탈: Wowhead WotLK Mount Type 분류와 모델 계열명을 기준으로 1차 분류",
        "",
        "## 요약",
        "",
        "| 반영상태 | 구분 | Model row 수 |",
        "|---|---|---:|",
    ]

    for key, count in sorted(
        summary.items(),
        key=lambda item: (order.get(item[0][0], 9), type_order.get(item[0][1], 9), item[0][1]),
    ):
        lines.append(f"| {key[0]} | {key[1]} | {count} |")

    lines += [
        "",
        "## 전체 표",
        "",
        "| No | ModelID | 모델 경로 | Display 수 | DisplayID | 반영상태 | 연결/상태 | 날탈/탈것 구분 | 판단 근거 |",
        "|---:|---:|---|---:|---|---|---|---|---|",
    ]

    for index, row in enumerate(rows, 1):
        lines.append(
            f"| {index} | {row['model_id']} | `{escape(row['model_path'])}` | "
            f"{row['display_count']} | `{escape(row['display_ids'])}` | {row['reflected']} | "
            f"{escape(row['status_detail'])} | {row['mount_type']} | {escape(row['reason'])} |"
        )

    md_path = OUT_DIR / "patch_z_mount_full_classification_ko.md"
    md_path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    return csv_path, md_path, summary


def main():
    rows = build_rows()
    csv_path, md_path, summary = write_outputs(rows)
    print(f"rows={len(rows)}")
    print(f"csv={csv_path}")
    print(f"md={md_path}")
    for key, count in sorted(summary.items()):
        print(f"{key[0]}\t{key[1]}\t{count}")


if __name__ == "__main__":
    main()
