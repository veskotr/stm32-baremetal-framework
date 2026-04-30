#include "port.h"
#include "mb.h"
#include "hss_modbus.h"

#define REG_INPUT_START  0
#define REG_INPUT_NREGS  10
#define REG_HOLDING_START  0
#define REG_HOLDING_NREGS  10

static uint16_t usRegInputBuf[REG_INPUT_NREGS];
static uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
extern hss_uart_id_t g_modbus_uart;
extern hss_timer_id_t g_modbus_timer;

void hss_modbus_init(const hss_modbus_config_t* cfg)
{
    g_modbus_uart = cfg->uart;
    g_modbus_timer = cfg->timer;

    eMBInit(MB_RTU,
            cfg->slave_id,
            0, // port (unused)
            cfg->baudrate,
            MB_PAR_NONE);


    eMBEnable();
}

void hss_modbus_poll(void)
{
    eMBPoll();
}

eMBErrorCode eMBRegInputCB(UCHAR* pucRegBuffer,
                           USHORT usAddress,
                           USHORT usNRegs)
{
    if ((usAddress >= REG_INPUT_START) &&
        (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
    {
        int i;

        usAddress -= REG_INPUT_START;

        for (i = 0; i < usNRegs; i++)
        {
            pucRegBuffer[2 * i] = (uint8_t)(usRegInputBuf[usAddress + i] >> 8);
            pucRegBuffer[2 * i + 1] = (uint8_t)(usRegInputBuf[usAddress + i] & 0xFF);
        }

        return MB_ENOERR;
    }

    return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(UCHAR* pucRegBuffer,
                             USHORT usAddress,
                             USHORT usNRegs,
                             eMBRegisterMode eMode)
{
    if ((usAddress >= REG_HOLDING_START) &&
        (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        int i;

        usAddress -= REG_HOLDING_START;

        if (eMode == MB_REG_READ)
        {
            for (i = 0; i < usNRegs; i++)
            {
                pucRegBuffer[2 * i] = (uint8_t)(usRegHoldingBuf[usAddress + i] >> 8);
                pucRegBuffer[2 * i + 1] = (uint8_t)(usRegHoldingBuf[usAddress + i] & 0xFF);
            }
        }
        else
        {
            for (i = 0; i < usNRegs; i++)
            {
                usRegHoldingBuf[usAddress + i] =
                    (pucRegBuffer[2 * i] << 8) |
                    pucRegBuffer[2 * i + 1];
            }
        }

        return MB_ENOERR;
    }

    return MB_ENOREG;
}

void hss_modbus_get_holding_register_value(uint16_t reg_addr, uint16_t* value)
{
    if (reg_addr < REG_HOLDING_NREGS)
    {
        *value = usRegHoldingBuf[reg_addr];
    }
    else
    {
        *value = 0; // Or some error code
    }
}


void hss_modbus_get_input_register_value(uint16_t reg_addr, uint16_t* value)
{
    if (reg_addr < REG_INPUT_NREGS)
    {
        *value = usRegInputBuf[reg_addr];
    }
    else
    {
        *value = 0; // Or some error code
    }
}

void hss_modbus_set_holding_register_value(uint16_t reg_addr, uint16_t value)
{
    if (reg_addr < REG_HOLDING_NREGS)
    {
        usRegHoldingBuf[reg_addr] = value;
    }
}

void hss_modbus_set_input_register_value(uint16_t reg_addr, uint16_t value)
{
    if (reg_addr < REG_INPUT_NREGS)
    {
        usRegInputBuf[reg_addr] = value;
    }
}

/* Coil and discrete-input registers are not used in this application.
   FreeModbus core always needs these callbacks to be defined, so provide
   stub implementations that simply return MB_ENOREG. */
eMBErrorCode eMBRegCoilsCB(UCHAR* pucRegBuffer,
                           USHORT usAddress,
                           USHORT usNCoils,
                           eMBRegisterMode eMode)
{
    (void)pucRegBuffer;
    (void)usAddress;
    (void)usNCoils;
    (void)eMode;
    return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR* pucRegBuffer,
                              USHORT usAddress,
                              USHORT usNDiscrete)
{
    (void)pucRegBuffer;
    (void)usAddress;
    (void)usNDiscrete;
    return MB_ENOREG;
}
