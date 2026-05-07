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
 * RTAM_EVAL fusion plugin definition.
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
#include "config.hh"
#include <c/randn.h>
#include "api.hh"
//#include "config_recurrence_plot.hh"

namespace master {
namespace plugins {
namespace rtam_eval {

/***********************************************************************
 * Defines
 ***********************************************************************/
enum ET_PORTS {
	E_PORT_PISTES,
  E_PORT_DETECTIONS,
	E_PORT_AUDIO_OUT,
	E_PORT_RTAM,
	E_PORT_FE,
	E_PORT_BFU_DATA,
	E_PORT_RECORD,
	E_PORT_READER,
	E_PORT_BFU,
	E_PORT_ZDA,
	E_PORT_AUDIO_IN,
	E_PORT_AUDIO_REPLAY,
  E_PORT_CALLBACK,
};

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

	/* Test Id */
	uint32_t _i_test_id;
  uint32_t _i_global_id_test;

	/* rtam id user */
	uint16_t _i_user_id_config_rtam;

	/* Timeout de test */
	uint64_t _i_test_update_time;

	/* Vector of hw device id */
	std::vector<uint32_t> _vi_board;

	/* Noise of each system */
	std::map<uint32_t, std::vector<float>> _m_noise;// id system    id antenne et/ou plot


	/* Status of RTAM */
	float _f_status_sample_lost_acc;
	uint32_t _i_status_sample_lost_cnt;
  uint32_t _i_status_sample_missing_cnt;
  uint32_t _i_status_sample_error_cnt;


	/* Hw status */
	std::map<uint32_t,ST_STATUS> _m_status_hw;
  /* Hw GPI */
  std::map<uint32_t, uint32_t> _m_gpi_hw;
	/* Version of boards */
	std::map<uint32_t,std::string> _m_status_version_hw;
	/* Version of RTAM */
	std:: string _str_status_rtam_version;

	/* Status of plugin */
	bool _b_status_zda;
	bool _b_status_pps;

  std::vector<uint32_t> _vi_boards_provided;

	/* Main thread */
	CT_GUARD<CT_HOST_CONTEXT> _pc_context;

	/* Test thread */
	CT_GUARD<CT_HOST_CONTEXT> _pc_context_test;

	/* Configuration recherché */
	CT_GUARD<CT_PORT_NODE> _pc_config_wanted;

	/* Mutex de l'état */
	CT_MUTEX _c_rtam_lock;
	CT_MUTEX _c_record_lock;

	CT_MUTEX _c_status_lock;
	CT_MUTEX _c_config_lock;

	/* Mode d'antenne */
	ET_ANTENNA_CONFIG _e_antenna_config;

	/* Mode de trigger d'antenne */
	bool _b_antenna_passiv_trigger;

	/* Etat de fonctionnement - RTAM */
	ET_RTAM_STATE _e_rtam_state_current;
	ET_RTAM_STATE _e_rtam_state_wanted;
	uint64_t _i_rtam_timeout_start;
	uint32_t _i_rtam_ping_count;

	/* Compteur d'erreur */
	uint32_t _i_rtam_error_cnt;
	/* Configuration RTAM recherché */
	CT_GUARD<CT_RTAM_EVAL_CONFIG> _pc_rtam_config_wanted;
	/* Configuration RTAM courante */
	CT_GUARD<CT_RTAM_EVAL_CONFIG> _pc_rtam_config_current;
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



  /* BFU time needed */
  uint64_t _i_rtam_first_time;
  uint64_t _i_rtam_first_hw_time;
  uint64_t _i_rtam_ping_time;
  std::list<std::pair<uint64_t,uint64_t>> _ls_time_ping;


