#!/usr/bin/env python3
"""Copy selected public gsync modules and one build configuration into MqasNet."""
import argparse
import os
import shutil
from pathlib import Path

MODULE_LIBRARIES = {"example1": "gsync_e1"}
LEGACY_HEADERS = (
    "include/def.h", "include/macro.h", "include/core/engine.h",
    "include/core/connection.h", "include/core/server.h", "include/core/session_handle.h",
    "include/core/p2p_context.h", "example1/lobby.h", "example1/p2p_peer.h",
    "example1/p2p_connection_datagram.h", "example1/p2p_peer_internal.h",
    "include/gsync/example1/p2p_peer.h", "example1/p2p_session.h", "example1/p2p_session_registry.h", "example1/p2p_datagram.pb.h",
)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", nargs="?", default=os.environ.get("GSYNC_ROOT"))
    parser.add_argument("--platform", default="windows")
    parser.add_argument("--arch", default="x64")
    parser.add_argument("--config", default="release")
    parser.add_argument("--modules", nargs="+", choices=MODULE_LIBRARIES, default=["example1"])
    args = parser.parse_args()
    if not args.root:
        parser.error("provide the gsync root or set GSYNC_ROOT")
    root = Path(args.root).resolve()
    dest = Path(__file__).resolve().parent
    modules = ["core", *dict.fromkeys(args.modules)]
    for module in modules:
        source = root / "include" / "gsync" / module
        if not source.is_dir():
            raise FileNotFoundError(source)
        target = dest / "include" / "gsync" / module
        target.mkdir(parents=True, exist_ok=True)
        for header in source.rglob("*.h"):
            destination = target / header.relative_to(source)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(header, destination)
    build = root / "output" / args.platform / args.arch / args.config
    target_build = dest / "output" / args.platform / args.arch / args.config
    lib_suffix = ".lib" if args.platform == "windows" else ".a"
    for module in args.modules:
        name = MODULE_LIBRARIES[module]
        matches = list((build / "lib").glob(f"*{name}{lib_suffix}"))
        if not matches:
            raise FileNotFoundError(f"import library for {name} not found in {build / 'lib'}")
        (target_build / "lib").mkdir(parents=True, exist_ok=True)
        for library in matches:
            shutil.copy2(library, target_build / "lib" / library.name)
    binary_dir = build / "bin"
    manifest = binary_dir / "dependencies.ini"
    if not manifest.is_file():
        raise FileNotFoundError(manifest)
    names = {Path(item.replace("\\", "/")).name for item in manifest.read_text().split(";") if item.strip()}
    dependency_pattern = "mqas.dll" if args.platform == "windows" else "libmqas.so*" if args.platform == "linux" else "libmqas.dylib"
    mqas_libraries = list(binary_dir.glob(dependency_pattern))
    if not mqas_libraries:
        raise FileNotFoundError(f"mqas runtime library not found in {binary_dir}")
    names.update(p.name for p in mqas_libraries)
    for module in args.modules:
        stem = MODULE_LIBRARIES[module]
        candidates = list(binary_dir.glob(f"*{stem}.dll" if args.platform == "windows" else f"*{stem}.so*" if args.platform == "linux" else f"*{stem}.dylib"))
        if not candidates:
            raise FileNotFoundError(f"runtime library for {stem} not found in {binary_dir}")
        names.update(p.name for p in candidates)
    (target_build / "bin").mkdir(parents=True, exist_ok=True)
    for name in sorted(names):
        source = binary_dir / name
        if not source.is_file():
            raise FileNotFoundError(source)
        shutil.copy2(source, target_build / "bin" / name)
    (target_build / "bin" / "gsync-runtime.txt").write_text("\n".join(sorted(names)) + "\n", encoding="utf-8")
    for old in LEGACY_HEADERS:
        path = dest / old
        if path.is_file():
            path.unlink()
    print(f"Copied {', '.join(modules)} for {args.platform}/{args.arch}/{args.config}")

if __name__ == "__main__":
    main()
