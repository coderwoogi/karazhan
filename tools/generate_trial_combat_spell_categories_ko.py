import html
import json
import re
import subprocess
import urllib.request
from collections import defaultdict
from pathlib import Path


ROOT = Path(r"E:\server\azerothcore-wotlk")
OUT_MD = ROOT / "doc/wotlk_335_trial_combat_spell_categories_ko.md"
OUT_HTML = ROOT / "doc/wotlk_335_trial_combat_spell_categories_ko.html"

WOWHEAD_BASE = "https://www.wowhead.com/wotlk/ko/spells/abilities"
CLASS_DATA = [
    (1, "전사", "Warrior", "warrior"),
    (2, "성기사", "Paladin", "paladin"),
    (3, "사냥꾼", "Hunter", "hunter"),
    (4, "도적", "Rogue", "rogue"),
    (5, "사제", "Priest", "priest"),
    (6, "죽음의 기사", "Death Knight", "death-knight"),
    (7, "주술사", "Shaman", "shaman"),
    (8, "마법사", "Mage", "mage"),
    (9, "흑마법사", "Warlock", "warlock"),
    (11, "드루이드", "Druid", "druid"),
]

CATEGORIES = [
    ("PRECOMBAT_BUFF", "전투 전 버프/준비", "전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다."),
    ("STANCE_FORM_AURA", "태세/형상/오라/폼 전환", "직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다."),
    ("ATTACK_SINGLE", "전투 중 공격기 - 단일 대상", "주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다."),
    ("ATTACK_AOE", "전투 중 공격기 - 광역/다중 대상", "여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다."),
    ("CC", "전투 중 CC/메즈/이동 제한", "기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다."),
    ("INTERRUPT", "차단/침묵/시전 방해", "주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다."),
    ("DISPEL_CLEANSE", "해제/정화", "아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다."),
    ("DEFENSIVE", "생존/방어/피해 감소", "자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다."),
    ("HEAL", "치유/회복", "체력 회복, 보호막성 회복, 생명력 회복 계열입니다."),
    ("RESURRECTION", "부활", "죽은 아군을 살리는 기술입니다. 전투 중 사용 가능 여부는 스펠별로 다릅니다."),
    ("RESOURCE", "마나/자원 회복", "마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다."),
    ("MOBILITY", "이동/돌진/도주", "돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다."),
    ("TAUNT_THREAT", "어그로/도발/위협 제어", "대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다."),
    ("PET_SUMMON", "소환/펫/하수인", "전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다."),
    ("UTILITY_COMBAT", "전투 유틸/특수 상황", "위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다."),
    ("EXCLUDE_NONCOMBAT", "전투 사용 제외 후보", "순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다."),
]

CATEGORY_TITLE = {key: title for key, title, _ in CATEGORIES}
CATEGORY_DESC = {key: desc for key, _, desc in CATEGORIES}

