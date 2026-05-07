/***********************************************************************
 ** iic_devices.h
 ***********************************************************************
 ** Copyright (c) SEAGNAL SAS
 **
 ** This software is the property of SEAGNAL and is protected
 ** by International laws on author rights, by the conventions and
 ** international treaties on author rights and any other applicable
 ** law.
 **
 ** User is not allowed to use, copy, modify, distribute, and sell
 ** this software and its documentation for any purpose.
 **
 ***********************************************************************/

/* define against mutual inclusion */
#ifndef IIC_DEVICES_H_
#define IIC_DEVICES_H_

/**
 * @file iic_devices.h
 * IIC devices defines
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @date 2018
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/

//#define LF_SIMU_REPLY
#define LF_DISPLAY_IIC
//#define LF_DISPLAY_STATUS

 // IIC STUFF - SHT3X
#define C_SHT3X_DIS_IIC_ADDR_VSS 0x44
#define C_SHT3X_DIS_IIC_ADDR_VDD 0x45
#define C_SHT3X_DIS_STOP_MSB 0x30
#define C_SHT3X_DIS_STOP_LSB 0x93
#define C_SHT3X_DIS_PERIODIC_COMMAND_MSB_1MPS 0x21
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_1MPS_LOWREP 0x2D
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_1MPS_MEDIUMREP 0x26
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_1MPS_HIGHREP 0x30
#define C_SHT3X_DIS_PERIODIC_COMMAND_MSB_4MPS 0x23
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_4MPS_LOWREP 0x29
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_4MPS_MEDIUMREP 0x22
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_4MPS_HIGHREP 0x34
#define C_SHT3X_DIS_PERIODIC_COMMAND_MSB_10MPS 0x27
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_10MPS_LOWREP 0x2A
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_10MPS_MEDIUMREP 0x21
#define C_SHT3X_DIS_PERIODIC_COMMAND_LSB_10MPS_HIGHREP 0x37
#define C_SHT3X_DIS_FETCH_MSB 0xE0
#define C_SHT3X_DIS_FETCH_LSB 0x00
#define C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_MSB 0x24
#define C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_LSB_LOWREP 0x16
#define C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_LSB_MEDIUMREP 0x0B
#define C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_LSB_HIGHREP 0x00
#define C_SHT3X_DIS_CLEAR_STATUS_COMMAND_MSB 0x30
#define C_SHT3X_DIS_CLEAR_STATUS_COMMAND_LSB 0x41
#define C_SHT3X_DIS_WAIT 192

// IIC STUFF - MCP23008
#define C_MCP23008_IIC_ADDR_0 0x20
#define C_MCP23008_IIC_REG_IODIR 0x0
#define C_MCP23008_IIC_REG_IOPOL 0x1
#define C_MCP23008_IIC_REG_GPINTEN 0x2
#define C_MCP23008_IIC_REG_DEFVAL 0x3
#define C_MCP23008_IIC_REG_INTCON 0x4
#define C_MCP23008_IIC_REG_IOCON 0x5
#define C_MCP23008_IIC_REG_GPFU 0x6
#define C_MCP23008_IIC_REG_INTF 0x7
#define C_MCP23008_IIC_REG_INTCAP 0x8
#define C_MCP23008_IIC_REG_GPIO 0x9
#define C_MCP23008_IIC_REG_OLAT 0xA

// IIC STUFF
#define C_MAX7500_IIC_ADDR 0x48
#define C_ADP5050_IIC_ADDR 0x48
#define C_ADS7828_IIC_ADDR_VSS_VDD   0x4A
#define C_ADS7828_IIC_ADDR_VDD_VSS   0x49
#define C_ADS7828_IIC_ADDR_VDD_VDD   0x4B
#define C_ADS7828_IIC_DIFF_CH0_CH1   0x0F
#define C_ADS7828_IIC_DIFF_CH2_CH3   0x1F
#define C_ADS7828_IIC_DIFF_CH4_CH5   0x2F
#define C_ADS7828_IIC_DIFF_CH6_CH7   0x3F
#define C_ADS7828_IIC_DIFF_CH1_CH0   0x4F
#define C_ADS7828_IIC_DIFF_CH3_CH2   0x5F
#define C_ADS7828_IIC_DIFF_CH5_CH4   0x6F
#define C_ADS7828_IIC_DIFF_CH7_CH6   0x7F
#define C_ADS7828_IIC_SE_CH0   0x8F
#define C_ADS7828_IIC_SE_CH2   0x9F
#define C_ADS7828_IIC_SE_CH4   0xAF
#define C_ADS7828_IIC_SE_CH6   0xBF
#define C_ADS7828_IIC_SE_CH1   0xCF
#define C_ADS7828_IIC_SE_CH3   0xDF
#define C_ADS7828_IIC_SE_CH5   0xEF
#define C_ADS7828_IIC_SE_CH7   0xFF

// IIC STUFF
#define C_SI5341_IIC_ADDR 0x75
#define C_SI5341_PAGE 0x01
#define C_SI5341_STICKY_STATUS 0x11

// IIC stuff - SYSMON
#define C_SYSMON_IIC_ADDR 0x38
#define C_SYSMON_PAGE 0x00
#define C_SYSMON_READ_TEMP 0x8B
#define C_SYSMON_READ_VOUT 0x8D
#define C_SYSMON_READ 0x04
#define C_SYSMON_WRITE 0x08
#define C_SYSMON_ADDR_CONFIG 0xD0
#define C_SYSMON_DATA_CONFIG 0xD1
#define C_SYSMON_MEAS_TEMP 0x00
#define C_SYSMON_MEAS_VCCINT 0x01
#define C_SYSMON_MEAS_VCCAUX 0x02
#define C_SYSMON_MEAS_VCCBRAM 0x06
#define C_SYSMON_MAX_TEMP 0x20
#define C_SYSMON_MAX_VCCINT 0x21
#define C_SYSMON_MAX_VCCAUX 0x22
#define C_SYSMON_MAX_VCCBRAM 0x23
#define C_SYSMON_MIN_TEMP 0x24
#define C_SYSMON_MIN_VCCINT 0x25
#define C_SYSMON_MIN_VCCAUX 0x26
#define C_SYSMON_MIN_VCCBRAM 0x27
#define C_SYSMON_MEAS_VCCAUX0 0x10
#define C_SYSMON_MEAS_VCCAUX1 0x11
#define C_SYSMON_MEAS_VCCAUX2 0x12
#define C_SYSMON_MEAS_VCCAUX3 0x13
#define C_SYSMON_MEAS_VCCAUX4 0x14
#define C_SYSMON_MEAS_VCCAUX5 0x15
#define C_SYSMON_MEAS_VCCAUX6 0x16
#define C_SYSMON_MEAS_VCCAUX7 0x17

#endif
