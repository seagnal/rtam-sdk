/***********************************************************************
** rtam-eval.cc
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

/**
* @file rtam-eval.cc
* Plugin RTAM_EVAL.
*
* @author SEAGNAL (johann.baudy@seagnal.fr)
* @author SEAGNAL (romain.pignatari@seagnal.fr)
* @date 2017
*
* @version 1.0 Original version
*/

/***********************************************************************
* Includes
***********************************************************************/

#include <rtam-eval.hh>
#include "rtam-eval_iic.hh"
#include <rtam/api.hh>
#include "api.hh"



using namespace std;
using namespace master::plugins::rtam_eval;
namespace rtam = master::plugins::fec;
namespace rtam_eval = master::plugins::rtam_eval;

CT_GUARD<CT_PORT_NODE> CT_RTAM_EVAL::f_iic_get_rx32(void) {
	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_IIC);
	pc_node->set_data<uint32_t>(1 + 4*2);
	//OFFSET:  2*4(SHT)

	// BUS INT
	// -PA: SHT en 0x45
	// -CM: ADS7828 en 0x4A
	// -PA: SHT en 0x44
	// -CM: ADP5050 en 0x48
	// BUS EXT NO

	/* Bad first IIC command on bus 0 (to get ride of the BUG) */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0xAD};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(0x7E);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	/* Internal Temperature sensor */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_SHT3X_DIS_PERIODIC_COMMAND_MSB_10MPS, C_SHT3X_DIS_PERIODIC_COMMAND_LSB_10MPS_HIGHREP};
		//uint8_t ai_data[] = {C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_MSB, C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_LSB_LOWREP};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VSS);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_SHT3X_DIS_CLEAR_STATUS_COMMAND_MSB, C_SHT3X_DIS_CLEAR_STATUS_COMMAND_LSB};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VSS);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	/* External Temperature sensor */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_SHT3X_DIS_PERIODIC_COMMAND_MSB_10MPS, C_SHT3X_DIS_PERIODIC_COMMAND_LSB_10MPS_HIGHREP};
		//uint8_t ai_data[] = {C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_MSB, C_SHT3X_DIS_SINGLE_NOSTRECH_COMMAND_LSB_LOWREP};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(1);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_SHT3X_DIS_CLEAR_STATUS_COMMAND_MSB, C_SHT3X_DIS_CLEAR_STATUS_COMMAND_LSB};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(1);
	}

	/*** DATA READING START HERE ****/

	/* READ ADS7828 - CH0 - POWER AMP*/
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH0};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH1 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH1};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH2 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH2};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH3 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH3};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH4 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH4};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH5 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH5};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH6 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH6};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* READ ADS7828 - CH7 */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_ADS7828_IIC_SE_CH7};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_ADS7828_IIC_ADDR_VSS_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Pool */
	/*{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[C_NB_IIC_WAIT_SHT_RX32] = {0,};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(0x01);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}*/

	/* Temperature sensor Internal */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_SHT3X_DIS_FETCH_MSB, C_SHT3X_DIS_FETCH_LSB};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VSS);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(6);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VSS);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Temperature sensor External */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {C_SHT3X_DIS_FETCH_MSB, C_SHT3X_DIS_FETCH_LSB};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(1);
	}

	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(6);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SHT3X_DIS_IIC_ADDR_VDD);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(1);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Temperature sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_TEMP, C_SYSMON_READ};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Max Temp sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MAX_TEMP, C_SYSMON_READ};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Min Temp sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MIN_TEMP, C_SYSMON_READ};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VCCINT sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCINT, C_SYSMON_READ};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VCCAUX sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX, C_SYSMON_READ};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VCCBRAM sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCBRAM, C_SYSMON_READ};
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Max VCCINT sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MAX_VCCINT, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Max VCCAUX sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MAX_VCCAUX, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Max VCCBRAM sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MAX_VCCBRAM, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Min VCCINT sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MIN_VCCINT, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Min VCCAUX sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MIN_VCCAUX, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* Min VCCBRAM sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MIN_VCCBRAM, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX0 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX0, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX1 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX1, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX2 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX2, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX3 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX3, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX4 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX4, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX5 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX5, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX6 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX6, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	/* VAUX7 sensor FPGA */
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		uint8_t ai_data[] = {0x00, 0x00, C_SYSMON_MEAS_VCCAUX7, C_SYSMON_READ};//C_SYSMON_PAGE, C_SYSMON_MEAS_VCCINT };
		pc_cmd->memcpy_from_buffer((char*)ai_data, sizeof(ai_data));
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
	}
	{
		auto pc_cmd = pc_node->add(rtam::E_ID_IIC_CMD);
		pc_cmd->set_data<uint32_t>(2);
		pc_cmd->add(rtam::E_ID_IIC_ADDR)->set_data<uint8_t>(C_SYSMON_IIC_ADDR);
		pc_cmd->add(rtam::E_ID_IIC_BUS)->set_data<uint8_t>(0);
		pc_cmd->add(rtam::E_ID_IIC_READ);
	}

	{
		size_t sz_expected = /*12+*/1 + 2*4;
		sz_expected += C_NB_VOLTAGE_RX32*C_NB_CMD_IIC_READ_ADS7828_CHANNEL;// READ VOLTAGE STATUS
		//sz_expected += C_NB_IIC_WAIT_SHT_RX32;
		sz_expected += C_NB_SHT_STATUS*C_NB_CMD_IIC_READ_SHT3X;// READ SHT STATUS
		sz_expected +=  (C_NB_TEMP_FPGA+C_NB_VOLTAGE_FPGA)*C_NB_CMD_IIC_READ_SYSMON;// READ SYSMON VALUES

		size_t sz_get = 0;
		for(auto pc_tmp : pc_node->get_childs(rtam::E_ID_IIC_CMD)) {
			if(pc_tmp->has(rtam::E_ID_IIC_READ)) {
			sz_get += pc_tmp->get_data<uint32_t>();
			} else {
			sz_get += pc_tmp->get_size();
			}
		}
		_DBG << _V(sz_expected) <<_V(sz_get);
		M_ASSERT(sz_get == sz_expected);
	}

	return pc_node;
}