NAME_CATEGORY_OVERRIDES = {
    "부활": "RESURRECTION",
    "환생": "RESURRECTION",
    "되살리기": "RESURRECTION",
    "환기": "RESOURCE",
    "마나석 창조": "RESOURCE",
    "생명력 전환": "RESOURCE",
    "피의 분노": "RESOURCE",
    "광전사의 격노": "RESOURCE",
    "정신 자극": "RESOURCE",
    "혈기 전환": "RESOURCE",
    "룬 무기 강화": "RESOURCE",
    "돌진": "MOBILITY",
    "봉쇄": "MOBILITY",
    "점멸": "MOBILITY",
    "전력 질주": "MOBILITY",
    "철수": "MOBILITY",
    "야성의 돌진": "MOBILITY",
    "도발": "TAUNT_THREAT",
    "도전의 외침": "TAUNT_THREAT",
    "도발의 일격": "TAUNT_THREAT",
    "심판의 손길": "TAUNT_THREAT",
    "어둠의 명령": "TAUNT_THREAT",
    "포효": "TAUNT_THREAT",
    "정의의 방어": "TAUNT_THREAT",
    "눈속임": "TAUNT_THREAT",
    "자루 공격": "INTERRUPT",
    "방패 가격": "INTERRUPT",
    "마법 차단": "INTERRUPT",
    "발차기": "INTERRUPT",
    "정신 얼리기": "INTERRUPT",
    "날카로운 바람": "INTERRUPT",
    "대지 충격": "INTERRUPT",
    "주문 잠금": "INTERRUPT",
    "위협의 외침": "CC",
    "무장 해제": "CC",
    "심판의 망치": "CC",
    "변이": "CC",
    "얼음 회오리": "CC",
    "공포": "CC",
    "추방": "CC",
    "현혹": "CC",
    "비열한 습격": "CC",
    "급소 가격": "CC",
    "실명": "CC",
    "혼절시키기": "CC",
    "후려치기": "CC",
    "목조르기": "CC",
    "동결의 덫": "CC",
    "동물 겁주기": "CC",
    "회오리바람": "CC",
    "휘감는 뿌리": "CC",
    "겨울잠": "CC",
    "주술": "CC",
    "서리 충격": "CC",
    "얼음 결계": "CC",
    "죽음의 손아귀": "CC",
    "질식시키기": "CC",
    "방패의 벽": "DEFENSIVE",
    "방패 막기": "DEFENSIVE",
    "주문 반사": "DEFENSIVE",
    "최후의 저항": "DEFENSIVE",
    "천상의 보호막": "DEFENSIVE",
    "신의 가호": "DEFENSIVE",
    "보호의 손길": "DEFENSIVE",
    "희생의 손길": "DEFENSIVE",
    "자유의 손길": "DEFENSIVE",
    "얼음 방패": "DEFENSIVE",
    "마나 보호막": "DEFENSIVE",
    "냉기계 수호": "DEFENSIVE",
    "화염계 수호": "DEFENSIVE",
    "얼음 보호막": "DEFENSIVE",
    "얼음같은 인내력": "DEFENSIVE",
    "대마법 보호막": "DEFENSIVE",
    "흡혈": "DEFENSIVE",
    "회피": "DEFENSIVE",
    "그림자 망토": "DEFENSIVE",
    "소멸": "DEFENSIVE",
    "분산": "DEFENSIVE",
    "나무 껍질": "DEFENSIVE",
    "광포한 재생력": "DEFENSIVE",
    "주술의 분노": "DEFENSIVE",
    "성스러운 빛": "HEAL",
    "빛의 섬광": "HEAL",
    "신의 축복": "HEAL",
    "치유": "HEAL",
    "순간 치유": "HEAL",
    "상급 치유": "HEAL",
    "소생": "HEAL",
    "회복의 기원": "HEAL",
    "치유의 기원": "HEAL",
    "결속의 치유": "HEAL",
    "급속 성장": "HEAL",
    "회복": "HEAL",
    "재생": "HEAL",
    "피어나는 생명": "HEAL",
    "치유의 손길": "HEAL",
    "육성": "HEAL",
    "신속한 치유": "HEAL",
    "치유의 물결": "HEAL",
    "하급 치유의 물결": "HEAL",
    "연쇄 치유": "HEAL",
    "마법 무효화": "DISPEL_CLEANSE",
    "마법 해제": "DISPEL_CLEANSE",
    "질병 해제": "DISPEL_CLEANSE",
    "질병 치료": "DISPEL_CLEANSE",
    "독 해제": "DISPEL_CLEANSE",
    "독소 해제": "DISPEL_CLEANSE",
    "정화": "DISPEL_CLEANSE",
    "마법 훔치기": "DISPEL_CLEANSE",
    "저주 해제": "DISPEL_CLEANSE",
    "질병 정화": "DISPEL_CLEANSE",
    "독 정화": "DISPEL_CLEANSE",
    "소환": "PET_SUMMON",
    "야수 부르기": "PET_SUMMON",
    "야수 되살리기": "PET_SUMMON",
    "야수 치료": "HEAL",
    "물의 정령 소환": "PET_SUMMON",
    "구울 되살리기": "PET_SUMMON",
    "시체 되살리기": "PET_SUMMON",
    "사자의 군대": "PET_SUMMON",
    "불정령 토템": "PET_SUMMON",
    "대지의 정령 토템": "PET_SUMMON",
    "늑대 정령": "PET_SUMMON",
    "토템": "PET_SUMMON",
    "임프 소환": "PET_SUMMON",
    "보이드워커 소환": "PET_SUMMON",
    "서큐버스 소환": "PET_SUMMON",
    "지옥사냥개 소환": "PET_SUMMON",
    "지옥수호병 소환": "PET_SUMMON",
    "지옥마 소환": "EXCLUDE_NONCOMBAT",
    "공포마 소환": "EXCLUDE_NONCOMBAT",
    "전투 태세": "STANCE_FORM_AURA",
    "광폭 태세": "STANCE_FORM_AURA",
    "방어 태세": "STANCE_FORM_AURA",
    "곰 변신": "STANCE_FORM_AURA",
    "광포한 곰 변신": "STANCE_FORM_AURA",
    "표범 변신": "STANCE_FORM_AURA",
    "생명의 나무": "STANCE_FORM_AURA",
    "오라": "STANCE_FORM_AURA",
    "문장": "STANCE_FORM_AURA",
    "정의의 격노": "TAUNT_THREAT",
    "피의 형상": "STANCE_FORM_AURA",
    "냉기의 형상": "STANCE_FORM_AURA",
    "부정의 형상": "STANCE_FORM_AURA",
    "은신": "STANCE_FORM_AURA",
    "내면의 열정": "PRECOMBAT_BUFF",
    "전투의 외침": "PRECOMBAT_BUFF",
    "지휘의 외침": "PRECOMBAT_BUFF",
    "신비한 지능": "PRECOMBAT_BUFF",
    "신의 권능: 인내": "PRECOMBAT_BUFF",
    "인내의 기원": "PRECOMBAT_BUFF",
    "어둠의 보호": "PRECOMBAT_BUFF",
    "천상의 정신": "PRECOMBAT_BUFF",
    "야생의 징표": "PRECOMBAT_BUFF",
    "야생의 선물": "PRECOMBAT_BUFF",
    "가시": "PRECOMBAT_BUFF",
    "대지의 보호막": "PRECOMBAT_BUFF",
    "번개 보호막": "PRECOMBAT_BUFF",
    "물의 보호막": "PRECOMBAT_BUFF",
    "불꽃의 무기": "PRECOMBAT_BUFF",
    "대지생명의 무기": "PRECOMBAT_BUFF",
    "질풍의 무기": "PRECOMBAT_BUFF",
    "냉기의 무기": "PRECOMBAT_BUFF",
    "대지의 무기": "PRECOMBAT_BUFF",
    "마의 갑옷": "PRECOMBAT_BUFF",
    "악마의 피부": "PRECOMBAT_BUFF",
    "악마의 갑옷": "PRECOMBAT_BUFF",
    "공포의 수호물": "PRECOMBAT_BUFF",
    "얼음 갑옷": "PRECOMBAT_BUFF",
    "마법사 갑옷": "PRECOMBAT_BUFF",
    "타오르는 갑옷": "PRECOMBAT_BUFF",
    "타락한 성전사의 룬": "PRECOMBAT_BUFF",
    "전쟁노래": "PRECOMBAT_BUFF",
}

