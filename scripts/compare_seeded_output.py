"""Compare every public dungeon field and the complete RNG state with a Git baseline."""

import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import sys


def runCommand(command, *, directory, log_path):
    result = subprocess.run(command, cwd=directory, capture_output=True, timeout=180)
    log_path.write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {command}; see {log_path}")
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-ref", required=True)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--frontend", choices=("gnu", "msvc"), default="gnu")
    parser.add_argument("--output-dir", type=Path, default=Path("parity-artifacts"))
    arguments = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = (root / arguments.output_dir).resolve()
    output.mkdir(parents=True, exist_ok=True)
    manifest = {
        "baseline_ref": arguments.baseline_ref,
        "compiler": arguments.compiler,
        "frontend": arguments.frontend,
        "platform": platform.platform(),
        "matches": False,
        "snapshot_driver_sha256": hashlib.sha256((root / "tests" / "seeded_snapshot.cpp").read_bytes()).hexdigest(),
    }
    try:
        for name, reference in (("baseline", arguments.baseline_ref), ("candidate", "HEAD")):
            manifest[f"{name}_sha"] = (
                runCommand(
                    ["git", "rev-parse", "--verify", f"{reference}^{{commit}}"],
                    directory=root,
                    log_path=output / f"{name}-revision.log",
                )
                .decode()
                .strip()
            )
        source = root / "tests" / "seeded_snapshot.cpp"
        snapshots = {}
        for name in ("baseline", "candidate"):
            directory = output / name
            directory.mkdir(exist_ok=True)
            if name == "baseline":
                for header in ("rdg.h", "vstd.h"):
                    content = runCommand(
                        ["git", "show", f"{manifest['baseline_sha']}:{header}"],
                        directory=root,
                        log_path=directory / f"{header}.log",
                    )
                    (directory / header).write_bytes(content)
                include = directory
            else:
                include = root
            manifest[f"{name}_header_sha256"] = {
                header: hashlib.sha256((include / header).read_bytes()).hexdigest() for header in ("rdg.h", "vstd.h")
            }
            executable = directory / ("snapshot.exe" if sys.platform == "win32" else "snapshot")
            if arguments.frontend == "msvc":
                command = [
                    arguments.compiler,
                    "/std:c++latest",
                    "/EHsc",
                    "/O2",
                    f"/I{include}",
                    str(source),
                    f"/Fe:{executable}",
                    f"/Fo:{directory / 'snapshot.obj'}",
                ]
            else:
                command = [
                    arguments.compiler,
                    "-std=c++26",
                    "-O2",
                    "-I",
                    str(include),
                    str(source),
                    "-o",
                    str(executable),
                ]
            manifest[f"{name}_command"] = command
            runCommand(command, directory=directory, log_path=directory / "compile.log")
            snapshots[name] = runCommand([str(executable)], directory=directory, log_path=output / f"{name}.txt")
            manifest[f"{name}_snapshot_sha256"] = hashlib.sha256(snapshots[name]).hexdigest()
            manifest[f"{name}_snapshot_bytes"] = len(snapshots[name])
        manifest["matches"] = snapshots["baseline"] == snapshots["candidate"]
        if not manifest["matches"]:
            raise RuntimeError(
                f"Seeded dungeon or RNG output changed; compare {output / 'baseline.txt'} and candidate.txt"
            )
        print(f"Complete seeded dungeon and RNG parity passed: {len(snapshots['candidate'])} bytes; evidence: {output}")
    except Exception as error:
        manifest["error"] = str(error)
        raise
    finally:
        (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