int CT_RTAM_EVAL::f_iic_status_update_rx32(
	CT_GUARD<CT_PORT_NODE> const & in_rpc_node,
	uint32_t in_i_hw_uid,
	bool in_b_debug) {

	bool b_error = false;

	/* parse IIC reply */
	auto s_res = in_rpc_node->get_childs(rtam::E_ID_IIC_REPLY);
	auto & s_status_hw = _m_status_hw[in_i_hw_uid];

	{
		uint i_offset_config = 0;
		i_offset_config += 1 + 2*4; // Config SHT

#if 0
		auto s_res = in_rpc_node->get_childs(rtam::E_ID_IIC_REPLY);
		for(uint i=0; i<s_res.size(); i++) {
		int i_data = (int)s_res[i]->get_data<uint8_t>();
		int i_error = (int)s_res[i]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
		int i_addr = (int)s_res[i]->get(rtam::E_ID_IIC_ADDR)->get_data<uint8_t>();
		int i_bus = (int)s_res[i]->get(rtam::E_ID_IIC_BUS)->get_data<uint8_t>();

		int i_cnt = (int)s_res[i]->get(rtam::E_ID_IIC_CNT)->get_data<uint8_t>();
		_DBG <<  _V(i) << std::hex <<  _V(i_data) << _V(i_error)<< _V(i_addr)<< _V(i_bus) << _V(i_cnt) ;
		}
#endif
		size_t sz_expected = i_offset_config;
		sz_expected += C_NB_VOLTAGE_RX32*C_NB_CMD_IIC_READ_ADS7828_CHANNEL;// READ VOLTAGE STATUS
		//sz_expected += C_NB_IIC_WAIT_SHT_RX32;
		sz_expected += C_NB_SHT_STATUS*C_NB_CMD_IIC_READ_SHT3X;// READ SHT STATUS
		sz_expected += (C_NB_TEMP_FPGA+C_NB_VOLTAGE_FPGA)*C_NB_CMD_IIC_READ_SYSMON;// READ SYSMON VALUES
		// SET PA GPIO (REARM)

		if(s_res.size() != sz_expected) {
			_CRIT << "Abnormal IIC reply size" <<_V(s_res.size()) <<_V(sz_expected) << std::hex << _V(in_i_hw_uid);
			b_error = true;
			goto out_err;
		}

		//_DBG << "VOLTAGE";
		/* Voltage */
		{
			char const * astr_voltage_name[]       ={ "0V85", "1V8", "CORE_0V9", "TERM_1V2", "3V3", "12V", "DRVDD", "DVDD"};
			double af_scale[C_NB_VOLTAGE_STATUS]    ={    1,     2,          1,          1,     2,    10,       1,      1 };
			double af_offset[C_NB_VOLTAGE_STATUS]   ={   0,      0,          0,          0,     0,     0,       0,      0 };
			double af_min_value[C_NB_VOLTAGE_STATUS]={ 0.8,    1.7,       0.85,        1.1,   3.2,   4.8,     1.7,    1.3 };
			double af_max_value[C_NB_VOLTAGE_STATUS]={ 0.9,    1.9,       0.95,        1.3,   3.4,  14.0,     1.9,    1.5 };

			for(int ii = 0; ii < C_NB_VOLTAGE_STATUS; ii++) {
				if(ii < C_NB_VOLTAGE_RX32) {
					uint32_t i_offset = i_offset_config;
					i_offset += C_NB_CMD_IIC_READ_ADS7828_CHANNEL*ii+1;

					int i_error = 0;
					M_ASSERT(i_offset+1 < s_res.size());
					i_error = s_res[i_offset]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
					i_error |= s_res[i_offset+1]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();

					double f_tmp;
					double f_tmp2;
					if(!i_error) {
						uint32_t i_tmp1 = s_res[i_offset]->get_data<uint8_t>();
						uint32_t i_tmp2 = s_res[i_offset+1]->get_data<uint8_t>();
						f_tmp2 = double(i_tmp1*256.0+i_tmp2);
						f_tmp = f_tmp2/(4096.0)*2.5*double(af_scale[ii])+double(af_offset[ii]);
					} else {
						//_WARN << "BUG: "<<_V( s_res[3*ii+1+i_offset]->second->get(rtam::E_ID_IIC_ADDR)->get_data<uint8_t>())
						f_tmp = nan("");
					}
					//_DBG << _V(ii) << _V(i_error) << _V(f_tmp) << _V(3*ii+1+i_offset) << _V(3*ii+2+i_offset) << _V(f_tmp2) << "  " << ac_voltage_name[ii];
					s_status_hw.af_voltages[ii] = f_tmp;
					s_status_hw.astr_voltages_name[ii] = astr_voltage_name[ii];

					if(f_tmp < af_min_value[ii]) {
						//_CRIT << "Abnormal min value" <<_V(f_tmp) <<_V(min_value[ii]);
						b_error = true;
					}

					if(f_tmp > af_max_value[ii]) {
						//_CRIT << "Abnormal max value" <<_V(f_tmp) <<_V(max_value[ii]);
						b_error = true;
					}
				} else {
					s_status_hw.af_voltages[ii] = nan("");
					s_status_hw.astr_voltages_name[ii] = "";
				}
			}
		}

		//_DBG << "SHT";
		/* SHT Temperature */
		{
			char const * ac_sht_name[] = {"RX32-INT", "RX32-EXT"};
			for(int ii = 0; ii < C_NB_SHT_STATUS; ii++) {
				if(ii < 2) {
					int i_error = 0;
					uint32_t i_offset = i_offset_config;
					i_offset += C_NB_VOLTAGE_RX32*C_NB_CMD_IIC_READ_ADS7828_CHANNEL;// READ VOLTAGE STATUS
					//i_offset += C_NB_IIC_WAIT_SHT_RX32;
					i_offset +=  2 + ii*C_NB_CMD_IIC_READ_SHT3X;
					//_DBG << _V(ii)<< _V(i_offset);
					M_ASSERT(i_offset+4 < s_res.size());

					i_error |= s_res[i_offset+0]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
					i_error |= s_res[i_offset+1]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
					i_error |= s_res[i_offset+3]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
					i_error |= s_res[i_offset+4]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();

					if(!i_error) {
						uint32_t i_tmp1 = s_res[i_offset+0]->get_data<uint8_t>();
						uint32_t i_tmp2 = s_res[i_offset+1]->get_data<uint8_t>();
						uint32_t i_tmp3 = s_res[i_offset+3]->get_data<uint8_t>();
						uint32_t i_tmp4 = s_res[i_offset+4]->get_data<uint8_t>();
						//_DBG << _V(i_tmp1)  << _V(i_tmp2)  << _V(i_tmp3)  << _V(i_tmp4);

						double f_temp = -45.0+175.0*double(i_tmp1*256+ i_tmp2)/double((1<<16)-1);
						double f_hum = 100.0*double(i_tmp3*256 + i_tmp4)/double((1<<16)-1);
						//_DBG << _V(f_temp) << _V(f_hum);

						s_status_hw.af_sht_temperature[ii] = f_temp;
						s_status_hw.af_sht_humidity[ii] = f_hum;

						if(f_temp >= 60.0) {
							_CRIT << "Abnormal temperature value" <<_V(f_temp);
							b_error = true;
						}
					} else {
						s_status_hw.af_sht_temperature[ii] = nan("");
						s_status_hw.af_sht_humidity[ii] = nan("");
					}
					s_status_hw.astr_sht_name[ii] = ac_sht_name[ii];
				} else {
					s_status_hw.af_sht_temperature[ii] = nan("");
					s_status_hw.af_sht_humidity[ii] = nan("");
					s_status_hw.astr_sht_name[ii] = "";
				}
			}
		}

		//_DBG << "SYSMON Temp";
		/* SYStem MONitor into ARTIX */
		{
			char const * ac_fpga_name[] = {"FPGA-TEMP", "FPGA-MaxTEMP", "FPGA-MinTEMP"};
			for(int ii = 0; ii <  (C_NB_TEMP_FPGA); ii++) {
				int i_error = 0;
				uint32_t i_offset = i_offset_config;
				i_offset += C_NB_VOLTAGE_RX32*C_NB_CMD_IIC_READ_ADS7828_CHANNEL;// READ VOLTAGE STATUS
				i_offset += C_NB_SHT_STATUS*C_NB_CMD_IIC_READ_SHT3X;// READ TEMP SHT
				i_offset += (ii+1)*C_NB_CMD_IIC_READ_SYSMON -2;
				//_DBG << "FPGA Temp" << _V(i_offset) << _V(s_res.size());
				M_ASSERT(i_offset < s_res.size());

				i_error |= s_res[i_offset+0]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
				i_error |= s_res[i_offset+1]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();

				if(!i_error) {
					uint32_t i_tmp1 = s_res[i_offset+0]->get_data<uint8_t>();
					uint32_t i_tmp2 = s_res[i_offset+1]->get_data<uint8_t>();

					uint16_t i_tmp = (i_tmp2*256+ i_tmp1);
					double   f_temp = ((double)i_tmp*509.3140064)/pow(2.0,16)-280.23087870; //eq 2.11 UG580 //(double)i_val*pow(2,i_exp);
					//_DBG << std::hex << _V(i_tmp1) << std::hex << _V(i_tmp2) << std::hex << _V(i_tmp) << _V(f_temp);

					s_status_hw.af_fpga_temperature[ii] = f_temp;

					if(f_temp >= 80.0) {
						_CRIT << "Abnormal temperature value" << _V(ac_fpga_name[ii]) << _V(f_temp);
						b_error = true;
					}

				} else {
					s_status_hw.af_fpga_temperature[ii] = nan("");
					s_status_hw.astr_fpga_name[ii] = "";
				}

				s_status_hw.astr_fpga_name[ii] = ac_fpga_name[ii];

			}
		}

		//_DBG << "SYSMON Voltage";
		/* SYStem MONitor into ARTIX */
		{
			char const * ac_fpga_name[] =          {"FPGA-VCCINT", "FPGA-VCCAUX", "FPGA-VCCBRAM", "FPGA-MaxINT", "FPGA-MaxAUX", "FPGA-MaxBRAM",  "FPGA-MinINT", "FPGA-MinAUX", "FPGA-MinBRAM", "AVDD", "AVVD2", "DVDD", "DRVDD", "2_AVDD", "2AVDD2", "CORE", "TERM"};
			double af_scale[C_NB_VOLTAGE_FPGA]    ={           3,             3,              3,             3,             3,              3,              3,             3,              3,      3,       6,      3,       4,        3,        6,      2,      2 };
			double af_offset[C_NB_VOLTAGE_FPGA]   ={           0,             0,              0,             0,             0,              0,              0,             0,              0,      0,       0,      0,       0,        0,        0,      0,      0 };
			double af_min_value[C_NB_VOLTAGE_FPGA]={         0.8,           1.7,            0.8,           0.8,           1.7,            0.8,            0.8,           1.7,            0.8,    1.7,     2.9,    1.3,     1.7,      1.7,      2.9,    0.85,   1.1 };
			double af_max_value[C_NB_VOLTAGE_FPGA]={         0.9,           1.9,            0.9,           0.9,           1.9,            0.9,            0.9,           1.9,            0.9,    1.9,     3.1,    1.5,     1.9,      1.9,      3.1,    0.95,   1.3 };

			for(int ii = 0; ii < C_NB_VOLTAGE_FPGA; ii++) {
				int i_error = 0;
				uint32_t i_offset = i_offset_config;
				i_offset += C_NB_VOLTAGE_RX32*C_NB_CMD_IIC_READ_ADS7828_CHANNEL;// READ VOLTAGE STATUS
				i_offset += C_NB_SHT_STATUS*C_NB_CMD_IIC_READ_SHT3X;// READ TEMP SHT
				i_offset += C_NB_TEMP_FPGA*C_NB_CMD_IIC_READ_SYSMON;
				i_offset += (ii+1)*C_NB_CMD_IIC_READ_SYSMON -2;
				//_DBG << "FPGA VOLTAGE" << _V(i_offset) << _V(s_res.size());
				M_ASSERT(i_offset < s_res.size());

				i_error |= s_res[i_offset+0]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
				i_error |= s_res[i_offset+1]->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();

				if(!i_error) {
					uint32_t i_tmp1 = s_res[i_offset+0]->get_data<uint8_t>();
					uint32_t i_tmp2 = s_res[i_offset+1]->get_data<uint8_t>();

					uint16_t i_tmp = (i_tmp2*256+ i_tmp1);
					//int16_t i_exp = (0x8000&i_tmp) ? ((0x7800&i_tmp)>>11)-0x0010 : ((0x7800&i_tmp)>>11);
					//int16_t i_val = (0x0400&i_tmp) ? (0x03ff&i_tmp)-0x0400 : (0x03ff&i_tmp);
					double   f_val = ((double)i_tmp)/pow(2.0,16); //eq 2.14 UG580 //(double)i_val*pow(2,i_exp);
					//_DBG << _V(f_val);
					f_val *= double(af_scale[ii]);
					//_DBG << _V(f_val);
					f_val += double(af_offset[ii]);
					//_DBG << std::hex << _V(i_tmp1) << std::hex << _V(i_tmp2) << std::hex << _V(i_tmp) << _V(f_val);

					s_status_hw.af_fpga_voltages[ii] = f_val;

					if(f_val < af_min_value[ii]) {
						_CRIT << "Abnormal min value" << _V(ac_fpga_name[ii]) << _V(f_val) <<_V(af_min_value[ii]);
						b_error = true;
					}

					if(f_val > af_max_value[ii]) {
						_CRIT << "Abnormal max value" << _V(ac_fpga_name[ii]) << _V(f_val) <<_V(af_max_value[ii]);
						b_error = true;
					}

				} else {
					s_status_hw.af_fpga_voltages[ii] = nan("");
					s_status_hw.astr_fpga_voltages_name[ii] = "";
				}

				s_status_hw.astr_fpga_voltages_name[ii] = ac_fpga_name[ii];
			}
		}
	}
#if 1
	if(in_b_debug) {
		std::cout << "\t (" << s_status_hw.i_error_cnt << ")"<< std::endl;
		for(int ii = 0; ii < C_NB_TEMP_FPGA; ii++) {
			std::cout << s_status_hw.astr_fpga_name[ii] << "(" << std::setw(6)
				<< std::setprecision(4) << s_status_hw.af_fpga_temperature[ii] << ") ";
		}
		std::cout << std::endl;
		for(int ii = 0; ii < C_NB_SHT_STATUS; ii++) {
			std::cout << s_status_hw.astr_sht_name[ii] << " : H° " <<  s_status_hw.af_sht_humidity[ii] << "\t T° " << s_status_hw.af_sht_temperature[ii] << std::endl;
		}
		std::cout << std::endl;
		for(int ii = 0; ii < C_NB_VOLTAGE_STATUS; ii++) {
			std::cout << s_status_hw.astr_voltages_name[ii] << "(" << std::setw(6)
			<< std::setprecision(4) << s_status_hw.af_voltages[ii] << ") ";
		}
		std::cout << std::endl;
		for(int ii = 0; ii < C_NB_VOLTAGE_FPGA; ii++) {
			std::cout << s_status_hw.af_fpga_voltages[ii] << "(" << std::setw(6)
			<< std::setprecision(4) << s_status_hw.astr_fpga_voltages_name[ii] << ") ";
		}
		std::cout << std::endl;
	}
#endif
	out_err:
	if(b_error) {
		return EC_FAILURE;
	} else {
		return EC_SUCCESS;
	}
}

