"""
実行には python3 環境が必要です。
TOKEN 変数を書き換えて実行してください。
"""
import os
import random
import subprocess
import sys
import time
import json
from typing import Iterable, List, Set, Tuple
from urllib.request import urlopen

# ゲームサーバのアドレス / トークン
GAME_SERVER = os.getenv('GAME_SERVER', 'https://contest.2021-autumn.gbc.tenka1.klab.jp')
TOKEN = os.getenv('TOKEN', '30fafe10accbd85d9a61ab9df091c5d4')


p = subprocess.Popen(sys.argv[1:], stdin=subprocess.PIPE, stdout=subprocess.PIPE)


def call_api(x: str) -> dict:
    url = f'{GAME_SERVER}{x}'
    with urlopen(url) as res:
        return json.loads(res.read())


def call_game():
    r = call_api(f'/api/game/{TOKEN}')
    assert r['status'] == 'ok'
    p.stdin.write(f'{r["now"]} {len(r["agent"])} {len(r["resource"])} {r["next_resource"]} {len(r["owned_resource"])}\n'.encode())
    for a in r["agent"]:
        p.stdin.write(f'{len(a["move"])}\n'.encode())
        for m in a["move"]:
            p.stdin.write(f'{m["x"]} {m["y"]} {m["t"]}\n'.encode())
    for s in r["resource"]:
        p.stdin.write(f'{s["id"]} {s["x"]} {s["y"]} {s["t0"]} {s["t1"]} {s["type"]} {s["weight"]}\n'.encode())
    for o in r["owned_resource"]:
        p.stdin.write(f'{o["type"]} {o["amount"]}\n'.encode())
    p.stdin.flush()


def read_move(r):
    assert r['status'] == 'ok'
    p.stdin.write(f'{r["now"]} {len(r["move"])}\n'.encode())
    for m in r["move"]:
        p.stdin.write(f'{m["x"]} {m["y"]} {m["t"]}\n'.encode())
    p.stdin.flush()


def call_move(index: str, x: str, y: str):
    read_move(call_api(f'/api/move/{TOKEN}/{index}-{x}-{y}'))


def call_will_move(index: str, x: str, y: str, t: str):
    read_move(call_api(f'/api/will_move/{TOKEN}/{index}-{x}-{y}-{t}'))


def call_resources(ids: Iterable[str]):
    r = call_api(f'/api/resources/{TOKEN}/{"-".join(ids)}')
    assert r['status'] == 'ok'
    p.stdin.write(f'{len(r["resource"])}\n'.encode())
    for s in r["resource"]:
        p.stdin.write(f'{s["id"]} {s["x"]} {s["y"]} {s["t0"]} {s["t1"]} {s["type"]} {s["weight"]} {s["amount"]}\n'.encode())
    p.stdin.flush()


def main():
    while True:
        line = p.stdout.readline()
        if not line:
            break
        a = line.decode().rstrip().split(' ')
        if a[0] == 'game':
            call_game()
        elif a[0] == 'move':
            call_move(a[1], a[2], a[3])
        elif a[0] == 'will_move':
            call_will_move(a[1], a[2], a[3], a[4])
        elif a[0] == 'resources':
            call_resources(a[1:])
        else:
            assert False, f'invalid command {repr(a[0])}'


if __name__ == "__main__":
    main()
