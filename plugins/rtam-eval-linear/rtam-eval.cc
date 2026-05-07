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
 * Plugin rtam-eval.
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
#include <rtam/api.hh>
#include "api.hh"

using namespace std;
using namespace master::plugins::rtam_eval;
namespace rtam = master::plugins::fec;


/* Constructor.
 */
extern "C" {
extern char **environ;
}

CT_RTAM_EVAL::CT_RTAM_EVAL(void) :
   	 CT_CORE() {
    D("Creating RTAM_EVAL CORE [%p]", this);

	/* Print env variables */
	{
		int i_index = 1;
		char *pc_s = *environ;

		for (; pc_s; i_index++) {
			string str_tmp(pc_s);

			if(str_tmp.find("TEST_") != string::npos) {
				_WARN << "TEST VARIABLES: " << str_tmp;
			}
			if(str_tmp.find("DEBUG_") != string::npos) {
				_WARN << "DEBUG VARIABLES: " << str_tmp;
			}

			/* Get env value */
			pc_s = *(environ+i_index);
		}
	}

	cout << "-----------------------------------------------------------------------------------------------" << endl;
	cout << "ANTENNA=[antenne, antenne2,      : Modèle de l'antenne\nmaquette16, maquette24, 2Xeval16]\n" << endl;
	cout << "TEST_RTAM_EVAL=[Num]             : Numéro de test à executer" << endl;
	cout << "TEST_TRIGGER_DURATION=[0-5000]   : Durée du trigger (ms)" << endl;
	cout << "TEST_PULSE_LENGTH=[0-200]        : Durée du pulse (ms)" << endl;
	cout << "TEST_PULSE_FSTART=[10000-300000] : Fréquence de démarrage du pulse (Hz)" << endl;
	cout << "TEST_PULSE_FEND=[10000-300000]   : Fréquence de démarrage du pulse (Hz)" << endl;
	cout << "TEST_PULSE_AMPLITUDE=[0-100]     : Amplitude du pulse (%%)" << endl;
	cout << "TEST_ADC=[0-7]                   : Numérisation d'une voie non démodulée par ADC" << endl;
	cout << "TEST_TONE=[0/1]                  : Numérisation d'un test tone analogique" << endl;
	cout << "TEST_CHANNEL_ID=[0/1]            : Numérisation d'un test d'identification des voies ADC HW" << endl;
	cout << "TEST_ADC_ID=[0/1]                : Numérisation d'un test d'identification des voies ADC SW" << endl;
	cout << "TEST_TERM_Z1=[0/1]               : Choix du mode de terminaison des ADC Z1" << endl;
	cout << "TEST_TERM_Z2=[0/1]               : Choix du mode de terminaison des ADC Z2" << endl;
	cout << "TEST_TERM_Z1_Z2=[0/1]            : Choix du mode de terminaison des ADC Z1//Z2" << endl;
	cout << "TEST_TERM_OPEN=[0/1]             : Choix du mode de terminaison des ADC Ouvert" << endl;
	cout << "TEST_FULL_SCALE=[1/1000]         : Choix de la pleine échelle de numérisation (mV)" << endl;
	cout << "TEST_NO_TX=[0/1]                 : Desactive l'émission" << endl;
	cout << "TEST_NO_IIC=[0/1]                : Desactive la lecture IIC" << endl;
	cout << "DEBUG_ON_RUNNING=[0/1]           : Active la lecture du lien de debug pendant l'acquisition" << endl;
	cout << "DEBUG_ON_ERROR=[0/1]             : Active la lecture du lien de debug en cas d'erreur" << endl;
  cout << "ENABLE_IIC_STATUS=[1]            : Active la lecture du lien de debug en cas d'erreur" << endl;
	cout << "-----------------------------------------------------------------------------------------------" << endl;

  /* Init random seed */
  srand (time(NULL));

  /* Create data port */
  f_port_create(E_PORT_RTAM, "rtam", M_PORT_BIND(CT_RTAM_EVAL, f_event_rtam, this));

  /* Get unique id of device */
  if (getenv("PROC_MANAGER_ID")!= NULL)
		_i_uid = atoll(getenv("PROC_MANAGER_ID"));
	else
		_i_uid = 0;
    
    /* Init id user rtam config */
	_i_user_id_config_rtam = 0;
    
	/* Init status of plugin */
  _b_status_zda = false;
  _b_status_pps = false;

  /* Rapport of error RTAM */
  _i_status_sample_missing_cnt = 0;
  _i_status_sample_lost_cnt = 0;
  _i_status_sample_error_cnt = 0;
	
    /* Default state wanted for audio*/
	_e_record_state_current = E_RECORD_STATE_IDLE;
	_e_record_state_wanted = E_RECORD_STATE_IDLE;
	_e_rtam_state_current = E_RTAM_STATE_OFF;
    _e_rtam_state_wanted = E_RTAM_STATE_OFF;

    /* Select antenna mode */
	_e_antenna_config = E_ANTENNA_CONFIG_MAQUETTE16;
	_b_antenna_passiv_trigger = false;

    if(getenv("ANTENNA")!= NULL) {
		auto str_antenna = string(getenv("ANTENNA"));
		if(str_antenna == "antenne") {
			_e_antenna_config = E_ANTENNA_CONFIG_ANTENNE1;
			_b_antenna_passiv_trigger = true;
			_WARN << "MODE: SINGLE ANTENNA";
		} else if (str_antenna == "antenne2") {
			_e_antenna_config = E_ANTENNA_CONFIG_ANTENNE2;
			_b_antenna_passiv_trigger = true;
			_WARN << "MODE: SINGLE ANTENNA";
		} else if (str_antenna == "maquette16") {
			_e_antenna_config = E_ANTENNA_CONFIG_MAQUETTE16;
			_WARN << "MODE: 16 bits Maquette";
		} else if (str_antenna == "maquette24") {
			_e_antenna_config = E_ANTENNA_CONFIG_MAQUETTE24;
			_WARN << "MODE: 24 bits Maquette";
		} else if (str_antenna == "2Xeval16") {
			_e_antenna_config = E_ANTENNA_CONFIG_2XEVAL16;
			_WARN << "MODE: 16 bits 2 Cartes evaluation en série";
		} else {
			_FATAL << "Unsupported antenna: "<< str_antenna;
		}
	}

	/* Init record */
    _str_record_folder = "/tmp";
	_i_record_autosplit_gb = 1;
	if(getenv("TEST_FILENAME")!= NULL) {
		_str_record_file_current = getenv("TEST_FILENAME");
	} else {
		_str_record_file_current = "autorun";
	}
	_pc_writer = NULL;

	/* Context */
	_pc_context_main = CT_GUARD<CT_HOST_CONTEXT>(CT_HOST::host->f_new_context());
	M_ASSERT(_pc_context_main);
  D("RTAM_EVAL CORE created");
}