void CT_RTAM_EVAL::f_iic_display_status(void) {

	for (auto && pc_it : _m_status_hw) {
		std::cout << " - HW "<< std::hex << pc_it.first << " : " << std::endl;

		std::cout << "   * T, H (°C,%RH) : "<< std::hex << pc_it.first << " : ";
		for(int i = 0; i<C_NB_SHT_STATUS; i++) {
			std::cout << " ("<< f_string_format("%03.1f°", pc_it.second.af_sht_temperature[i])<<", "<< f_string_format("%03.1f%%", pc_it.second.af_sht_humidity[i])<<")";
		}
		std::cout << std::endl;

		std::cout << "   * V : "<< std::hex << pc_it.first << " : ";
		for(int i = 0; i< C_NB_VOLTAGE_STATUS; i++) {
			if(pc_it.second.af_voltages[i] == pc_it.second.af_voltages[i]) {
				std::cout <<  f_string_format("%s(%03.2fV) ", pc_it.second.astr_voltages_name[i].c_str(), pc_it.second.af_voltages[i]);
		}
		}
		std::cout << std::endl;

		std::cout << "   * V : "<< std::hex << pc_it.first << " : ";
		for(int i = 0; i< C_NB_VOLTAGE_FPGA; i++) {
			if(pc_it.second.af_fpga_voltages[i] == pc_it.second.af_fpga_voltages[i]) {
				std::cout <<  f_string_format("%s(%04.3fV) ", pc_it.second.astr_fpga_voltages_name[i].c_str(), pc_it.second.af_fpga_voltages[i]);
		}
		}
		std::cout << std::endl;

		std::cout << "   * T(°C) : "<< std::hex << pc_it.first << " : ";
		for(int i = 0; i<C_NB_TEMP_FPGA; i++) {
			std::cout << f_string_format("%s(%03.1f°) ",  pc_it.second.astr_fpga_name[i].c_str(), pc_it.second.af_fpga_temperature[i]);
		}
		std::cout << std::endl;
	}
}

