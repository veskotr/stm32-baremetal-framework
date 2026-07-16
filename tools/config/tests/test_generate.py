from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.config.generate import (
    ConfigError,
    ConfigWarnings,
    generated_cmake,
    generated_header,
    generated_meta,
    load_config,
)


class ConfigGenerateTests(unittest.TestCase):
    def write(self, root: Path, name: str, content: str) -> Path:
        path = root / name
        path.write_text(content, encoding="utf-8")
        return path

    def test_custom_keys_are_prefixed_in_generated_header(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                'APP_DEVICE_NAME="demo"\nSAMPLE_PERIOD_MS=250\nTOOL_UPLOAD_PORT="/dev/ttyUSB0"\n',
            )

            values, _ = load_config(config, [])
            header = generated_header(values, [])

            self.assertIn('#define CONFIG_APP_DEVICE_NAME "demo"', header)
            self.assertNotIn("SAMPLE_PERIOD_MS", header)
            self.assertNotIn("TOOL_UPLOAD_PORT", header)
            self.assertIn("#define HSS_ENABLE_FREEMODBUS 0", header)
            self.assertNotIn("#define APP_DEVICE_NAME", header)

    def test_comments_are_ignored(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                '# comment\nAPP_DEVICE_NAME="demo" # trailing comment\n',
            )

            values, _ = load_config(config, [])

            self.assertEqual(values["APP_DEVICE_NAME"].value, "demo")

    def test_unparsable_lines_are_warnings_when_collector_is_provided(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                'not a config line\nbad-key=1\nAPP_DEVICE_NAME=\nAPP_OK="yes"\n',
            )
            warnings = ConfigWarnings()

            values, _ = load_config(config, [], warnings)

            self.assertEqual(values["APP_OK"].value, "yes")
            self.assertEqual(len(warnings.messages), 3)
            self.assertIn("expected KEY=VALUE", warnings.messages[0])
            self.assertIn("invalid config key", warnings.messages[1])
            self.assertIn("missing value", warnings.messages[2])

    def test_optional_missing_profile_warns_and_continues(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "APP_OK=y\n")
            warnings = ConfigWarnings()

            values, loaded_files = load_config(config, ["local"], warnings, optional_profiles=["local"])

            self.assertTrue(values["APP_OK"].value)
            self.assertEqual([path.name for path in loaded_files], ["hss.conf"])
            self.assertEqual(len(warnings.messages), 1)
            self.assertIn("optional profile 'local' not found", warnings.messages[0])

    def test_explicit_profile_file_overrides_named_profiles(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "APP_SAMPLE_MS=1000\n")
            self.write(root, "hss-dev.conf", "APP_SAMPLE_MS=500\n")
            overlay = self.write(root, "hardware_rev_b.conf", "APP_SAMPLE_MS=250\n")

            values, loaded_files = load_config(config, ["dev"], profile_files=[overlay])

            self.assertEqual(values["APP_SAMPLE_MS"].value, 250)
            self.assertEqual([path.name for path in loaded_files], [
                "hss.conf",
                "hss-dev.conf",
                "hardware_rev_b.conf",
            ])

    def test_profile_values_override_base_values_in_order(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "SAMPLE_PERIOD_MS=1000\n")
            self.write(root, "hss-dev.conf", "SAMPLE_PERIOD_MS=500\n")
            self.write(root, "hss-hw_rev_b.conf", "SAMPLE_PERIOD_MS=250\n")

            values, loaded_files = load_config(config, ["dev", "hw_rev_b"])

            self.assertEqual(values["SAMPLE_PERIOD_MS"].value, 250)
            self.assertEqual([path.name for path in loaded_files], [
                "hss.conf",
                "hss-dev.conf",
                "hss-hw_rev_b.conf",
            ])

    def test_missing_profile_is_an_error(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "SAMPLE_PERIOD_MS=1000\n")

            with self.assertRaisesRegex(ConfigError, "missing config file"):
                load_config(config, ["missing"])

    def test_unknown_hss_key_is_an_error(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "HSS_NOT_A_REAL_KEY=y\n")

            with self.assertRaisesRegex(ConfigError, "unknown HSS config key"):
                load_config(config, [])

    def test_schema_range_validation(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "HSS_CONSOLE_STDIO_TIMEOUT_MS=-1\n")

            with self.assertRaisesRegex(ConfigError, "must be >= 0"):
                load_config(config, [])

    def test_max31865_schema_default_exists(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "")

            values, _ = load_config(config, [])

            self.assertIn("HSS_ENABLE_MAX31865", values)
            self.assertFalse(values["HSS_ENABLE_MAX31865"].value)

    def test_watchdog_schema_default_exists(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "")

            values, _ = load_config(config, [])

            self.assertIn("HSS_ENABLE_WATCHDOG", values)
            self.assertFalse(values["HSS_ENABLE_WATCHDOG"].value)

    def test_eeprom_schema_defaults_exist(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "")

            values, _ = load_config(config, [])

            self.assertIn("HSS_ENABLE_EEPROM_EMULATION", values)
            self.assertFalse(values["HSS_ENABLE_EEPROM_EMULATION"].value)
            self.assertEqual(values["HSS_EEPROM_FLASH_ORIGIN"].value, 0)
            self.assertEqual(values["HSS_EEPROM_FLASH_SIZE"].value, 0)
            self.assertEqual(values["HSS_EEPROM_PAGE_SIZE"].value, 0)
            self.assertEqual(values["HSS_EEPROM_SLOT_COUNT"].value, 0)

    def test_metadata_records_generated_macro_name(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "SAMPLE_PERIOD_MS=1000\n")

            values, loaded_files = load_config(config, [])
            meta = json.loads(generated_meta(values, [], loaded_files))

            self.assertEqual(meta["values"]["SAMPLE_PERIOD_MS"]["macro"], "CONFIG_SAMPLE_PERIOD_MS")
            self.assertFalse(meta["values"]["SAMPLE_PERIOD_MS"]["c_facing"])

    def test_cmake_compile_definitions_only_include_c_facing_values(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                'APP_DEVICE_NAME="demo"\nTOOL_UPLOAD_PORT="/dev/ttyUSB0"\n',
            )

            values, loaded_files = load_config(config, [])
            cmake = generated_cmake(values, [], loaded_files)

            self.assertIn('CONFIG_APP_DEVICE_NAME=\\"demo\\"', cmake)
            self.assertIn('HSS_CONFIG_VALUE_TOOL_UPLOAD_PORT "/dev/ttyUSB0"', cmake)
            self.assertNotIn("CONFIG_TOOL_UPLOAD_PORT", cmake)

    def test_eeprom_values_are_included_in_framework_compile_definitions(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                "HSS_ENABLE_EEPROM_EMULATION=y\n"
                "HSS_EEPROM_FLASH_ORIGIN=0x0800F800\n"
                "HSS_EEPROM_FLASH_SIZE=2048\n"
                "HSS_EEPROM_PAGE_SIZE=1024\n"
                "HSS_EEPROM_SLOT_COUNT=32\n",
            )

            values, loaded_files = load_config(config, [])
            cmake = generated_cmake(values, [], loaded_files)

            self.assertIn("HSS_ENABLE_EEPROM_EMULATION=1", cmake)
            self.assertIn("HSS_EEPROM_FLASH_ORIGIN=134281216", cmake)
            self.assertIn("HSS_EEPROM_FLASH_SIZE=2048", cmake)
            self.assertIn("HSS_EEPROM_PAGE_SIZE=1024", cmake)
            self.assertIn("HSS_EEPROM_SLOT_COUNT=32", cmake)

    def test_watchdog_value_is_included_in_framework_compile_definitions(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "HSS_ENABLE_WATCHDOG=y\n")

            values, loaded_files = load_config(config, [])
            cmake = generated_cmake(values, [], loaded_files)

            self.assertIn("HSS_ENABLE_WATCHDOG=1", cmake)

    def test_eeprom_requires_non_zero_geometry_when_enabled(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(root, "hss.conf", "HSS_ENABLE_EEPROM_EMULATION=y\n")

            with self.assertRaisesRegex(ConfigError, "HSS_EEPROM_FLASH_ORIGIN"):
                load_config(config, [])

    def test_eeprom_slot_count_must_fit_flash_capacity(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                "HSS_ENABLE_EEPROM_EMULATION=y\n"
                "HSS_EEPROM_FLASH_ORIGIN=0x0800F800\n"
                "HSS_EEPROM_FLASH_SIZE=16\n"
                "HSS_EEPROM_PAGE_SIZE=16\n"
                "HSS_EEPROM_SLOT_COUNT=3\n",
            )

            with self.assertRaisesRegex(ConfigError, "HSS_EEPROM_SLOT_COUNT"):
                load_config(config, [])

    def test_eeprom_origin_must_be_page_aligned(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            config = self.write(
                root,
                "hss.conf",
                "HSS_ENABLE_EEPROM_EMULATION=y\n"
                "HSS_EEPROM_FLASH_ORIGIN=0x0800F801\n"
                "HSS_EEPROM_FLASH_SIZE=2048\n"
                "HSS_EEPROM_PAGE_SIZE=1024\n"
                "HSS_EEPROM_SLOT_COUNT=32\n",
            )

            with self.assertRaisesRegex(ConfigError, "aligned to the EEPROM page size"):
                load_config(config, [])


if __name__ == "__main__":
    unittest.main()
