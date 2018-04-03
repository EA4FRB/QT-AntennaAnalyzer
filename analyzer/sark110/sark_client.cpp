/**
  ******************************************************************************
  * @file    sark_client.c
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    3-Apr-2018
  * @brief   SARK-110 Commands processing
  ******************************************************************************
  * @copy
  *
  *  This file is a part of the "SARK110 Antenna Vector Impedance Analyzer" software
  *
  *  "SARK110 Antenna Vector Impedance Analyzer software" is free software: you can redistribute it
  *  and/or modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation, either version 3 of the License,
  *  or (at your option) any later version.
  *
  *  "SARK110 Antenna Vector Impedance Analyzer" software is distributed in the hope that it will be
  *  useful,  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with "SARK110 Antenna Vector Impedance Analyzer" software.  If not,
  *  see <http://www.gnu.org/licenses/>.
  *
  * <h2><center>&copy; COPYRIGHT 2011-2018 Melchor Varela - EA4FRB </center></h2>
  *  Melchor Varela, Madrid, Spain.
  *  melchor.varela@gmail.com
  */

/** @addtogroup SARK110
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#if defined(_WIN32)
#include "windows.h"
#include "hid.h"
#endif
#include <string.h>
#if defined(__linux__)
#include "hidapi.h"
#endif
#include "sark_cmd_defs.h"
#include "sark_client.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TX_TIMEOUT			100
#define RX_TIMEOUT			220

/* Private macro -------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined(__linux__)
static hid_device *handle = NULL;
#endif

/* Private function prototypes -----------------------------------------------*/
static void Float2Buf (uint8_t tu8Buf[4], float fVal);
static void Int2Buf (uint8_t tu8Buf[4], uint32_t u32Val);
static void Buf2Int (uint32_t *pu32Val, uint8_t tu8Buf[4]);
static void Buf2Float (float *pfVal, uint8_t tu8Buf[4]);
static void Buf2Short (uint16_t *pu16Val, uint8_t tu8Buf[4]);
static void Short2Buf (uint8_t tu8Buf[4], uint16_t u16Val);
static uint16_t Float2Half(float value);
static float Half2Float(uint16_t value);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief Connects to the SARK-110 device
  *
  * @retval
  *			@li 1:      OK
  *			@li -1: 	device not detected
  */
int Sark_Connect (void)
{
#if defined(_WIN32)
    int iRc = rawhid_open(1, 0x0483, 0x5750, 0xFFB0, 0x0300);
    if (iRc <= 0)
        return -1;
#else
    // Initialize the hidapi library
    hid_init();

    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(0x0483, 0x5750, NULL);
    if (handle == NULL)
        return -1;
#endif
    return 1;
}

/**
  * @brief Close connection with the device
  *
  * @retval
  *			@li 1: Ok
  */
int Sark_Close (void)
{
#if defined(_WIN32)
    rawhid_close(0);
#else
    // Finalize the hidapi library
    hid_exit();
#endif
    return 1;
}


/**
  * @brief Send receive
  *
  * @param  None
  * @retval
  *			@li 1: Ok
  *			@li -1: error
  */
int Sark_SndRcv (uint8_t *tx, uint8_t *rx)
{
    int i;
    int rc;
#if defined(_WIN32)
    for (i=0; i < 5; i++)
    {
        rc = rawhid_send(0, tx, SARKCMD_TX_SIZE, TX_TIMEOUT);
        if (rc < 0)
            break;
        rc = rawhid_recv(0, rx, SARKCMD_RX_SIZE, RX_TIMEOUT);
        if (rc < 0)
            break;
        if (rx[0]==ANS_SARK_OK || rx[0]==ANS_SARK_ERR)
            break;
        else
            rc = -1;
    }
#else
    if (handle == NULL)
        return -1;

    for (i=0; i < 5; i++)
    {
        rc = hid_write(handle, tx, SARKCMD_TX_SIZE);
        if (rc < 0)
            break;
        rc = hid_read(handle, rx, SARKCMD_RX_SIZE);
        if (rc < 0)
            break;
        if (rx[0]==ANS_SARK_OK || rx[0]==ANS_SARK_ERR)
            break;
        else
            rc = -2;
    }
#endif
    return rc;
}

