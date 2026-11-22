"""Run two headless g01 clients against an isolated local P2P helper server.

Example:
  python run_p2p_travel_smoke.py --editor D:/UE/Engine/Binaries/Win64/UnrealEditor-Cmd.exe \
      --server-dir D:/code/g01/Saved/LocalP2PBundle/server
The server directory must contain mqas_p2p_sample_server.exe, its DLLs,
conf.txt, cert-def.pem and key-def.pem. The project lobby config must trust the
server certificate. Logs are written to Saved/Logs/P2PTravelSmoke-{host,join}.log.
"""
import argparse
import re
import socket
import subprocess
import tempfile
import time
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--editor', type=Path, required=True)
    parser.add_argument('--server-dir', type=Path, required=True)
    parser.add_argument('--timeout', type=float, default=150)
    args = parser.parse_args()
    project = Path(__file__).resolve().parents[5]
    server = args.server_dir.resolve()
    editor = args.editor.resolve()
    for path in (editor, server / 'mqas_p2p_sample_server.exe', server / 'conf.txt',
                 server / 'cert-def.pem', server / 'key-def.pem', project / 'g01.uproject'):
        if not path.is_file():
            raise FileNotFoundError(path)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('127.0.0.1', 0))
    port = sock.getsockname()[1]
    sock.close()
    processes = []
    temp = tempfile.TemporaryDirectory(prefix='g01-p2p-travel-')
    try:
        work = Path(temp.name)
        config = (server / 'conf.txt').read_text(encoding='utf-8')
        config, count = re.subn(r'(?m)^port[ \t]*=[ \t]*8084[ \t]*$', f'port={port}', config, count=1)
        if count != 1:
            raise RuntimeError('Expected server bind port 8084 in conf.txt')
        config = config.replace('"cert-def.pem"', f'"{(server / "cert-def.pem").as_posix()}"')
        config = config.replace('"key-def.pem"', f'"{(server / "key-def.pem").as_posix()}"')
        (work / 'conf.txt').write_text(config, encoding='utf-8')
        hidden = getattr(subprocess, 'CREATE_NO_WINDOW', 0)
        server_process = subprocess.Popen([str(server / 'mqas_p2p_sample_server.exe')], cwd=work,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=hidden)
        processes.append(('server', None, server_process))
        time.sleep(2)
        if server_process.poll() is not None:
            raise RuntimeError(f'P2P server exited: {server_process.returncode}')
        for role in ('host', 'join'):
            log = project / 'Saved' / 'Logs' / f'P2PTravelSmoke-{role}.log'
            log.parent.mkdir(parents=True, exist_ok=True)
            log.unlink(missing_ok=True)
            command = [str(editor), str(project / 'g01.uproject'), '-game', '-nullrhi',
                '-unattended', '-nop4', '-nosplash', '-NoSound', '-Multiprocess',
                f'-ExecCmds=P2P.TravelSmoke {role} {port}', f'-abslog={log}']
            process = subprocess.Popen(command, stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL, creationflags=hidden)
            processes.append((role, log, process))
            time.sleep(2)
        deadline = time.monotonic() + args.timeout
        results = {}
        while time.monotonic() < deadline:
            for role, log, process in processes[1:]:
                content = log.read_text(encoding='utf-8', errors='replace') if log.exists() else ''
                results[role] = ('P2P_TRAVEL_SMOKE_PASS' in content,
                                 'P2P_TRAVEL_SMOKE_FAIL' in content, process.poll())
            if all(r[0] for r in results.values()) or any(r[1] or r[2] is not None for r in results.values()):
                break
            time.sleep(2)
        print('P2P travel:', results, flush=True)
        if not results or not all(r[0] for r in results.values()):
            for role, log, _ in processes[1:]:
                if log.exists():
                    print(f'{role}: {log}', flush=True)
            raise SystemExit(1)
    finally:
        for _, _, process in processes:
            if process.poll() is None:
                process.terminate()
        for _, _, process in processes:
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        temp.cleanup()


if __name__ == '__main__':
    main()
