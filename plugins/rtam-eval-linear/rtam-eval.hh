/***********************************************************************
 ** rtam-eval.hh
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
#ifndef RTAM_EVAL_HH_
#define RTAM_EVAL_HH_

/**
 * @file rtam-eval.hh
 * rtam-eval plugin definition.
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @date 2017
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <plugin.hh>
#include <host.hh>
#include <c/randn.h>
#include "api.hh"

/* string is needed for struct generation with swig */

namespace master {
namespace plugins {
namespace rtam_eval {

/***********************************************************************
 * Defines
 ***********************************************************************/

enum ET_PORTS {
    E_PORT_RTAM = 3,
};

/* TIMEOUT */
#define C_TIMEOUT_RETRY_CONNECT (1)
#define C_TIMEOUT_STOP_SECS (10)
#define C_TIMEOUT_CONFIGURE_SECS (30)
#define C_TIMEOUT_HISTORY_SECS (1)
#define C_TIMEOUT_ADC_SECS (1)
#define C_TIMEOUT_PING_SECS (10)
#define C_SECOND_JTAG_REQUEST_SECS (30)//(2*60)

#define C_NB_VOLTAGE_STATUS (8)
#define C_NB_VOLTAGE_RX32 (8)
#define C_NB_VOLTAGE_FPGA (9+8)
#define C_NB_TEMP_FPGA (3)
#define C_NB_SHT_STATUS (2)
#define C_NB_IIC_WAIT_SHT_RX32 (192)
#define C_NB_IIC_WAIT_SHT_DIST (192-3*8)

#define C_NB_CMD_IIC_READ_SHT3X (6+2)
#define C_NB_CMD_IIC_READ_ADS7828_CHANNEL (3)
#define C_NB_CMD_IIC_READ_MCP23008 (2)
#define C_NB_CMD_IIC_READ_SYSMON (4+2)
#define C_NB_PA_STATUS (1)
#define C_DEVICE_ERROR_MAX (10)
#define C_NB_PLOTS_MAX (6)

/* ZDA */
#define TYPE_NAME_ZDA "GPZDA"
#define NB_VIRGULE_ZDA (6)

/* IIC STUFF - SHT3X */
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

/* IIC STUFF - MCP23008 */
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

/* IIC STUFF */
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

/* IIC STUFF */
#define C_SI5341_IIC_ADDR 0x75
#define C_SI5341_PAGE 0x01
#define C_SI5341_STICKY_STATUS 0x11

/* IIC STUFF - SYSMON */
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

enum ET_ANTENNA_CONFIG {
	E_ANTENNA_CONFIG_MAQUETTE24,
	E_ANTENNA_CONFIG_MAQUETTE16,
	E_ANTENNA_CONFIG_ANTENNE1,
	E_ANTENNA_CONFIG_ANTENNE2,
	E_ANTENNA_CONFIG_2XEVAL16,
};

/***********************************************************************
 * Types
 ***********************************************************************/

struct ST_STATUS {
	double af_sht_temperature[C_NB_SHT_STATUS];
	double af_sht_humidity[C_NB_SHT_STATUS];
	std::string astr_sht_name[C_NB_SHT_STATUS];

	double af_fpga_temperature[C_NB_TEMP_FPGA];
	std::string astr_fpga_name[C_NB_TEMP_FPGA];

	double  af_voltages[C_NB_VOLTAGE_STATUS];
	std::string astr_voltages_name[C_NB_VOLTAGE_STATUS];

	double  af_fpga_voltages[C_NB_VOLTAGE_FPGA];
	std::string astr_fpga_voltages_name[C_NB_VOLTAGE_FPGA];

	uint8_t ai_pa_alarm[C_NB_PA_STATUS];
	uint32_t i_error_cnt;
	uint64_t i_time_last_seen;
	std::string str_version_hw;

	ST_STATUS() {
		i_error_cnt = 0;
		i_time_last_seen = f_get_time_ns64();
		for (size_t i = 0; i < C_NB_SHT_STATUS; i++) {
			af_sht_temperature[i] = std::nan("");
			af_sht_humidity[i] = std::nan("");
			astr_sht_name[i] = "";
		}
		for (size_t j = 0; j < C_NB_VOLTAGE_STATUS; j++) {
			af_voltages[j] = std::nan("");
			astr_voltages_name[j] = "";
		}
		for (size_t j = 0; j < C_NB_VOLTAGE_FPGA; j++) {
			af_fpga_voltages[j] = std::nan("");
			astr_fpga_voltages_name[j] = "";
		}
		for (size_t k = 0; k < C_NB_PA_STATUS; k++) {
			ai_pa_alarm[k] = 0;
		}
		str_version_hw = "";

	}
};

class CT_RTAM_EVAL: public CT_CORE {

