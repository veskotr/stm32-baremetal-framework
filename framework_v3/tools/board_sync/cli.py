from __future__ import annotations

import argparse
from pathlib import Path

from .sync import sync_board


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate framework metadata from a CubeMX board directory.")
    parser.add_argument("board_dir", type=Path, help="CubeMX-generated board directory")
    args = parser.parse_args()
    sync_board(args.board_dir)


if __name__ == "__main__":
    main()