/**
  * @brief Get protocol version
  *
  * @param  pu16Ver		protocol version
  * @param  pu8Fw		FW version
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Version (uint16_t *pu16Ver, uint8_t *pu8FW)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_VERSION;

    rc = Sark_SndRcv (tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    Buf2Short(pu16Ver, &tu8Rx[1]);
    memcpy(pu8FW, &tu8Rx[3], SARKCMD_RX_SIZE-3);

    return 1;
}

/**
  * @brief Measure R and X
  *
  * @param  u32Freq		frequency
  * @param  bCal		{TRUE: OSL calibrated measurement; FALSE: not calibrated}
  * @param  u8Samples	Number of samples to average
  * @param  pfR			return R (real Z)
  * @param  pfX			return X (imag Z)
  * @param  pfS21re		return S21 real (SARK110 MK1)
  * @param  pfS21im		return S21 imag (SARK110 MK1)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Rx (uint32_t u32Freq, uint8_t bCal, uint8_t u8Samples, float *pfR, float *pfX, float *pfS21re, float *pfS21im)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_MEAS_RX;
    Int2Buf(&tu8Tx[1], u32Freq);
    if (bCal)
        tu8Tx[5] = PAR_SARK_CAL;
    else
        tu8Tx[5] = PAR_SARK_UNCAL;
    tu8Tx[6] = u8Samples;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    Buf2Float(pfR, &tu8Rx[1]);
    Buf2Float(pfX, &tu8Rx[5]);
    Buf2Float(pfS21re, &tu8Rx[9]);
    Buf2Float(pfS21im, &tu8Rx[13]);

    return 1;
}

/**
  * @brief Measure R and X - efficient
  *
  * @param  u32Freq		frequency
  * @param  u32Step		step
  * @param  bCal		{TRUE: OSL calibrated measurement; FALSE: not calibrated}
  * @param  u8Samples	Number of samples to average
  * @param  pfR			return R (real Z)
  * @param  pfX			return X (imag Z)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Rx_Eff (uint32_t u32Freq, uint32_t u32Step, uint8_t bCal, uint8_t u8Samples,
    float *pfR1, float *pfX1,
    float *pfR2, float *pfX2,
    float *pfR3, float *pfX3,
    float *pfR4, float *pfX4
    )
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_MEAS_RX_EFF;
    Int2Buf(&tu8Tx[1], u32Freq);
    Int2Buf(&tu8Tx[7], u32Step);
    if (bCal)
        tu8Tx[5] = PAR_SARK_CAL;
    else
        tu8Tx[5] = PAR_SARK_UNCAL;
    tu8Tx[6] = u8Samples;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }

    uint16_t u16R, u16X;
    float fR, fX;
    int offset = 1;

    Buf2Short(&u16R, &tu8Rx[offset]);
    fR = Half2Float(u16R);
    Buf2Short(&u16X, &tu8Rx[offset+2]);
    fX = Half2Float(u16X);
    *pfR1 = fR;
    *pfX1 = fX;
    offset += 4;

    Buf2Short(&u16R, &tu8Rx[offset]);
    fR = Half2Float(u16R);
    Buf2Short(&u16X, &tu8Rx[offset+2]);
    fX = Half2Float(u16X);
    *pfR2 = fR;
    *pfX2 = fX;
    offset += 4;

    Buf2Short(&u16R, &tu8Rx[offset]);
    fR = Half2Float(u16R);
    Buf2Short(&u16X, &tu8Rx[offset+2]);
    fX = Half2Float(u16X);
    *pfR3 = fR;
    *pfX3 = fX;
    offset += 4;

    Buf2Short(&u16R, &tu8Rx[offset]);
    fR = Half2Float(u16R);
    Buf2Short(&u16X, &tu8Rx[offset+2]);
    fX = Half2Float(u16X);
    *pfR4 = fR;
    *pfX4 = fX;
    offset += 4;

    return 1;
}

/**
  * @brief Measure raw vector
  *
  * @param  u32Freq		frequency
  * @param  pfMagV		magnitude voltage
  * @param  pfPhV		phase voltage
  * @param  pfMagI		magnitude current
  * @param  pfPhI		phase current
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Vect (uint32_t u32Freq, float *pfMagV, float *pfPhV, float *pfMagI, float *pfPhI )
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_MEAS_VECTOR;
    Int2Buf(&tu8Tx[1], u32Freq);
    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    Buf2Float(pfMagV, &tu8Rx[1]);
    Buf2Float(pfPhV, &tu8Rx[5]);
    Buf2Float(pfMagI, &tu8Rx[9]);
    Buf2Float(pfPhI, &tu8Rx[13]);

    return 1;
}

/**
  * @brief Measure RF
  *
  * @param  u32Freq		frequency
  * @param  pfMagV		magnitude voltage
  * @param  pfPhV		phase voltage
  * @param  pfMagI		magnitude current
  * @param  pfPhI		phase current
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_RF (uint32_t u32Freq, float *pfMagV, float *pfPhV, float *pfMagI, float *pfPhI )
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_MEAS_RF;
    Int2Buf(&tu8Tx[1], u32Freq);
    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    Buf2Float(pfMagV, &tu8Rx[1]);
    Buf2Float(pfPhV, &tu8Rx[5]);
    Buf2Float(pfMagI, &tu8Rx[9]);
    Buf2Float(pfPhI, &tu8Rx[13]);

    return 1;
}

/**
  * @brief Measure raw vector Thru (SARK110 MK1)
  *
  * @param  u32Freq		frequency
  * @param  pfMagVout	magnitude voltage out
  * @param  pfPhVout	phase voltage out
  * @param  pfMagVin	magnitude voltage in
  * @param  pfPhVin		phase voltage in
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Vect_Thru (uint32_t u32Freq, float *pfMagVout, float *pfPhVout, float *pfMagVin, float *pfPhVin )
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_MEAS_VEC_THRU;
    Int2Buf(&tu8Tx[1], u32Freq);
    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    Buf2Float(pfMagVout, &tu8Rx[1]);
    Buf2Float(pfPhVout, &tu8Rx[5]);
    Buf2Float(pfMagVin, &tu8Rx[9]);
    Buf2Float(pfPhVin, &tu8Rx[13]);

    return 1;
}

/**
  * @brief Signal generator
  *
  * @param  u32Freq		frequency
  * @param  u16Level	output level
  * @param  u8Gain		gain multiplier
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Signal_Gen (uint32_t u32Freq, uint16_t u16Level, uint8_t u8Gain)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_SARK_SIGNAL_GEN;
    Int2Buf(&tu8Tx[1], u32Freq);
    Short2Buf(&tu8Tx[5], u16Level);
    tu8Tx[7] = u8Gain;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    return 1;
}

/**
  * @brief Battery status
  *
  * @param  pu8Vbus		USB vbus value
  * @param  pu16Volt	Battery voltage
  * @param  pu8Chr		Charger status
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_BatteryStatus (uint8_t *pu8Vbus, uint16_t *pu16Volt, uint8_t *pu8Chr)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_BATT_STAT;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    *pu8Vbus = tu8Rx[1];
    Buf2Short(pu16Volt, &tu8Rx[2]);
    *pu8Chr = tu8Rx[4];

    return 1;
}

/**
  * @brief Get key press
  *
  * @param  pu8Key
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_GetKey (uint8_t *pu8Key)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_GET_KEY;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    *pu8Key = tu8Rx[1];

    return 1;
}

/**
  * @brief Resets device
  *
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Device_Reset (void)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_DEV_RST;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    return 1;
}

/**
  * @brief Disk information
  *
  * @param  pu32Tot
  * @param  pu32Fre
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_DiskInfo (uint32_t *pu32Tot, uint32_t *pu32Fre)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_DISK_INFO;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    Buf2Int(pu32Tot, &tu8Rx[1]);
    Buf2Int(pu32Fre, &tu8Rx[5]);

    return 1;
}

/**
  * @brief Disk volume
  *
  * @param  pu8Volume	volume name
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_DiskVolume (uint8_t *pu8Volume)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_DISK_VOLUME;

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    memcpy(pu8Volume, &tu8Rx[1], SARKCMD_RX_SIZE-1);

    return 1;
}

/**
  * @brief Buzzer
  *
  * @param  u16Freq		frequency
  * @param  u16Duration
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Buzzer (uint16_t u16Freq, uint16_t u16Duration)
{
    uint8_t tu8Rx[SARKCMD_RX_SIZE];
    uint8_t tu8Tx[SARKCMD_TX_SIZE];
    int rc;

    memset(tu8Tx, 0, SARKCMD_TX_SIZE);
    tu8Tx[0] = CMD_BUZZER;
    Short2Buf(&tu8Tx[1], u16Freq);
    Short2Buf(&tu8Tx[3], u16Duration);

    rc = Sark_SndRcv(tu8Tx, tu8Rx);
    if (rc < 0)
    {
        return -1;
    }
    if (tu8Rx[0]!=ANS_SARK_OK)
    {
        return -2;
    }
    return 1;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Float2Buf (uint8_t tu8Buf[4], float fVal)
{
    uint32_t u32Val = *((uint32_t*)(&fVal));
    Int2Buf(tu8Buf, u32Val);
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Int2Buf (uint8_t tu8Buf[4], uint32_t u32Val)
{
    tu8Buf[3] = (uint8_t)((u32Val&0xff000000)>>24);
    tu8Buf[2] = (uint8_t)((u32Val&0x00ff0000)>>16);
    tu8Buf[1] = (uint8_t)((u32Val&0x0000ff00)>>8);
    tu8Buf[0] = (uint8_t)((u32Val&0x000000ff)>>0);
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Buf2Float (float *pfVal, uint8_t tu8Buf[4])
{
    uint32_t u32Val;
    Buf2Int(&u32Val, tu8Buf);
    *((uint32_t*)(pfVal)) = u32Val;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Buf2Int (uint32_t *pu32Val, uint8_t tu8Buf[4])
{
    uint32_t u32Val;

    u32Val = tu8Buf[3] << 24;
    u32Val += tu8Buf[2] << 16;
    u32Val += tu8Buf[1] << 8;
    u32Val += tu8Buf[0] << 0;

    *pu32Val = u32Val;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Buf2Short (uint16_t *pu16Val, uint8_t tu8Buf[4])
{
    uint16_t u16Val;

    u16Val = tu8Buf[1] << 8;
    u16Val += tu8Buf[0] << 0;

    *pu16Val = u16Val;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Short2Buf (uint8_t tu8Buf[4], uint16_t u16Val)
{
    tu8Buf[1] = (uint8_t)((u16Val&0xff00)>>8);
    tu8Buf[0] = (uint8_t)((u16Val&0x00ff)>>0);
}

/**
  * @brief
  *
  * @param
  * @retval None
  */