    /* Unique Id of controlled HW */
	uint64_t _i_uid;
	/* RTAM Id user */
	uint16_t _i_user_id_config_rtam;

    /* Status of RTAM */
	uint32_t _i_status_sample_lost_cnt;
    uint32_t _i_status_sample_missing_cnt;
    uint32_t _i_status_sample_error_cnt;

	/* Hw status */
	std::map<uint32_t,ST_STATUS> _m_status_hw;
	/* Hw GPI */
  	std::map<uint32_t, uint32_t> _m_gpi_hw;

    /* Status of plugin */
	bool _b_status_zda;
	bool _b_status_pps;

    /* Main thread */
	CT_GUARD<CT_HOST_CONTEXT> _pc_context_main;

    /* Mutex state */
	CT_MUTEX _c_rtam_lock;
	CT_MUTEX _c_record_lock;
	CT_MUTEX _c_status_lock;

    /* Antenna mode */
	ET_ANTENNA_CONFIG _e_antenna_config;
	/* Antenna trigger mode */
	bool _b_antenna_passiv_trigger;

	/* Number of tx antenna */
	uint32_t _i_rtam_nb_tx;
	/* Masque d'affinité des systeme en mode RX */
	uint32_t _i_rtam_affinity_rx;
	/* Identifiant des cartes RTAM */
	std::vector<uint8_t> _vi_rtam_boards;
	/* Number od systems */
	uint8_t _i_rtam_nb_system;
	uint32_t _i_rtam_affinity_tx_offset;
	/* Dernier temps de ping rtam */
	uint64_t _i_rtam_time_last_ping_received;
	/* Frequency samplin=g */
	float _f_fs;


    /* Working state - Record */
	ET_RECORD_STATE _e_record_state_current;
	ET_RECORD_STATE _e_record_state_wanted;
	uint64_t _i_record_timeout_start;

    /* Working state - RTAM */
	ET_RTAM_STATE _e_rtam_state_current;
	ET_RTAM_STATE _e_rtam_state_wanted;
	uint64_t _i_rtam_timeout_start;
	uint32_t _i_rtam_ping_count;

	/* Record file current */
    std::string _str_record_file_current;
	/* Record file wanted */
	std::string _str_record_file_wanted;
	/* Record folder */
	std::string _str_record_folder;
	/* BML writer */
	CT_GUARD<bml::node_file_writer<CT_GUARD>> _pc_writer;
	/* Record file split */
	uint32_t _i_record_autosplit_gb;

	std::vector<uint32_t> _vi_boards_provided;


	/* IIC enable */
	bool _b_iic_enable;

	uint64_t _i_time_iic_display;

private:
	/* Basic functions */
	int f_run_off_to_idle(bool timeout);
		int f_rtam_init(void);
		void f_timeout_off_to_init(void);

	int f_run_idle_to_configured(bool timeout);
		int f_rtam_config(void);
		CT_GUARD<CT_PORT_NODE> f_rtam_config_init(void);
		void f_timeout_idle_to_config(void);

	int f_run_receive_n_ping(uint32_t n, bool timeout);
	
	int f_run_update(void);

	int f_run_run_to_idle(bool timeout);
		void f_timeout_run_to_idle(void);

	int f_run_state_to_off(void);

	/* Vérification de la configuration du relecteur */
	int f_record_check_config(std::string const & in_rstr_file);
	/* State Machine */
	int f_record_fsm(void);

	/* Verification de la configuration du RTAM */
	int f_rtam_check_config(void);
	/* State Machine */
	int f_rtam_fsm(void);

	/* Aggegate main context */
	int f_run(CT_HOST_CONTEXT& inout_rc_context);

	void f_rtam_handle_ping(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	int f_rtam_handle_id_ready(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	
	int f_record_start(void);
	int f_record_stop(void);

	int f_rtam_start(void);
	int f_rtam_stop(void);
	int f_rtam_close(void);

	void f_iic_timeout(void);

	CT_GUARD<CT_PORT_NODE> f_iic_get_rx32(void);

	int f_iic_status_update_rx32(CT_GUARD<CT_PORT_NODE> const & in_rpc_node, uint32_t in_i_hw_uid, bool in_b_debug);
	int f_iic_status_update(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);


public:
    CT_RTAM_EVAL();
    virtual ~CT_RTAM_EVAL();

	/* Common plugin function */
    int f_settings(CT_NODE & in_c_node);
    int f_post_init(void);

    /* Port handler function */
    int f_event_rtam(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
};

}
}
}

#endif /* RTAM_EVAL_HH_ */