void CT_RTAM_EVAL::f_iic_update_node_status(ST_STATUS & inout_rs_status, CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {

	double f_temperature;
	double f_humidity;
	std::string str_name;
	double f_voltage;
	std::string str_voltage_name;

	in_rpc_node->add(rtam_eval::E_ID_STATUS_HW_VERSION)->set_data<std::string>(inout_rs_status.str_version_hw);
	for(int i = 0; i<C_NB_SHT_STATUS; i++) {
		f_temperature = inout_rs_status.af_sht_temperature[i];
		f_humidity = inout_rs_status.af_sht_humidity[i];
		str_name = inout_rs_status.astr_sht_name[i];
		auto pc_node_tmp =  in_rpc_node->add(rtam_eval::E_ID_STATUS_HW_SHT);
		pc_node_tmp->add(rtam_eval::E_ID_STATUS_HW_SHT_TEMPERATURE)->set_data<double>(f_temperature);
		pc_node_tmp->add(rtam_eval::E_ID_STATUS_HW_SHT_HUMIDITY)->set_data<double>(f_humidity);
		pc_node_tmp->add(rtam_eval::E_ID_STATUS_HW_SHT_NAME)->set_data<std::string>(str_name);
	}

	for(int i = 0; i< C_NB_VOLTAGE_STATUS; i++) {
		f_voltage = inout_rs_status.af_voltages[i];
		str_voltage_name = inout_rs_status.astr_voltages_name[i];
		auto pc_node_tmp =  in_rpc_node->add(rtam_eval::E_ID_STATUS_HW_VOLTAGE);
		pc_node_tmp->add(rtam_eval::E_ID_STATUS_HW_VOLTAGE_VALUE)->set_data<double>(f_voltage);
		pc_node_tmp->add(rtam_eval::E_ID_STATUS_HW_VOLTAGE_NAME)->set_data<std::string>(str_voltage_name);
	}

}

void CT_RTAM_EVAL::f_iic_timeout(void) {
    uint64_t i_now = f_get_time_ns64();
    for(auto pc_it = _m_status_hw.begin(); pc_it != _m_status_hw.end(); ) {
		if(M_ABS(i_now - pc_it->second.i_time_last_seen) > 5*1e9) {
			pc_it = _m_status_hw.erase(pc_it);
		} else {
			pc_it++;
		}
    }
}

int CT_RTAM_EVAL::f_iic_status_update(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
	int ec;
	M_ASSERT(in_rpc_node->has(rtam::E_ID_SYSTEM_UID));
	M_ASSERT(in_rpc_node->has(rtam::E_ID_HW_UID));
	int32_t i_error = 0;
	bool b_debug = false;

	uint32_t i_hw_uid = in_rpc_node->get(rtam::E_ID_HW_UID)->get_data<uint32_t>();
	if(in_rpc_node->has(rtam::E_ID_HW_GPI)){
		_m_gpi_hw[i_hw_uid] = in_rpc_node->get(rtam::E_ID_HW_GPI)->get_data<uint32_t>();
	}
	auto & s_status_hw = _m_status_hw[i_hw_uid];

	//_DBG << "IIC status received" << std::hex <<_V(i_hw_uid)<<_V(i_system_uid);
#if 0
	uint32_t i_system_uid = in_rpc_node->get(rtam::E_ID_SYSTEM_UID)->get_data<uint32_t>();
	auto s_res = in_rpc_node->find(rtam::E_ID_IIC_REPLY);
	for(uint i=0; i<s_res.size(); i++) {
	int i_data = (int)s_res[i]->second->get_data<uint8_t>();
	int i_error = (int)s_res[i]->second->get(rtam::E_ID_IIC_ERROR)->get_data<uint8_t>();
	int i_addr = (int)s_res[i]->second->get(rtam::E_ID_IIC_ADDR)->get_data<uint8_t>();
	int i_bus = (int)s_res[i]->second->get(rtam::E_ID_IIC_BUS)->get_data<uint8_t>();

	int i_cnt = (int)s_res[i]->second->get(rtam::E_ID_IIC_CNT)->get_data<uint8_t>();
	_DBG <<  _V(i) << std::hex << _V(i_system_uid) <<_V(i_hw_uid)<<_V(i_data) << _V(i_error)<< _V(i_addr)<< _V(i_bus) << _V(i_cnt) ;}
#endif

	//if(i_hw_uid == 0x40) {
		//b_debug = true;
	//}

	switch(_e_antenna_config) {
		case E_ANTENNA_CONFIG_ANTENNE1:
		case E_ANTENNA_CONFIG_ANTENNE2:
		case E_ANTENNA_CONFIG_MAQUETTE16:
		case E_ANTENNA_CONFIG_MAQUETTE24:
		case E_ANTENNA_CONFIG_2XEVAL16:
		//case E_ANTENNA_CONFIG_ONE_ANTENNA:
		i_error = f_iic_status_update_rx32(in_rpc_node, i_hw_uid, b_debug);
		break;
	}


	if(i_error) {
		if(s_status_hw.i_error_cnt < C_DEVICE_ERROR_MAX) {
			s_status_hw.i_error_cnt ++ ;
		}
	} else {
		s_status_hw.i_error_cnt = 0;
	}

	s_status_hw.i_time_last_seen = f_get_time_ns64();

	ec = EC_SUCCESS;
	//out_err:
	return ec;
}