EXCLUDE_KEYWORDS = [
    "순간이동", "차원문", "귀환", "수중 호흡", "수중 걷기", "천리안", "야수 연구",
    "야수의 눈", "훔치기", "함정 해제", "자물쇠 따기", "주문석 창조", "화염석 창조",
    "영혼석 창조", "의식", "킬로그의 눈", "투명체 감지", "저속 낙하", "마나석 창조",
    "추적", "순찰", "먹이주기", "야수 길들이기", "야수 소환 해제",
]

AOE_KEYWORDS = [
    "회전베기", "소용돌이", "천둥벼락", "신성화", "신성한 폭발", "죽음과 부패",
    "피의 소용돌이", "연쇄 번개", "불꽃 회오리", "눈보라", "불기둥", "화염 폭풍",
    "신비한 폭발", "지옥의 불길", "불의 비", "씨앗", "휘둘러치기", "폭풍", "일제 사격",
    "폭발의 덫", "연발 사격", "부채",
]

ATTACK_HINTS = [
    "일격", "강타", "베기", "가격", "사격", "쐐기", "화살", "작렬", "불태우기", "화염구",
    "얼음창", "얼음 화살", "번개", "충격", "고통", "채찍", "정신 분열", "정신의 쐐기",
    "성스러운 일격", "신성한 불꽃", "역병", "죽음의 고리", "제물", "부패", "저주",
    "흡수", "절단", "독살", "파열", "기습", "사악한 일격", "후려치기", "분쇄",
    "퇴마술", "심판", "응징의 방패", "천벌", "별빛 화살", "달빛 섬광", "요정의 불꽃",
    "휘둘러치기", "도려내기", "짓이기기", "갈퀴 발톱", "찢어발기기", "살쾡이의 이빨",
]


