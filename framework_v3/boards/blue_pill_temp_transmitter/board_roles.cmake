# Framework-owned semantic role mapping for blue_pill_temp_transmitter.
# This file is created once by tools/sync_board.py and is safe to edit.
# Rerunning the sync script will not overwrite it.

# Examples:
set(BOARD_ROLE_MODBUS_TIMER TIM2)
# TODO: enable after CubeMX regenerates USART2 and exposes huart2.
# set(BOARD_ROLE_DEBUG_UART USART2)
set(BOARD_ROLE_CONSOLE_UART USART1)
set(BOARD_ROLE_MODBUS_UART USART1)
set(BOARD_ROLE_SENSOR_SPI SPI1)
# TODO: enable after CubeMX configures a dedicated sensor chip-select GPIO.
# set(BOARD_ROLE_SENSOR_CS PA4)
# set(BOARD_ROLE_SENSOR_CS_ACTIVE_LOW ON)
set(BOARD_ROLE_STATUS_LED PC13)
set(BOARD_ROLE_STATUS_LED_ACTIVE_LOW ON)