CT_RTAM_EVAL::~CT_RTAM_EVAL() {
}

int CT_RTAM_EVAL::f_settings(CT_NODE & in_c_node) {
	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_post_init(void) {
	WARN("Starting RTAM-EVAL");

	/* AUTO TEST TRIGGER */
	M_ASSERT(_pc_context_main);
	/** Start context */
	if (_pc_context_main->f_start(M_CONTEXT_BIND(CT_RTAM_EVAL, f_run, this)) != EC_SUCCESS) {
		CRIT("Unable to start context");
		return EC_FAILURE;
	}
	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_run(CT_HOST_CONTEXT & inout_rc_context) {

  f_run_off_to_idle(true);

  f_run_idle_to_configured(true);

  f_run_receive_n_ping(1, true);

  // f_run_update();

  // f_run_receive_n_ping(1, true);

  f_run_run_to_idle(true);

  f_run_state_to_off();

  M_ASSERT(0);

	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_run_off_to_idle(bool timeout) {
	{
		CT_GUARD_LOCK c_guard(_c_record_lock);
    _e_record_state_wanted = E_RECORD_STATE_RUNNING;
    _e_record_state_current = E_RECORD_STATE_RUNNING;
    f_record_start();
	}

	{
		CT_GUARD_LOCK c_guard(_c_rtam_lock);
		_e_rtam_state_wanted = E_RTAM_STATE_IDLE;
		_vi_boards_provided.clear();
		f_rtam_init();
		_e_rtam_state_current = E_RTAM_STATE_INITIALIZING;
		_i_rtam_timeout_start = f_get_time_ns64();
	}

  do {
    if (timeout)
      f_timeout_off_to_init();
    usleep(100e3);
  } while (_e_rtam_state_current == E_RTAM_STATE_INITIALIZING);

	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_init() {
	//0x81 // BJ
	//0x82 // Atterissement
	//0xa0 // MODULE sfp
	//0X40 // MODULE FERM

  switch(_e_antenna_config) {
		case E_ANTENNA_CONFIG_MAQUETTE24:
			_f_fs = 125e6/4; // 31.25 MHz
			break;
		default:
			_f_fs = 25e6;
			break;
	}

	switch(_e_antenna_config) {
		case E_ANTENNA_CONFIG_MAQUETTE16:
		case E_ANTENNA_CONFIG_MAQUETTE24:
			_i_rtam_nb_tx = 0;
			_i_rtam_nb_system = 1;
			_i_rtam_affinity_tx_offset = 0;
			_vi_rtam_boards.resize(1);//_i_rtam_nb_tx);
			M_ASSERT(_vi_rtam_boards.size() == 1);
			_vi_rtam_boards[0]=0x82;
			break;
		case E_ANTENNA_CONFIG_2XEVAL16:
			_i_rtam_nb_tx = 2;
			_i_rtam_nb_system = 2;
			_i_rtam_affinity_tx_offset = 0;
			_vi_rtam_boards.resize(2);//_i_rtam_nb_tx);
			M_ASSERT(_vi_rtam_boards.size() == 2);
			_vi_rtam_boards[0]=0xA1;
			_vi_rtam_boards[1]=0xA2;
			break;
		case E_ANTENNA_CONFIG_ANTENNE1:
			_i_rtam_nb_tx = 0;
			_i_rtam_nb_system = 4;
			_i_rtam_affinity_tx_offset = 0;
			_vi_rtam_boards.resize(4);//_i_rtam_nb_tx);
			M_ASSERT(_vi_rtam_boards.size() == 4);
			_vi_rtam_boards[0]=0xa8;
			_vi_rtam_boards[1]=0x69;
			_vi_rtam_boards[2]=0x6a;
			_vi_rtam_boards[3]=0x2b;
			break;
		case E_ANTENNA_CONFIG_ANTENNE2:
			_i_rtam_nb_tx = 0;
			_i_rtam_nb_system = 4;
			_i_rtam_affinity_tx_offset = 0;
			_vi_rtam_boards.resize(4);//_i_rtam_nb_tx);
			M_ASSERT(_vi_rtam_boards.size() == 4);
			_vi_rtam_boards[0]=0xac;
			_vi_rtam_boards[1]=0x6d;
			_vi_rtam_boards[2]=0x6e;
			_vi_rtam_boards[3]=0x2f;
			break;
		default:
			break;
	}

  CT_PORT_NODE_GUARD pc_node(rtam::E_ID_INIT);

  switch(_e_antenna_config) {
    case E_ANTENNA_CONFIG_MAQUETTE16:
    case E_ANTENNA_CONFIG_MAQUETTE24: {
      uint8_t i_id_board_pwm = _vi_rtam_boards[0];

      {
        for(uint8_t i_tx=0; i_tx<_i_rtam_nb_tx; i_tx++) {
          CT_GUARD<CT_PORT_NODE> pc_syst = pc_node->add(rtam::E_ID_INIT_SYST);
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_SYSTEM);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
          }
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_PULSER);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
            pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(0);
          }
        }
      }
      {
        CT_GUARD<CT_PORT_NODE> pc_syst = pc_node->add(rtam::E_ID_INIT_SYST);
        {
        auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
        pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_SYSTEM);
        pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
        pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
        }
        {
        auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
        pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
        pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
        pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
        pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
        }
        {
        auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
        pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
        pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
        pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(1);
        pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
        }
        {
        auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
        pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
        pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
        pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(2);
        pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
        }
        {
        auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
        pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
        pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
        pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(3);
        pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
        }
      }
      break;
    }
    case E_ANTENNA_CONFIG_2XEVAL16:
    case E_ANTENNA_CONFIG_ANTENNE1:
    case E_ANTENNA_CONFIG_ANTENNE2: {
      {
        for(uint8_t i_id_board=0; i_id_board<_i_rtam_nb_system; i_id_board++) {
          CT_GUARD<CT_PORT_NODE> pc_syst = pc_node->add(rtam::E_ID_INIT_SYST);
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_SYSTEM);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
          }
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_PULSER);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
            pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(0);
          }
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
            pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
          }
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(1);
            pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
          }
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(2);
            pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
          }
          {
            auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
            pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_ADC);
            pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
            pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(3);
            pc_subsyst->add(rtam::E_ID_HW_CHANNEL_NUMBER)->set_data<uint32_t>(8);
          }
        }
      }
      break;
    }
    default:
      break;
  }

	_WARN << "Initializing RTAM";
	f_port_get(E_PORT_RTAM).f_send(pc_node);
	return EC_SUCCESS;
}