def extract_listview_spells(page):
    match = re.search(r"var listviewspells = (\[.*?\]);", page, re.S)
    if not match:
        raise RuntimeError("Wowhead listviewspells 데이터를 찾을 수 없습니다.")

    parser = """
const fs = require('fs');
const data = Function('return ' + fs.readFileSync(0, 'utf8'))();
console.log(JSON.stringify(data));
"""
    result = subprocess.run(
        ["node", "-e", parser],
        input=match.group(1),
        text=True,
        capture_output=True,
        encoding="utf-8",
        check=True,
    )
    return json.loads(result.stdout)


def fetch_wowhead_spells():
    spells = []
    for class_id, class_name, class_en, slug in CLASS_DATA:
        url = f"{WOWHEAD_BASE}/{slug}"
        with urllib.request.urlopen(url) as response:
            page = response.read().decode("utf-8")

        for row in extract_listview_spells(page):
            name = str(row.get("name", "")).strip()
            if not name:
                continue

            spells.append({
                "class_id": class_id,
                "class_name": class_name,
                "class_en": class_en,
                "slug": slug,
                "source": "Wowhead",
                "url": f"https://www.wowhead.com/wotlk/ko/spell={row['id']}",
                "spell_id": int(row["id"]),
                "name": name,
                "rank": str(row.get("rank", "")).strip(),
                "level": int(row.get("level") or 0),
                "schools": int(row.get("schools") or 0),
                "source_flags": row.get("source", []),
            })

    return [select_highest_rank(group) for group in group_by_name(spells)]


def group_by_name(spells):
    grouped = defaultdict(list)
    for spell in spells:
        grouped[(spell["class_id"], spell["name"])].append(spell)
    return grouped.values()


def rank_number(rank):
    match = re.search(r"(\d+)\s*레벨", rank or "")
    return int(match.group(1)) if match else 0


def select_highest_rank(rows):
    return max(rows, key=lambda row: (row["level"], rank_number(row["rank"]), row["spell_id"]))


def classify(spell):
    name = spell["name"]

    for keyword in EXCLUDE_KEYWORDS:
        if keyword in name:
            return "EXCLUDE_NONCOMBAT"

    if name in NAME_CATEGORY_OVERRIDES:
        return NAME_CATEGORY_OVERRIDES[name]

    for keyword, category in NAME_CATEGORY_OVERRIDES.items():
        if keyword in name:
            return category

    if "토템" in name:
        return "PET_SUMMON"

    if any(keyword in name for keyword in AOE_KEYWORDS):
        return "ATTACK_AOE"

    if any(keyword in name for keyword in ATTACK_HINTS):
        return "ATTACK_SINGLE"

    if any(keyword in name for keyword in ["치유", "회복", "소생", "재생", "생명력 흡수"]):
        return "HEAL"

    if any(keyword in name for keyword in ["보호막", "보호", "갑옷", "수호", "껍질"]):
        return "DEFENSIVE"

    if any(keyword in name for keyword in ["외침", "축복", "은총", "징표", "지능", "정신", "인내"]):
        return "PRECOMBAT_BUFF"

    return "UTILITY_COMBAT"


