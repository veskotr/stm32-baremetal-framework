from __future__ import annotations

import shutil
import tempfile
import unittest
from pathlib import Path

from board_sync.sync import sync_board


class BoardSyncIdempotenceTests(unittest.TestCase):
    REPOSITORY_ROOT = Path(__file__).resolve().parents[3]
    OUTPUTS = (
        Path("board_manifest.cmake"),
        Path("generated/board_config.h"),
        Path("generated/board_glue.c"),
    )

    def assert_sync_is_idempotent(self, board_name: str) -> None:
        source = self.REPOSITORY_ROOT / "examples/boards" / board_name

        with tempfile.TemporaryDirectory() as temporary_directory:
            board = Path(temporary_directory) / board_name
            shutil.copytree(source, board)

            sync_board(board)
            first_outputs = {
                output: (board / output).read_bytes()
                for output in self.OUTPUTS
            }

            sync_board(board)

            for output, first_content in first_outputs.items():
                self.assertEqual(
                    first_content,
                    (board / output).read_bytes(),
                    f"{board_name} changed {output} on its second sync",
                )

    def test_blue_pill_reference_board_sync_is_idempotent(self) -> None:
        self.assert_sync_is_idempotent("blue_pill_temp_transmitter")

    def test_stm32g0_reference_board_sync_is_idempotent(self) -> None:
        self.assert_sync_is_idempotent("stm32g071kb_temp_transmitter")