union Bits
{
    float f;
    int32_t si;
    uint32_t ui;
};

#define C_SHIFT         13
#define C_SHIFTSIGN     16

#define C_INFN  0x7F800000  // flt32 infinity
#define C_MAXN  0x477FE000  // max flt16 normal as a flt32
#define C_MINN  0x38800000  // min flt16 normal as a flt32
#define C_SIGNN 0x80000000  // flt32 sign bit

#define C_INFC (C_INFN >> C_SHIFT)
#define C_NANN ((C_INFC + 1) << C_SHIFT)    // minimum flt16 nan as a flt32
#define C_MAXC (C_MAXN >> C_SHIFT)
#define C_MINC (C_MINN >> C_SHIFT)
#define C_SIGNC (C_SIGNN >> C_SHIFTSIGN)    // flt16 sign bit

#define C_MULN 0x52000000   // (1 << 23) / C_MINN
#define C_MULC 0x33800000   // C_MINN / (1 << (23 - C_SHIFT))

#define C_SUBC 0x003FF      // max flt32 subnormal down shifted
#define C_NORC 0x00400      // min flt32 normal down shifted

static int32_t const C_MAXD = C_INFC - C_MAXC - 1;
static int32_t const C_MIND = C_MINC - C_SUBC - 1;

static uint16_t Float2Half(float value)
{
    union Bits v, s;
    v.f = value;
    uint32_t sign = v.si & C_SIGNN;
    v.si ^= sign;
    sign >>= C_SHIFTSIGN; // logical shift
    s.si = C_MULN;
    s.si = s.f * v.f; // correct subnormals
    v.si ^= (s.si ^ v.si) & -(C_MINN > v.si);
    v.si ^= (C_INFN ^ v.si) & -((C_INFN > v.si) & (v.si > C_MAXN));
    v.si ^= (C_NANN ^ v.si) & -((C_NANN > v.si) & (v.si > C_INFN));
    v.ui >>= C_SHIFT; // logical shift
    v.si ^= ((v.si - C_MAXD) ^ v.si) & -(v.si > C_MAXC);
    v.si ^= ((v.si - C_MIND) ^ v.si) & -(v.si > C_SUBC);
    return v.ui | sign;
}

static float Half2Float(uint16_t value)
{
    union Bits v;
    v.ui = value;
    int32_t sign = v.si & C_SIGNC;
    v.si ^= sign;
    sign <<= C_SHIFTSIGN;
    v.si ^= ((v.si + C_MIND) ^ v.si) & -(v.si > C_SUBC);
    v.si ^= ((v.si + C_MAXD) ^ v.si) & -(v.si > C_MAXC);
    union Bits s;
    s.si = C_MULC;
    s.f *= v.si;
    int32_t mask = -(C_NORC > v.si);
    v.si <<= C_SHIFT;
    v.si ^= (s.si ^ v.si) & mask;
    v.si |= sign;
    return v.f;
}
/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2018 Melchor Varela - EA4FRB *****END OF FILE****/