def memo(category):
    return {
        "PRECOMBAT_BUFF": "전투 전 유지 확인",
        "STANCE_FORM_AURA": "필요 태세/형상일 때 사용",
        "ATTACK_SINGLE": "단일 대상 기본 전투 로테이션",
        "ATTACK_AOE": "다수 대상일 때 사용",
        "CC": "대상 제어/시간 벌기",
        "INTERRUPT": "적 시전 중 우선 사용",
        "DISPEL_CLEANSE": "해제 가능한 효과가 있을 때 사용",
        "DEFENSIVE": "체력 위험 또는 큰 피해 예측",
        "HEAL": "아군/자신 체력 회복",
        "RESURRECTION": "사망 아군 복구 상황",
        "RESOURCE": "자원 부족 시 사용",
        "MOBILITY": "거리 조절/접근/이탈",
        "TAUNT_THREAT": "대상 고정/위협 제어",
        "PET_SUMMON": "전투 전 또는 펫 부재 시",
        "UTILITY_COMBAT": "조건부 전투 유틸",
        "EXCLUDE_NONCOMBAT": "시련 전투 AI 사용 제외 권장",
    }.get(category, "조건부 전투 유틸")


def spell_display(spell):
    if spell["rank"]:
        return f"{spell['name']} {spell['rank']}"
    return spell["name"]


def esc_md(value):
    return str(value).replace("|", "\\|").replace("\r", " ").replace("\n", " ").strip()


def sort_spells(rows):
    return sorted(rows, key=lambda row: (row["level"], row["name"], row["spell_id"]))


def render_markdown(spells_by_category):
    md = [
        "# 3.3.5 시련 전투용 직업 스펠 카테고리",
        "",
        "이 문서는 Wowhead WotLK 한국어 직업 능력 페이지를 기준으로 시련 전투 AI/전투 로직에서 쓰기 쉽도록 전투 목적별로 재분류한 목록입니다.",
        "",
        "## 기준",
        "",
        "| 항목 | 내용 |",
        "|---|---|",
        f"| 기준 사이트 | `{WOWHEAD_BASE}/{{class}}` |",
        "| 직업 URL | `warrior`, `paladin`, `hunter`, `rogue`, `priest`, `death-knight`, `shaman`, `mage`, `warlock`, `druid` |",
        "| 포함 범위 | Wowhead WotLK `spells/abilities/{직업}` 페이지에 노출되는 직업 능력 |",
        "| 제외 범위 | 해당 Wowhead 직업 능력 페이지에 없는 특성 전용 스펠은 포함하지 않습니다. 예: 사제 `침묵(15487)` |",
        "| 랭크 처리 | 같은 직업/같은 스펠명은 최고 레벨/최고 랭크만 표시 |",
        "| 목적 | 시련 그림자/AI가 전투 상황에 따라 어떤 스펠을 사용할지 분류하기 위한 기획 자료 |",
        "",
        "## 카테고리 요약",
        "",
        "| 코드 | 카테고리 | 스펠 수 | 사용 시점 |",
        "|---|---|---:|---|",
    ]

    for key, title, desc in CATEGORIES:
        md.append(f"| `{key}` | {title} | {len(spells_by_category.get(key, []))} | {desc} |")

    md += [
        "",
        "## 시련 전투 적용 가이드",
        "",
        "| 우선순위 | 상황 | 권장 카테고리 |",
        "|---:|---|---|",
        "| 1 | 전투 시작 전 또는 재시작 직후 | `전투 전 버프/준비`, `태세/형상/오라/폼 전환`, `소환/펫/하수인` |",
        "| 2 | 적 캐스터가 주문 시전 중 | `차단/침묵/시전 방해` |",
        "| 3 | 체력이 위험함 | `생존/방어/피해 감소`, `치유/회복` |",
        "| 4 | 마나 또는 자원이 부족함 | `마나/자원 회복` |",
        "| 5 | 적을 묶거나 시간을 벌어야 함 | `전투 중 CC/메즈/이동 제한` |",
        "| 6 | 거리가 벌어짐 | `이동/돌진/도주` |",
        "| 7 | 탱커형 그림자가 대상 고정을 해야 함 | `어그로/도발/위협 제어` |",
        "| 8 | 일반 딜 사이클 | `전투 중 공격기 - 단일 대상`, `전투 중 공격기 - 광역/다중 대상` |",
        "| 9 | 사망 후 복구 로직 | `부활` |",
        "",
        "## 직업별 전투 스펠 분류",
        "",
    ]

    for class_id, class_name, class_en, slug in CLASS_DATA:
        md += [
            f"## {class_name} ({class_en})",
            "",
            f"Wowhead 기준 URL: [{slug}]({WOWHEAD_BASE}/{slug})",
            "",
            "### 카테고리 요약",
            "",
            "| 카테고리 | 스펠 수 |",
            "|---|---:|",
        ]

        for key, title, _ in CATEGORIES:
            rows = [spell for spell in spells_by_category.get(key, []) if spell["class_id"] == class_id]
            if rows:
                md.append(f"| {title} | {len(rows)} |")
        md.append("")

        for key, title, desc in CATEGORIES:
            rows = [spell for spell in sort_spells(spells_by_category.get(key, [])) if spell["class_id"] == class_id]
            if not rows:
                continue
            md += [
                f"### {title}",
                "",
                desc,
                "",
                "| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |",
                "|---|---:|---|---:|---|",
            ]
            for spell in rows:
                link = f"[{spell['spell_id']}]({spell['url']})"
                md.append(f"| {spell['source']} | {link} | {esc_md(spell_display(spell))} | {spell['level']} | {memo(key)} |")
            md.append("")

        md.append("")

    OUT_MD.write_text("\n".join(md) + "\n", encoding="utf-8-sig")