void CT_RTAM_EVAL::f_timeout_off_to_init(void) {
	if (M_ABS(f_get_time_ns64() - _i_rtam_timeout_start) > C_TIMEOUT_CONFIGURE_SECS*1e9) {
    if (_b_antenna_passiv_trigger && (_e_rtam_state_current == E_RTAM_STATE_CONFIGURED)) {
      /* Nothing to do */
    } else {
      _CRIT << "ERROR DETECTED MOVED TO OFF STATE, NO RUNNING";
      M_ASSERT(0);
      _e_rtam_state_current = E_RTAM_STATE_ERROR;
    }
	}
}

int CT_RTAM_EVAL::f_run_idle_to_configured(bool timeout) {
  {
    CT_GUARD_LOCK c_guard(_c_rtam_lock);
    f_rtam_config();
    _e_rtam_state_current = E_RTAM_STATE_CONFIGURING;
    _i_rtam_timeout_start = f_get_time_ns64();
  }

  do {
    if (timeout)
      f_timeout_idle_to_config();
    usleep(100e3);
  } while (_e_rtam_state_current == E_RTAM_STATE_CONFIGURING
      || _e_rtam_state_current == E_RTAM_STATE_CONFIGURED);
    
	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_config(void) {

  /* Initialisation de la configuration */
  auto pc_config = f_rtam_config_init();

  {
    auto pc_node = pc_config->get(rtam::E_ID_SCENARIO);
    switch(_e_antenna_config) {
      case E_ANTENNA_CONFIG_2XEVAL16: {
		    pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xFF);//(0xA1);
		    _CRIT << "SET MASTER";
		    break;
      }
      case E_ANTENNA_CONFIG_MAQUETTE24:
      case E_ANTENNA_CONFIG_MAQUETTE16: {
        pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0x82);//(0xFF);
        _CRIT << "SET MASTER";
        break;
      }
      case E_ANTENNA_CONFIG_ANTENNE1:
      case E_ANTENNA_CONFIG_ANTENNE2: {
        pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xFF);
        _CRIT << "SET MASTER";
        break;
      }
      default: {
        M_BUG();
        break;
      }
    }
    _CRIT << "SET MASTER: " <<std::hex << pc_node->get(rtam::E_ID_TRIGGER_MASTER)->get_data<uint64_t>() ;
  }


  /* Transmission du noeud au plugin RTAM */
  f_port_get(E_PORT_RTAM).f_send(pc_config);

  return EC_SUCCESS;
}

