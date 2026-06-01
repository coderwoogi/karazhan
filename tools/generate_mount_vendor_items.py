import subprocess
from pathlib import Path


ROOT = Path(r"E:\server\azerothcore-wotlk")
SQL_OUT = (
    ROOT
    / "modules"
    / "mod-item-karazhan"
    / "data"
    / "sql"
    / "db-world"
    / "updates"
    / "2026_05_31_01_karazhan_mount_collection_vendor.sql"
)

VENDOR_ENTRY = 190059
TEMPLATE_ITEM = 19872
OLD_COLLECTION_ITEM = 960000
ITEM_START = 970001


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


def load_mounts():
    rows = run_mysql(
        """
        SELECT `spell_id`, `creature_entry`, `display_id`, `mount_type`, `source`, `name`
        FROM `karazhan_mount_collection_spell`
        ORDER BY `spell_id`;
        """
    )
    mounts = []
    for index, line in enumerate(rows.splitlines()):
        if not line.strip():
            continue
        spell_id, creature_entry, display_id, mount_type, source, name = line.split("\t")
        mounts.append(
            {
                "item_id": ITEM_START + index,
                "slot": index + 1,
                "spell_id": int(spell_id),
                "creature_entry": int(creature_entry),
                "display_id": int(display_id),
                "mount_type": mount_type,
                "source": source,
                "name": name,
            }
        )
    return mounts


def item_insert(columns, mount):
    description = "사용 시 해당 탈것 소환 주문을 배웁니다."
    item_name = mount["name"]
    overrides = {
        "entry": str(mount["item_id"]),
        "class": "15",
        "subclass": "5",
        "name": sql_quote(item_name),
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
        "description": sql_quote(description),
        "ScriptName": "''",
        "VerifiedBuild": "12340",
    }
    values = [overrides.get(column, f"`{column}`") for column in columns]
    return (
        f"INSERT INTO `item_template` (`{'`,`'.join(columns)}`)\n"
        f"SELECT {', '.join(values)} FROM `item_template` WHERE `entry`={TEMPLATE_ITEM};"
    )


def main():
    columns = load_columns("item_template")
    mounts = load_mounts()
    if not mounts:
        raise RuntimeError("karazhan_mount_collection_spell has no rows")

    item_end = ITEM_START + len(mounts) - 1
    lines = [
        "-- Register one learn item per Karazhan mount spell on vendor 190059.",
        "SET NAMES utf8mb4;",
        "",
        f"DELETE FROM `npc_vendor` WHERE `entry`={VENDOR_ENTRY} AND (`item`={OLD_COLLECTION_ITEM} OR `item` BETWEEN {ITEM_START} AND {item_end});",
        f"DELETE FROM `item_template` WHERE `entry`={OLD_COLLECTION_ITEM} OR `entry` BETWEEN {ITEM_START} AND {item_end};",
        "",
    ]

    for mount in mounts:
        lines.append(item_insert(columns, mount))
        lines.append("")

    values = [
        f"({VENDOR_ENTRY},{mount['slot']},{mount['item_id']},0,0,0,12340)"
        for mount in mounts
    ]
    lines.extend(
        [
            "INSERT INTO `npc_vendor`",
            "(`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`VerifiedBuild`) VALUES",
            ",\n".join(values) + ";",
        ]
    )

    SQL_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    text = SQL_OUT.read_text(encoding="utf-8", errors="replace")
    if "\ufffd" in text:
        raise RuntimeError("replacement character found in generated SQL")
    print(f"items={len(mounts)} range={ITEM_START}-{item_end}")
    print(SQL_OUT)


if __name__ == "__main__":
    main()