def parse_table_row(line):
    stripped = line.strip()
    if stripped.startswith("|"):
        stripped = stripped[1:]
    if stripped.endswith("|"):
        stripped = stripped[:-1]
    return [cell.strip() for cell in stripped.split("|")]


def inline_md(text):
    text = html.escape(text)
    text = re.sub(r"`([^`]+)`", r"<code>\1</code>", text)
    text = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r'<a href="\2">\1</a>', text)
    return text


def render_html():
    text = OUT_MD.read_text(encoding="utf-8-sig")
    lines = text.splitlines()
    slug_counts = {}

    def slugify(title):
        base = re.sub(r"[^0-9A-Za-z가-힣]+", "-", title).strip("-").lower() or "section"
        count = slug_counts.get(base, 0)
        slug_counts[base] = count + 1
        return base if count == 0 else f"{base}-{count + 1}"

    heading_records = []
    for line in lines:
        match = re.match(r"^(#{1,3})\s+(.+)$", line)
        if match:
            heading_records.append((len(match.group(1)), match.group(2).strip(), slugify(match.group(2).strip())))

    slug_counts = {}
    nav = "\n".join(
        f'<a class="nav-l{level}" href="#{slug}">{html.escape(title)}</a>'
        for level, title, slug in heading_records
        if level > 1
    )

    body = []
    in_table = False
    in_ul = False

    def close():
        nonlocal in_table, in_ul
        if in_table:
            body.append("</tbody></table>")
            in_table = False
        if in_ul:
            body.append("</ul>")
            in_ul = False

    for line in lines:
        if not line.strip():
            close()
            continue

        match = re.match(r"^(#{1,3})\s+(.+)$", line)
        if match:
            close()
            level = len(match.group(1))
            title = match.group(2).strip()
            slug = slugify(title)
            body.append(f'<h{level} id="{slug}">{html.escape(title)}</h{level}>')
            continue

        if line.startswith("|") and "|" in line[1:]:
            cells = parse_table_row(line)
            if all(set(cell) <= {"-", ":"} for cell in cells):
                continue
            if not in_table:
                body.append("<table><tbody>")
                in_table = True
            tag = "th" if all(cell in ["항목", "내용", "코드", "카테고리", "스펠 수", "사용 시점", "우선순위", "상황", "권장 카테고리", "출처", "주문 ID", "스펠", "요구 레벨", "사용 메모"] for cell in cells) else "td"
            body.append("<tr>" + "".join(f"<{tag}>{inline_md(cell)}</{tag}>" for cell in cells) + "</tr>")
            continue

        close()
        body.append(f"<p>{inline_md(line)}</p>")

    close()

    html_doc = f"""<!doctype html>
<html lang="ko">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>3.3.5 시련 전투용 직업 스펠 카테고리</title>
<style>
:root {{
  --bg: #f5efe4;
  --panel: #fffaf0;
  --ink: #21190f;
  --muted: #6f604b;
  --line: #d8c7aa;
  --accent: #8a4d21;
  --accent-2: #1f5d58;
}}
body {{
  margin: 0;
  background: radial-gradient(circle at top left, #fdf4db 0, #f5efe4 32rem, #eadcc5 100%);
  color: var(--ink);
  font-family: "Malgun Gothic", "Noto Sans KR", sans-serif;
  line-height: 1.62;
}}
.layout {{
  display: grid;
  grid-template-columns: 18rem minmax(0, 1fr);
  gap: 2rem;
  max-width: 1480px;
  margin: 0 auto;
  padding: 2rem;
}}
nav {{
  position: sticky;
  top: 1rem;
  align-self: start;
  max-height: calc(100vh - 2rem);
  overflow: auto;
  padding: 1rem;
  background: rgba(255, 250, 240, .9);
  border: 1px solid var(--line);
  border-radius: 18px;
  box-shadow: 0 18px 48px rgba(69, 48, 20, .12);
}}
nav a {{
  display: block;
  color: var(--accent-2);
  text-decoration: none;
  padding: .18rem 0;
  font-size: .9rem;
}}
.nav-l3 {{ padding-left: 1rem; color: var(--muted); }}
main {{
  min-width: 0;
  padding: 2rem;
  background: rgba(255, 250, 240, .94);
  border: 1px solid var(--line);
  border-radius: 24px;
  box-shadow: 0 24px 70px rgba(69, 48, 20, .16);
}}
h1, h2, h3 {{
  line-height: 1.25;
}}
h1 {{
  font-size: clamp(2rem, 4vw, 3.4rem);
  margin-top: 0;
  color: #3b2614;
}}
h2 {{
  margin-top: 3rem;
  padding-top: 1rem;
  border-top: 2px solid var(--line);
  color: var(--accent);
}}
h3 {{ color: #594026; }}
table {{
  width: 100%;
  border-collapse: collapse;
  margin: 1rem 0 2rem;
  font-size: .94rem;
  background: #fffdf7;
}}
th, td {{
  border: 1px solid var(--line);
  padding: .55rem .7rem;
  vertical-align: top;
}}
th {{
  background: #efe1c8;
  color: #332313;
}}
tr:nth-child(even) td {{ background: #fbf4e6; }}
code {{
  background: #eadcc5;
  padding: .1rem .32rem;
  border-radius: .35rem;
}}
a {{ color: var(--accent-2); }}
@media (max-width: 980px) {{
  .layout {{ display: block; padding: 1rem; }}
  nav {{ position: relative; max-height: none; margin-bottom: 1rem; }}
  main {{ padding: 1rem; }}
}}
</style>
</head>
<body>
<div class="layout">
<nav>
<strong>목차</strong>
{nav}
</nav>
<main>
{chr(10).join(body)}
</main>
</div>
</body>
</html>
"""
    OUT_HTML.write_text(html_doc, encoding="utf-8")


def main():
    spells = fetch_wowhead_spells()
    by_category = defaultdict(list)
    for spell in spells:
        by_category[classify(spell)].append(spell)

    render_markdown(by_category)
    render_html()
    print(OUT_MD)
    print(OUT_HTML)
    print(f"total={len(spells)}")
    for key, _, _ in CATEGORIES:
        print(f"{key}={len(by_category.get(key, []))}")


if __name__ == "__main__":
    main()
