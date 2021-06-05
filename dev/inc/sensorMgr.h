/*
 * Copyright (c) 2020-2021 Ravi Chilwant
 * Apache-2.0
 * File name   : sensorMgr.h
 * Description : Getter interface for DISCO Boards different sensors.
 *               It uses the BSP_B-L475E-IOT01 library from Mbed 
 */

#ifndef _SENSORMGR_H_
#define _SENSORMGR_H_

#include "mbed.h"

// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"

class sensorMgr {
    public:
    sensorMgr(void);
    ~sensorMgr(void);

    /*Initialization of on-board sensor modules */
    private:

    protected:

};

#endif // _SENSORMGR_H_