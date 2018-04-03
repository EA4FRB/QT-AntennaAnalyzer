/**
  ******************************************************************************
  * @file    sark_client.h
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    3-Apr-2018
  * @brief	 SARK-110 Commands processing
  ******************************************************************************
  * @copy
  *
  *  This file is a part of the "SARK110 Antenna Vector Impedance Analyzer" software
  *
  *  "SARK110 Antenna Vector Impedance Analyzer" software is free software: you can redistribute it
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
  *  along with "SARK110 Antenna Vector Impedance Analyzer" firmware.  If not,
  *  see <http://www.gnu.org/licenses/>.
  *
  * <h2><center>&copy; COPYRIGHT 2011-2018 Melchor Varela - EA4FRB </center></h2>
  *  Melchor Varela, Madrid, Spain.
  *  melchor.varela@gmail.com
  */

/** @addtogroup SARK110
  * @{
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SARK_CLIENT_H__
#define __SARK_CLIENT_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern int Sark_Connect (void);
extern int Sark_Close (void);
extern int Sark_SndRcv (uint8_t *tx, uint8_t *rx);
extern int Sark_Version (uint16_t *pu16Ver, uint8_t *pu8FW);
extern int Sark_Meas_Rx (uint32_t u32Freq, uint8_t bCal, uint8_t u8Samples, float *pfR, float *pfX, float *pfS21re, float *pfS21im);
extern int Sark_Meas_Rx_Eff (uint32_t u32Freq, uint32_t u32Step, uint8_t bCal, uint8_t u8Samples,
    float *pfR1, float *pfX1,
    float *pfR2, float *pfX2,
    float *pfR3, float *pfX3,
    float *pfR4, float *pfX4
    );
extern int Sark_Meas_Vect (uint32_t u32Freq, float *pfMagV, float *pfPhV,
                                           float *pfMagI, float *pfPhI );
extern int Sark_Meas_RF (uint32_t u32Freq, float *pfMagV, float *pfPhV,
                                           float *pfMagI, float *pfPhI );
extern int Sark_Meas_Vect_Thru (uint32_t u32Freq, float *pfMagVout, float *pfPhVout,
                                           float *pfMagVin, float *pfPhVin );
extern int Sark_Signal_Gen (uint32_t u32Freq, uint16_t u16Level, uint8_t u8Gain);
extern int Sark_BatteryStatus (uint8_t *pu8Vbus, uint16_t *pu16Volt, uint8_t *pu8Chr);
extern int Sark_GetKey (uint8_t *pu8Key);
extern int Sark_DiskInfo (uint32_t *pu32Tot, uint32_t *pu32Fre);
extern int Sark_DiskVolume (uint8_t *pu8Volume);
extern int Sark_Buzzer (uint16_t u16Freq, uint16_t u16Duration);
extern int Sark_Device_Reset (int16_t num);

#endif	 /* __SARK_CLIENT_H__ */

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2018 Melchor Varela - EA4FRB *****END OF FILE****/