	/* Etat de fonctionnement - Record */
	ET_RECORD_STATE _e_record_state_current;
	ET_RECORD_STATE _e_record_state_wanted;
	uint64_t _i_record_timeout_start;
	/* Compteur d'erreur - Record */
	uint32_t _i_record_error_cnt;
	/* Fichier d'enregistrement courant */
	std::string _str_record_file_current;
	/* Fichier d'enregistrement voulu */
	std::string _str_record_file_wanted;
	/* Repertoire d'enregistrement */
	std::string _str_record_folder;
	/* BML writer */
	CT_GUARD<bml::node_file_writer<CT_GUARD> >  _pc_writer;
	/* Decoupe des fichiers enregistrement */
	uint32_t _i_record_autosplit_gb;

	/* Celerité */
	std::string _str_celerity;
	uint64_t _i_last_time;
	uint64_t _i_first_time;

private:

	/* IIC - */
	CT_GUARD<CT_PORT_NODE> f_iic_get_rx32(void);
	void f_iic_timeout(void);
	void f_iic_display_status(void);
	void f_iic_update_node_status(ST_STATUS & inout_rs_status, CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	int f_iic_status_update_rx32(
			CT_GUARD<CT_PORT_NODE> const & in_rpc_node,
			uint32_t in_i_hw_uid,
			bool in_b_debug);
	int f_iic_status_update(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);

	/* decodage trame ZDA */
	/* RTAM function */
	void f_rtam_clear_status(void);
	/* Mise à jour des versions */
	int f_rtam_version_update(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	/* Initilisation des plugins RTAM */
	int f_rtam_init(void);
	/* Démarrage des tirs */
	int f_rtam_start(void);
	/* Arret du plugin RTAM */
	int f_rtam_stop(void);
	/* Fermeture du plugin RTAM */
	int f_rtam_close(void);
	/* Configuration du plugin RTAM */
	int f_rtam_config(void);
	/* Configuration initiale du plugin RTAM */
	CT_GUARD<CT_PORT_NODE> f_rtam_config_init(void);
	/* Mise à jour de la configuration */
	int f_rtam_update(void);
	/* Analyse d'un paquet ping */
	void f_rtam_handle_ping(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	/* Analyse d'un paquet ready */
	int f_rtam_handle_id_ready(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	/* Déclanchement d'un paquet de debug */
	int f_rtam_jtag_trigger(void);
	/* Verification de la configuration du RTAM */
	int f_rtam_check_config(void);
	/* Affichage de l'état */
	const char * f_rtam_print_state(enum ET_RTAM_STATE const in_e_state);
	/* Simulation d'une mise à jour de configuration */
	void f_rtam_update_test(uint32_t in_i_test_id);

	/* Démarrage de l'enregistrement */
	int f_record_start(void);
	/* Arret de l'enregistrement */
	int f_record_stop(void);
	/* Vérification de la configuration du relecteur */
	int f_record_check_config(std::string const & in_rstr_file);
	/* Affichage de l'état */
	const char * f_record_print_state(enum ET_RECORD_STATE const in_e_state);

	/* State functions */
	int f_status_send(void);
	void f_state_send(void);
	/* Status functions */
	void f_status_reset(void);

	/* Compute PBIT */
	int f_compute_noise_variance(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);

	/* Aggegate main context */
	int f_run(CT_HOST_CONTEXT& inout_rc_context);

	/* Aggegate test context */
	int f_run_tests(CT_HOST_CONTEXT& inout_rc_context);

	/* State Machine */
	int f_rtam_fsm(void);
	int f_record_fsm(void);

private:
	/* Tests functions */
	void f_test_autorun_antenna(void);
	void f_test_autorun_off(void);
public:

	/* Common plugin function */
	int f_settings(CT_NODE & inout_rc_node);
	int f_post_init(void);
	int f_post_profile(void);
	int f_pre_profile(void);


	/* Port handler function */
	int f_event_rtam(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);
	int f_event_fe(CT_GUARD<CT_PORT_NODE> const & in_rpc_node);

	CT_RTAM_EVAL();
	virtual ~CT_RTAM_EVAL();
};

}
}
}

#endif /* RTAM_EVAL_HH_ */