CT_GUARD<CT_PORT_NODE> CT_RTAM_EVAL::f_rtam_config_init(void) {

  CT_PORT_NODE_GUARD pc_config(rtam::E_ID_CONFIG);

  /****************************** SCENARIO GLOBAL *****************************/
  pc_config->add(rtam::E_ID_NB_SCENARIO)->set_data<uint8_t>(0x1);


  /****************************** SCENARIO ************************************/
  auto pc_node = pc_config->add(rtam::E_ID_SCENARIO);

  /****************************** TRIGGER ******************************/
  /* Choose internal triggering mode */
  pc_node->add(rtam::E_ID_TRIGGER_MODE)->set_data<uint8_t>(rtam::E_TRIGGER_ACQUISITION_START);

  /* Add Master board id */
  pc_node->add(rtam::E_ID_TRIGGER_MASTER);

  // Bug probably due to 40Mhz and trigger detector
  pc_node->add(rtam::E_ID_TRIGGER_LATCH_MODE)->set_data<uint8_t>(rtam::E_TRIGGER_LATCH_MODE_AC);
  //pc_node->add(rtam::E_ID_TRIGGER_LATCH_MODE)->set_data<uint8_t>(rtam::E_TRIGGER_LATCH_MODE_NONE);

  pc_node->add(rtam::E_ID_USER_CONFIG)->set_data<uint16_t>(_i_user_id_config_rtam++);

  /* Set trigger duration */
  double f_duration = 750.0/1500.0;
  if((getenv("TEST_TRIGGER_DURATION")!=NULL) && (atoi(getenv("TEST_TRIGGER_DURATION"))<5000)) {
    uint32_t i_trigger_duration = atoi(getenv("TEST_TRIGGER_DURATION"));
    f_duration = double(i_trigger_duration)/1000.0;
  }
  pc_node->add(rtam::E_ID_TRIGGER_DURATION)->set_data<double>(f_duration*1.2);

  /* Set number of trigger */
  pc_node->add(rtam::E_ID_TRIGGER_MAX)->set_data<uint32_t>(0);
  if(getenv("TEST_TRIGGER_MAX")!=NULL) {
    uint32_t i_trigger_max = atoi(getenv("TEST_TRIGGER_MAX"));
    pc_node->add(rtam::E_ID_TRIGGER_MAX)->set_data<uint32_t>(i_trigger_max + 1);
    _WARN << " Acquisition of " << i_trigger_max << " triggers then STOP";
  }

  /* Set shift of SFP trigger */
  pc_node->add(rtam::E_ID_TRIGGER_SHIFT)->set_data<double>(0);// 0.1

  /****************************** POW SYNC  ***************************/
  std::vector<float> vf_pow_sync_freq = {_f_fs/32, _f_fs/32, _f_fs/32};
  std::vector<float> vf_pow_sync_duty = {0.5, 0.5, 0.5};
  pc_node->add(rtam::E_ID_TRIGGER_POW_SYNC_FREQ)->set_data(vf_pow_sync_freq);
  pc_node->add(rtam::E_ID_TRIGGER_POW_SYNC_DUTY)->set_data(vf_pow_sync_duty);

    /****************************** ADC ******************************/
  /* Select ADC after decimation */
  if(getenv("TEST_ADC")!=NULL) {
    pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_ADC_MUX_INPUT);
  } else if (getenv("TEST_CHANNEL_ID")!=NULL) {
    pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_ADC_MUX_DEMOD);
  } else if (getenv("TEST_MUX_DEMOD")!=NULL) {
    pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_ADC_MUX_DEMOD);
  } else if (getenv("TEST_MUX_FIR")!=NULL) {
    pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_MUX_FIR);
  } else {
    switch(_e_antenna_config) {
      case E_ANTENNA_CONFIG_MAQUETTE24:
        pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_ADC_MUX_INPUT);
        break;
      default:
        pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_MUX_DECIM);
        break;
    }
  }

  /* Test mode */
  if(getenv("TEST_TONE") != NULL) {
    pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_ANALOG_TONE);
  } else if(getenv("TEST_CHANNEL_ID") != NULL) {
    pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_CHANNEL_ID);
  } else if(getenv("TEST_ADC_ID") != NULL) {
    pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_ADC_ID);
  } else if(getenv("TEST_MODE_SINE") != NULL) {
    pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_SINE);
  } else if(getenv("TEST_MUX_FIR") != NULL) {
    _DBG << "TEST_MUX_FIR";
    if(getenv("TEST_NO_GAIN") != NULL)
      pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_DECIMATOR_NO_ACCUMULATOR_GAIN);
  } else if(getenv("TEST_MUX_DEMOD") != NULL) {
    _DBG << "TEST_MUX_DEMOD";
    if(getenv("TEST_NO_GAIN") != NULL)
      pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_DECIMATOR_NO_ACCUMULATOR_GAIN);
  } else {
    pc_node->add(rtam::E_ID_TEST_MODE)->set_data<uint8_t>(rtam::E_ADC_TEST_MODE_DECIMATOR_NO_ACCUMULATOR_GAIN);
  }

  /* Set Sampling frequency to 20Mhz */
  pc_node->add(rtam::E_ID_FREQ_SAMPLING)->set_data<double>(_f_fs);

  /* Set downsampling factor */
  uint16_t i_decim;
  switch(_e_antenna_config) {
    case E_ANTENNA_CONFIG_MAQUETTE24: {
      i_decim = 4 * 32; //MCLK_DIV(FAST MODE) / DECIM (32, 64, 128, 256, 512, 1024)
      break;
    }
    default: {
      if (getenv("TEST_ADC")!=NULL) {
        i_decim = 8;
      } else if (getenv("TEST_CHANNEL_ID")!=NULL) {
        i_decim = 64;
      } else if (getenv("TEST_MUX_DEMOD")!=NULL) {
        i_decim = 64;
      } else {
        i_decim = 64*5;
      }
      break;
    }
  }

  if(getenv("TEST_DECIM")!=NULL) {
    i_decim = atoi(getenv("TEST_DECIM"));
    if ( getenv("TEST_ADC")!=NULL || getenv("TEST_CHANNEL_ID")!=NULL || getenv("TEST_MUX_DEMOD")!=NULL) {
      if (i_decim > 64) {
        _CRIT << " Decimation factor (" << i_decim << ") is higher than possible. Reduced to 64. ";
        i_decim = 64;
      }
    }
  }

  pc_node->add(rtam::E_ID_DECIMATE)->set_data<uint16_t>(i_decim);

  /* Set digital gain */
  uint16_t i_digital_gain=0;
  if(getenv("TEST_DIGITAL_GAIN")!=NULL) {
    i_digital_gain = atoi(getenv("TEST_DIGITAL_GAIN"));
  }

  pc_node->add(rtam::E_ID_DIGITAL_GAIN)->set_data<uint16_t>(i_digital_gain);

  /* Set demodulation frequency */
  if(getenv("TEST_TONE") != NULL) {
    {
      auto pc_demod = pc_node->add(rtam::E_ID_FREQ_DEMOD);
      pc_demod->set_data<double>(790000);
    }
  } else {
    {
      auto pc_demod = pc_node->add(rtam::E_ID_FREQ_DEMOD);
      if(getenv("TEST_DEMOD_FREQUENCY")!=NULL) {
        pc_demod->set_data<double>(atof(getenv("TEST_DEMOD_FREQUENCY")));
      } else {
        pc_demod->set_data<double>(73242.1875);
        //pc_demod->set_data<double>(329589.8438);
        //pc_demod->set_data<double>(250244.1406);
      }
    }
  }

  /* Set cutoff frequency */
  {
    float f_freq_cuttoff;
    if (pc_node->get(rtam::E_ID_FREQ_DEMOD)->get_data<double>()==73242.1875) {
      f_freq_cuttoff = _f_fs/float(i_decim)/6;
    } else {
      f_freq_cuttoff = _f_fs/float(i_decim)*0.4;
    }
    if(getenv("TEST_CUTOFF_FREQUENCY")!=NULL) {
      f_freq_cuttoff = atof(getenv("TEST_CUTOFF_FREQUENCY"));
    }
    _DBG << _V(f_freq_cuttoff);
    pc_node->add(rtam::E_ID_FREQ_CUTOFF)->set_data<double>(f_freq_cuttoff);
  }

  if(getenv("TEST_TERM_Z1") != NULL) {
    pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_Z1);
  } else if (getenv("TEST_TERM_Z2") != NULL) {
    pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_Z2);
  } else if (getenv("TEST_TERM_OPEN") != NULL) {
    pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_OPEN);
  } else if(getenv("TEST_TERM_Z1_Z2") != NULL) {
    pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_Z1_Z2);
  } else {
    pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_OPEN);
  }

  /* Set Sampling samples */
  uint32_t i_nb_sample = f_duration*_f_fs/float(i_decim);
  if (i_nb_sample >= pow(2,19)) { // NB_SAMPLE is limited to 2**19-1 into FPGA
    _CRIT << "Nb samples was " << i_nb_sample << " and have been limited to 524287 (2**19-1) !";
    i_nb_sample = pow(2,19)-1;
  }
  pc_node->add(rtam::E_ID_NB_SAMPLE)->set_data<uint32_t>(round(i_nb_sample/124)*124);
  _CRIT << " Fs: " << _f_fs << " Duration: " << f_duration << " Nb samples: " << i_nb_sample << " Decim: " << i_decim << " Freq cutoff: " << pc_node->get(rtam::E_ID_FREQ_CUTOFF)->get_data<double>();

  /* Set fullscale of ADC */
  if (getenv("TEST_FULL_SCALE") != NULL) {
    pc_node->add(rtam::E_ID_FULL_SCALE)->set_data<double>(double(atoi(getenv("TEST_FULL_SCALE")))/1000.0);
  } else {
  switch(_e_antenna_config) {
    case E_ANTENNA_CONFIG_MAQUETTE24: {
      pc_node->add(rtam::E_ID_FULL_SCALE)->set_data<double>(4.096*2);
      break;
    }
    default: {
      pc_node->add(rtam::E_ID_FULL_SCALE)->set_data<double>(10e-3);
      break;
    }
  }
  }

  /* Post trigger after data acquisition */
  pc_node->add(rtam::E_ID_POST_TRIGGER)->set_data<uint32_t>(0);

  /****************************** PULSER ******************************/

  if (getenv("TEST_NO_TX") == NULL) {
    uint32_t i_pulse_length = 100;
    uint32_t i_freq_start = 60e3;
    uint32_t i_freq_end = 80e3;
    uint32_t i_amplitude = 70;

    if((getenv("TEST_PULSE_LENGTH")!=NULL) && (atoi(getenv("TEST_PULSE_LENGTH")))) {
      uint32_t i_tmp = atoi(getenv("TEST_PULSE_LENGTH"));
      if(i_tmp <= 200) {
        i_pulse_length = i_tmp;
      }
    }

    if((getenv("TEST_PULSE_FSTART")!=NULL) && (atoi(getenv("TEST_PULSE_FSTART")))) {
      uint32_t i_tmp = atoi(getenv("TEST_PULSE_FSTART"));
      if((i_tmp <= 300e3) && (i_tmp >= 10e3)) {
        i_freq_start = i_tmp;
      }
    }

    if((getenv("TEST_PULSE_FEND")!=NULL) && (atoi(getenv("TEST_PULSE_FEND")))) {
      uint32_t i_tmp = atoi(getenv("TEST_PULSE_FEND"));
      if((i_tmp <= 300e3) && (i_tmp >= 10e3)) {
        i_freq_end = i_tmp;
      }
    }

    if((getenv("TEST_PULSE_AMPLITUDE")!=NULL) && (atoi(getenv("TEST_PULSE_AMPLITUDE")))) {
      uint32_t i_tmp = atoi(getenv("TEST_PULSE_AMPLITUDE"));
      if((i_tmp <= 100) && (i_tmp >= 1)) {
        i_amplitude = i_tmp;
      }
    }
    //_WARN <<_GREEN <<_V(i_amplitude);
    /* Antenna Pulse */
    switch(_e_antenna_config)
    {
      // FIXME NEED LOOP, BUG DUAL CONFIGURATION SYSTEM RX AND TX FOR PULSE
      default: {
        for(uint8_t i_tx = 0; i_tx < _i_rtam_nb_tx; i_tx++)
        {
          auto pc_pulse = pc_node->add(rtam::E_ID_PULSE);
          pc_pulse->add(rtam::E_ID_AFFINITY_SYSTEM)->set_data<uint64_t>(1 << (i_tx+_i_rtam_affinity_tx_offset));
          pc_pulse->add(rtam::E_ID_PULSER_MODE)->set_data<uint8_t>(rtam::E_PULSE_HFM);
          if (_i_user_id_config_rtam % 2) {
            pc_pulse->add(rtam::E_ID_PULSER_FSTART)->set_data<float>(i_freq_start);
            pc_pulse->add(rtam::E_ID_PULSER_FEND)->set_data<float>(i_freq_end);
          } else {
            pc_pulse->add(rtam::E_ID_PULSER_FEND)->set_data<float>(i_freq_end);
            pc_pulse->add(rtam::E_ID_PULSER_FSTART)->set_data<float>(i_freq_start);
          }
          //pc_pulse->add(rtam::E_ID_PULSER_TIME_WEIGHTING_ID)->set_data<uint8_t>(0);
          //pc_pulse->add(rtam::E_ID_PULSER_TIME_WEIGHTING_STEP)->set_data<float>(float(((float)i_pulse_length/1000.0)/64.0));
          pc_pulse->add(rtam::E_ID_PULSER_DURATION)->set_data<double>(double(i_pulse_length)/1000.0);
          pc_pulse->add(rtam::E_ID_PULSER_AMPLITUDE)->set_data<double>(double(i_amplitude)/100.0);
        }
        break;
      }
    }
  }

  if(getenv("TEST_ADC")!=NULL) {
    pc_node->add(rtam::E_ID_CHANNELS)->set_data<uint8_t>(rtam::E_CHANNELS_WHITE_LIST);
    uint32_t i_capt = atoi(getenv("TEST_ADC"));
    uint32_t i_step = 8;
    for(uint32_t i_beam= i_capt; i_beam < 64; i_beam += i_step) {
      pc_node->get(rtam::E_ID_CHANNELS)->add(rtam::E_ID_CHANNEL)->set_data<uint16_t>(i_beam);
    }
  }

  /* Set vga values */
  {
    auto pc_dac = pc_node->add(rtam::E_ID_DAC);

    uint32_t i_step = 3000;
    if(getenv("TEST_VGA_STEP") != NULL) {
      i_step = atoi(getenv("TEST_VGA_STEP"));
    }

    pc_dac->add(rtam::E_ID_DAC_STEP)->set_data<uint16_t>(i_step);

    auto pc_vga1 = pc_dac->add(rtam::E_ID_DAC_VGA1);
    uint16_t i_sz_vga = 256;
    pc_vga1->resize(i_sz_vga*sizeof(uint16_t));
    uint16_t * pi_tmp = pc_vga1->mmap<uint16_t>();

    for(uint32_t i_dac=0; i_dac<i_sz_vga; i_dac++) {
      pi_tmp[i_dac] = 0;
    }

    if(getenv("TEST_VGA_RAMP")!=NULL) {
      for(uint32_t i_dac=0; i_dac<i_sz_vga; i_dac++) {
        pi_tmp[i_dac] = ceil(double(UINT16_MAX)*double(i_sz_vga-i_dac)/double(i_sz_vga)*2.5/3.3);
      }
    }

    if(getenv("TEST_VGA_VALUE") != NULL) {
      uint32_t i_vga = atoi(getenv("TEST_VGA_VALUE"));
      for(uint32_t i_dac=0; i_dac<i_sz_vga; i_dac++) {
        pi_tmp[i_dac] = i_vga;
      }
    }
  }

  return pc_config;
}

