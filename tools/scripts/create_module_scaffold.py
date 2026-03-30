#!/usr/bin/env python3
import argparse
import json
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from string import Template
from typing import Dict, List, Optional


MODULE_TYPES = {"app", "device", "package", "utility", "bsp"}


@dataclass
class ModuleSpec:
    module_type: str
    name: str
    board: Optional[str] = None
    brief: Optional[str] = None


def default_config() -> Dict:
    return {
        "version": 1,
        "project_name": "embedded_project",
        "author": "lxf",
        "last_editors": "lxf_zjnb@qq.com",
        "default_board": "custom-board",
        "default_ide": "mdk",
        "default_toolchain": "armclang",
        "paths": {
            "app": "app",
            "device": "components/fp-sdk/drivers/device",
            "package": "components/fp-sdk/packages",
            "utility": "components/fp-sdk/utilities",
            "board_root": "board",
            "bsp": "board/{board}/bsp",
            "docs": "docs",
        },
        "profiles": {},
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate embedded module scaffolds for the current repository."
    )
    parser.add_argument(
        "--config",
        help="Path to module_scaffold.json. Defaults to repository config if present.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print planned files without writing them.",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing files.",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)
    new_parser = subparsers.add_parser("new", help="Create scaffold content.")
    new_subparsers = new_parser.add_subparsers(dest="kind", required=True)

    module_parser = new_subparsers.add_parser("module", help="Create one module scaffold.")
    module_parser.add_argument("--type", required=True, choices=sorted(MODULE_TYPES))
    module_parser.add_argument("--name", required=True)
    module_parser.add_argument("--board")
    module_parser.add_argument("--brief")
    module_parser.add_argument("--dry-run", action="store_true")
    module_parser.add_argument("--force", action="store_true")

    board_parser = new_subparsers.add_parser("board", help="Create one board directory scaffold.")
    board_parser.add_argument("--board", required=True)
    board_parser.add_argument("--dry-run", action="store_true")

    profile_parser = new_subparsers.add_parser("profile", help="Create one profile scaffold.")
    profile_parser.add_argument("--name", required=True)
    profile_parser.add_argument("--board")
    profile_parser.add_argument(
        "--without",
        nargs="*",
        default=[],
        help="Skip optional module names in profile generation.",
    )
    profile_parser.add_argument("--dry-run", action="store_true")
    profile_parser.add_argument("--force", action="store_true")

    return parser.parse_args()


def config_candidates(repo_root: Path) -> List[Path]:
    return [
        repo_root / "tools" / "module_scaffold.json",
        repo_root / "module_scaffold.json",
    ]


def load_config(repo_root: Path, config_arg: Optional[str]) -> Dict:
    config = default_config()

    if config_arg:
        config_path = Path(config_arg)
        if not config_path.is_absolute():
            config_path = repo_root / config_path

        if not config_path.exists():
            raise FileNotFoundError(f"config file not found: {config_path}")

        with config_path.open("r", encoding="utf-8-sig") as fp:
            loaded = json.load(fp)
        config.update(loaded)
        if "paths" in loaded:
            config["paths"].update(loaded["paths"])
        if "profiles" in loaded:
            config["profiles"].update(loaded["profiles"])
        return config

    for candidate in config_candidates(repo_root):
        if candidate.exists():
            with candidate.open("r", encoding="utf-8-sig") as fp:
                loaded = json.load(fp)
            config.update(loaded)
            if "paths" in loaded:
                config["paths"].update(loaded["paths"])
            if "profiles" in loaded:
                config["profiles"].update(loaded["profiles"])
            return config

    return config


def ensure_type(module_type: str) -> None:
    if module_type not in MODULE_TYPES:
        raise ValueError(f"unsupported module type: {module_type}")


def module_root(repo_root: Path, config: Dict, module_type: str, board: Optional[str]) -> Path:
    paths = config["paths"]

    if module_type == "app":
        return repo_root / paths["app"]
    if module_type == "device":
        return repo_root / paths["device"]
    if module_type == "package":
        return repo_root / paths["package"]
    if module_type == "utility":
        return repo_root / paths["utility"]
    if module_type == "bsp":
        resolved_board = board or config.get("default_board")
        if not resolved_board:
            raise ValueError("board is required for bsp module")
        return repo_root / Path(paths["bsp"].format(board=resolved_board))

    raise ValueError(f"unsupported module type: {module_type}")


def module_paths(repo_root: Path, config: Dict, spec: ModuleSpec) -> Dict[str, Path]:
    ensure_type(spec.module_type)
    root = module_root(repo_root, config, spec.module_type, spec.board)

    if spec.module_type == "app":
        stem = f"app_{spec.name}"
        return {"header": root / f"{stem}.h", "source": root / f"{stem}.c"}

    if spec.module_type == "device":
        return {"header": root / f"{spec.name}.h", "source": root / f"{spec.name}.c"}

    if spec.module_type == "package":
        package_dir = root / spec.name
        return {
            "header": package_dir / f"{spec.name}.h",
            "source": package_dir / f"{spec.name}.c",
        }

    if spec.module_type == "utility":
        return {"header": root / f"{spec.name}.h", "source": root / f"{spec.name}.c"}

    if spec.module_type == "bsp":
        stem = f"bsp_{spec.name}"
        return {"header": root / f"{stem}.h", "source": root / f"{stem}.c"}

    raise ValueError(f"unsupported module type: {spec.module_type}")


def board_paths(repo_root: Path, config: Dict, board: str) -> List[Path]:
    board_root = repo_root / config["paths"]["board_root"] / board
    return [
        board_root,
        board_root / "bsp",
        board_root / "config",
        board_root / "src",
        board_root / "cpu",
        board_root / ".mdk",
    ]


def file_header(file_name: str, brief: str, author: str, last_editors: str, now: str) -> str:
    return (
        "/*\n"
        " * Copyright (c) 2025 by Lu Xianfan.\n"
        f" * @FilePath     : {file_name}\n"
        f" * @Author       : {author}\n"
        f" * @Date         : {now}\n"
        f" * @LastEditors  : {last_editors}\n"
        f" * @LastEditTime : {now}\n"
        f" * @Brief        : {brief}\n"
        " */"
    )


def include_guard_name(path: Path) -> str:
    return "__" + path.stem.upper() + "_H__"


def default_brief(module_type: str, name: str) -> str:
    brief_map = {
        "app": f"{name} 应用模块",
        "device": f"{name} 设备驱动",
        "package": f"{name} 领域模块",
        "utility": f"{name} 工具模块",
        "bsp": f"{name} 板级适配模块",
    }
    return brief_map[module_type]


def public_prefix(module_type: str, name: str) -> str:
    if module_type == "app":
        return f"app_{name}"
    if module_type == "bsp":
        return f"bsp_{name}"
    return name


def template_candidates(repo_root: Path) -> List[Path]:
    script_root = Path(__file__).resolve().parents[1]
    return [
        script_root / "assets" / "templates" / "module_scaffold",
        repo_root / "tools" / "templates" / "module_scaffold",
    ]


def template_path(repo_root: Path, module_type: str, ext: str) -> Path:
    for root in template_candidates(repo_root):
        path = root / f"{module_type}.{ext}.tpl"
        if path.exists():
            return path

    raise FileNotFoundError(f"template file not found for {module_type}.{ext}")


def render_template(path: Path, context: Dict[str, str]) -> str:
    content = path.read_text(encoding="utf-8")
    return Template(content).substitute(context)


def render_context(
    config: Dict,
    spec: ModuleSpec,
    target_path: Path,
    paired_header: Optional[Path] = None,
) -> Dict[str, str]:
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    brief = spec.brief or default_brief(spec.module_type, spec.name)
    public_name = public_prefix(spec.module_type, spec.name)
    resolved_board = spec.board or config.get("default_board", "")

    return {
        "file_header": file_header(target_path.name, brief, config["author"], config["last_editors"], now),
        "brief": brief,
        "module_name": spec.name,
        "module_upper": spec.name.upper(),
        "module_type": spec.module_type,
        "public_name": public_name,
        "public_upper": public_name.upper(),
        "board": resolved_board,
        "include_guard": include_guard_name(target_path),
        "header_file": paired_header.name if paired_header else "",
    }


def print_plan(file_map: Dict[str, Path], dry_run: bool) -> None:
    action = "PLAN" if dry_run else "CREATE"
    for role, path in file_map.items():
        print(f"[{action}] {role}: {path}")


def write_text(path: Path, content: str, force: bool) -> None:
    if path.exists() and not force:
        raise FileExistsError(f"file already exists: {path}")

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def create_module(repo_root: Path, config: Dict, spec: ModuleSpec, dry_run: bool, force: bool) -> None:
    file_map = module_paths(repo_root, config, spec)
    print_plan(file_map, dry_run)

    if dry_run:
        return

    header_context = render_context(config, spec, file_map["header"])
    source_context = render_context(config, spec, file_map["source"], paired_header=file_map["header"])

    header_text = render_template(template_path(repo_root, spec.module_type, "h"), header_context)
    source_text = render_template(template_path(repo_root, spec.module_type, "c"), source_context)

    write_text(file_map["header"], header_text, force)
    write_text(file_map["source"], source_text, force)


def create_board(repo_root: Path, config: Dict, board: str, dry_run: bool) -> None:
    directories = board_paths(repo_root, config, board)
    action = "PLAN" if dry_run else "CREATE"
    for directory in directories:
        print(f"[{action}] dir: {directory}")
        if not dry_run:
            directory.mkdir(parents=True, exist_ok=True)


def profile_specs(config: Dict, profile_name: str, board: Optional[str], without: List[str]) -> List[ModuleSpec]:
    profiles = config.get("profiles", {})
    if profile_name not in profiles:
        raise KeyError(f"profile not found: {profile_name}")

    profile = profiles[profile_name]
    specs: List[ModuleSpec] = []
    skipped = set(without)

    for item in profile.get("modules", []):
        module_name = item["name"]
        if item.get("optional", False) and module_name in skipped:
            continue

        specs.append(
            ModuleSpec(
                module_type=item["type"],
                name=module_name,
                board=board if item["type"] == "bsp" else None,
            )
        )

    return specs


def main() -> int:
    args = parse_args()
    repo_root = Path.cwd()
    config = load_config(repo_root, args.config)

    if args.command != "new":
        raise ValueError(f"unsupported command: {args.command}")

    if args.kind == "module":
        create_module(
            repo_root,
            config,
            ModuleSpec(
                module_type=args.type,
                name=args.name,
                board=args.board,
                brief=args.brief,
            ),
            args.dry_run,
            args.force,
        )
        return 0

    if args.kind == "board":
        create_board(repo_root, config, args.board, args.dry_run)
        return 0

    if args.kind == "profile":
        specs = profile_specs(config, args.name, args.board, args.without)
        for spec in specs:
            create_module(repo_root, config, spec, args.dry_run, args.force)
        return 0

    raise ValueError(f"unsupported kind: {args.kind}")


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[FAIL] {exc}")
        raise SystemExit(1)
