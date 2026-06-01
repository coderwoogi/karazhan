import subprocess
import sys
from pathlib import Path


MPQCLI = Path(r"E:\server\tools\mpqcli\mpqcli.exe")
PATCH_Z = Path(r"E:\server\3.3.5\Data\patch-Z.MPQ")
WORK_DIR = Path(r"E:\server\tmp_patchz_remove_base_asset_overlaps")
REPORT = Path(r"E:\server\azerothcore-wotlk\doc\patchz_base_asset_overlap_removed_ko.md")

BASE_MPQS = [
    Path(r"E:\server\3.3.5\Data\common.MPQ"),
    Path(r"E:\server\3.3.5\Data\common-2.MPQ"),
    Path(r"E:\server\3.3.5\Data\expansion.MPQ"),
    Path(r"E:\server\3.3.5\Data\lichking.MPQ"),
    Path(r"E:\server\3.3.5\Data\patch.MPQ"),
    Path(r"E:\server\3.3.5\Data\patch-2.MPQ"),
    Path(r"E:\server\3.3.5\Data\patch-3.MPQ"),
    Path(r"E:\server\3.3.5\Data\patch-6.MPQ"),
    Path(r"E:\server\3.3.5\Data\patch-A.mpq"),
    Path(r"E:\server\3.3.5\Data\koKR\locale-koKR.MPQ"),
    Path(r"E:\server\3.3.5\Data\koKR\patch-koKR.MPQ"),
    Path(r"E:\server\3.3.5\Data\koKR\patch-koKR-2.MPQ"),
    Path(r"E:\server\3.3.5\Data\koKR\patch-koKR-3.MPQ"),
    Path(r"E:\server\3.3.5\Data\koKR\patch-koKR-4.MPQ"),
]

ASSET_EXTS = (".m2", ".skin", ".blp", ".anim", ".bone")


def run(args):
    result = subprocess.run(
        args,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def norm(path):
    return path.replace("/", "\\").lower()


def mpq_list(mpq):
    return [line.strip() for line in run([str(MPQCLI), "list", str(mpq)]).splitlines() if line.strip()]


def creature_asset(path):
    value = norm(path)
    return value.startswith("creature\\") and value.endswith(ASSET_EXTS)


def main():
    WORK_DIR.mkdir(parents=True, exist_ok=True)
    temp_mpq = WORK_DIR / "patch-Z.MPQ"
    if not temp_mpq.exists():
        temp_mpq.write_bytes(PATCH_Z.read_bytes())

    base_paths = set()
    for mpq in BASE_MPQS:
        if not mpq.exists():
            continue
        for path in mpq_list(mpq):
            if creature_asset(path):
                base_paths.add(norm(path))

    patchz_assets = [path for path in mpq_list(temp_mpq) if creature_asset(path)]
    overlaps = [path for path in patchz_assets if norm(path) in base_paths]
    overlap_file = WORK_DIR / "overlap_paths.txt"
    overlap_file.write_text("\n".join(overlaps) + "\n", encoding="utf-8")

    removed = 0
    failed = []
    for index, path in enumerate(overlaps, 1):
        try:
            run([str(MPQCLI), "remove", path, str(temp_mpq)])
            removed += 1
        except Exception as exc:
            failed.append((path, str(exc)))
        if index % 100 == 0:
            print(f"progress={index}/{len(overlaps)} removed={removed}", flush=True)

    PATCH_Z.write_bytes(temp_mpq.read_bytes())

    lines = [
        "# patch-Z 원본 Creature 에셋 충돌 제거",
        "",
        f"- patch-Z Creature 에셋 수: {len(patchz_assets)}",
        f"- 원본 클라이언트와 경로가 겹친 에셋 수: {len(overlaps)}",
        f"- 제거 성공: {removed}",
        f"- 제거 실패: {len(failed)}",
        f"- 제거 목록: `{overlap_file}`",
        "",
        "## 목적",
        "",
        "원본과 같은 경로의 Creature 모델, 텍스처, 애니메이션 파일이 patch-Z에 있으면 일반 NPC도 커스텀 파일로 덮어써진다.",
        "기존 NPC 외형 복구를 위해 원본과 경로가 겹치는 Creature 에셋을 patch-Z에서 제거했다.",
    ]
    if failed:
        lines.extend(["", "## 실패 목록"])
        lines.extend(f"- `{path}`: {reason}" for path, reason in failed[:50])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"patchz_assets={len(patchz_assets)}")
    print(f"overlaps={len(overlaps)}")
    print(f"removed={removed}")
    print(f"failed={len(failed)}")
    print(REPORT)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