void CT_RTAM_EVAL::f_timeout_idle_to_config(void) {
	if (M_ABS(f_get_time_ns64() - _i_rtam_timeout_start) > C_TIMEOUT_CONFIGURE_SECS*1e9) {
    if (_b_antenna_passiv_trigger && (_e_rtam_state_current == E_RTAM_STATE_CONFIGURED)) {
      /* Nothing to do */
    } else {
      _CRIT << "ERROR DETECTED MOVED TO OFF STATE, NO RUNNING";
      M_ASSERT(0);
      _e_rtam_state_current = E_RTAM_STATE_ERROR;
    }
	}
}

int CT_RTAM_EVAL::f_run_receive_n_ping(uint32_t n, bool timeout) {
  while (n == 0 || _i_rtam_ping_count < n) {
    {
      CT_GUARD_LOCK c_guard(_c_rtam_lock);
      if (timeout
      && !_b_antenna_passiv_trigger) {
        if (getenv("RTAM_BFU_DEBUG") == NULL) {
          if (M_ABS(f_get_time_ns64()-_i_rtam_time_last_ping_received) > C_TIMEOUT_PING_SECS*1e9) {
            _WARN << "TIMEOUT DETECTED ...";
            M_ASSERT(0);
          }
        }
      }
    }
    usleep(100e3);
  }

  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_run_update(void) {
  {
    CT_GUARD_LOCK c_guard(_c_rtam_lock);
    f_rtam_config();
    _e_rtam_state_current = E_RTAM_STATE_RUNNING;//E_RTAM_STATE_UPDATING;
    _e_rtam_state_wanted = E_RTAM_STATE_RUNNING;
  }
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_run_run_to_idle(bool timeout) {
  {
    CT_GUARD_LOCK c_guard(_c_rtam_lock);
    f_rtam_stop();
    _e_rtam_state_current = E_RTAM_STATE_STOPPING;
    _i_rtam_timeout_start = f_get_time_ns64();
  }

  do {
    if (timeout)
      f_timeout_run_to_idle();
    usleep(100e3);
  } while (_e_rtam_state_current == E_RTAM_STATE_STOPPING);

  return EC_SUCCESS;
}

void CT_RTAM_EVAL::f_timeout_run_to_idle(void) {
  if (M_ABS(f_get_time_ns64() - _i_rtam_timeout_start) > C_TIMEOUT_STOP_SECS*1e9) {
    CRIT("Timeout on STOP");
    f_rtam_close();
    M_ASSERT(0);
    _e_rtam_state_current = E_RTAM_STATE_OFF;
  }
}

int CT_RTAM_EVAL::f_run_state_to_off(void) {
  {
    CT_GUARD_LOCK c_guard(_c_rtam_lock);
    f_record_stop();
    f_rtam_close();
    _e_rtam_state_current = E_RTAM_STATE_OFF;
  }

  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_event_rtam(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
  switch(in_rpc_node->get_id()) {
    case rtam::E_ID_VERSION: {
      _DBG << "VERSION received";
      break;
    }
    case rtam::E_ID_PING: {
      _DBG << "+++++++++++++++++++++++++++ PING received";
      // faire un calcul de debit
      f_rtam_handle_ping(in_rpc_node);
      //cpt++;
      break;
    }
    case rtam::E_ID_READY: {
      CT_GUARD_LOCK c_guard(_c_rtam_lock);
      /* Handle id ready */
      f_rtam_handle_id_ready(in_rpc_node);
      break;
    }
    case rtam::E_ID_CONFIG_STOPPED: {
      CT_GUARD_LOCK c_guard(_c_rtam_lock);
      if(_e_rtam_state_current == E_RTAM_STATE_STOPPING)
        _e_rtam_state_current = E_RTAM_STATE_IDLE;
      break;
    }
    case rtam::E_ID_CONFIG_READY: {
      CT_GUARD_LOCK c_guard(_c_rtam_lock);
      if(_e_rtam_state_current == E_RTAM_STATE_CONFIGURING) {
        _e_rtam_state_current = E_RTAM_STATE_CONFIGURED;
        _i_rtam_ping_count = 0;
        _i_rtam_timeout_start = f_get_time_ns64();
        /* RTAM is now ready to be started */
        f_rtam_start();
      }
      break;
    }
    default:
      break;
  }
  return EC_SUCCESS;
}

void CT_RTAM_EVAL::f_rtam_handle_ping(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
	CT_GUARD_LOCK c_guard(_c_rtam_lock);

	_i_rtam_time_last_ping_received = f_get_time_ns64();
  _i_rtam_ping_count++;
	if(_e_rtam_state_current == E_RTAM_STATE_CONFIGURED)
		_e_rtam_state_current = E_RTAM_STATE_RUNNING;

  if (_pc_writer) {
    if (in_rpc_node->to_writer(*_pc_writer) != EC_BML_SUCCESS) {
      _pc_writer = NULL;
      _str_record_file_current = "";
      _DBG << "Failed to write into record";
    }
	} else
		_CRIT << "Pointer to file not initialized ! Can not record data...";
}

int CT_RTAM_EVAL::f_rtam_handle_id_ready(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
  /* Waiting that all board are ready */
  if(in_rpc_node->get_data<uint8_t>()) {
    _WARN << "Moving to next state";
    if(_e_rtam_state_current == E_RTAM_STATE_INITIALIZING) {
      /* Move to Idle STATE */
      _e_rtam_state_current = E_RTAM_STATE_IDLE;
    }
  }
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_record_start(void) {
  // Initialize writer
  try {
  	_pc_writer = CT_GUARD<bml::node_file_writer<CT_GUARD> >(
  			new bml::node_file_writer<CT_GUARD>(
  					_str_record_folder.append("/").append(_str_record_file_current).append(".bml").c_str()));
  } catch (...) {
  	throw _CRIT<< "Unable to create file: "<< _str_record_file_current;
  }

  M_ASSERT(_pc_writer);

  _WARN << "Starting record : " << _str_record_file_current << " in " << _str_record_folder; // << "(pc_writer: " << std::hex << &_pc_writer << ")";

  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_record_stop(void) {
  //TODO Ecriture du noeud de configuration d'arret de l'enregistrement
  _WARN << "Stopping record";
  _pc_writer = NULL;
  _str_record_file_current = "";
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_start(void) {
	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_SYNC_START);
	f_port_get(E_PORT_RTAM).f_send(pc_node);
	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_stop(void) {
  CT_PORT_NODE_GUARD pc_node(rtam::E_ID_SYNC_STOP);
  f_port_get(E_PORT_RTAM).f_send(pc_node);
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_close(void) {
  CT_PORT_NODE_GUARD pc_node(rtam::E_ID_CLOSE);
  f_port_get(E_PORT_RTAM).f_send(pc_node);
  return EC_SUCCESS;
}

M_PLUGIN_CORE(CT_RTAM_EVAL, "rtam-eval-linear", M_STR_VERSION);