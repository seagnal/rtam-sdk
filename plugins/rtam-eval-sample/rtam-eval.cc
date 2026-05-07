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

  if (getenv("TEST_NO_IIC")!= NULL)
		_b_iic_enable = !atoll(getenv("TEST_NO_IIC"));
  else
		_b_iic_enable = 1;

  _i_time_iic_display = 0;
    
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
	_b_antenna_passiv_trigger = false; // remove this ??? 

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

  // f_run_receive_n_ping(15, true);

  // f_run_run_to_idle(true);

  // f_run_idle_to_configured(true);

  // f_run_receive_n_ping(10, true);

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
    if (_b_iic_enable)
      f_iic_timeout();
    usleep(100e3);
  } while (_e_rtam_state_current == E_RTAM_STATE_INITIALIZING);

  

	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_init() {
	//0x81 // BJ
	//0x82 // Atterissement
	//0xa0 // MODULE sfp
	//0X40 // MODULE FERM

  _f_fs = 25e6;

  _i_rtam_nb_tx = 0;
  _i_rtam_nb_system = 1;
  _i_rtam_affinity_tx_offset = 0;
  _vi_rtam_boards.resize(1);//_i_rtam_nb_tx);
  M_ASSERT(_vi_rtam_boards.size() == 1);
  _vi_rtam_boards[0]=0x82;

  CT_PORT_NODE_GUARD pc_node(rtam::E_ID_INIT);

  {
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

    pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0x82);//(0xFF);

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
    pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_MUX_DECIM);
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

  if (getenv("TEST_ADC")!=NULL) {
    i_decim = 8;
  } else if (getenv("TEST_CHANNEL_ID")!=NULL) {
    i_decim = 64;
  } else if (getenv("TEST_MUX_DEMOD")!=NULL) {
    i_decim = 64;
  } else {
    i_decim = 64*5;
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
    pc_node->add(rtam::E_ID_FULL_SCALE)->set_data<double>(10e-3);
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
  }

  /****************************** IIC ******************************/
  if(getenv("TEST_NO_IIC") == NULL) {
    auto pc_iic = pc_node->add(rtam::E_ID_IIC);
    pc_iic->copy(*f_iic_get_rx32());
    // pc_iic->add(rtam::E_ID_AFFINITY_SYSTEM)->set_data<uint64_t>(0x03);
  } else {
    _CRIT << "NO IIC !!!!!!!! ";
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
  static uint32_t _i_ping_count = 0;

  if (_i_rtam_ping_count < _i_ping_count)
    _i_ping_count = 0;
  while (n == 0 || _i_rtam_ping_count - _i_ping_count < n) {
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
  _i_ping_count += n;
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
    case rtam::E_ID_IIC:{
      if (_b_iic_enable) {
        CT_GUARD_LOCK c_guard(_c_status_lock);
        try {
          /* Update IIc */
          _WARN << "IIC update start";
          f_iic_status_update(in_rpc_node);
        } catch(...) {
          _CRIT << "Error during Status update";
        }
      }
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
        /* RTAM-EVAL is now ready to be started */
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

void CT_RTAM_EVAL::f_iic_timeout(void) {
    CT_GUARD_LOCK c_guard(_c_status_lock);
    uint64_t i_now = f_get_time_ns64();
    for(auto pc_it = _m_status_hw.begin(); pc_it != _m_status_hw.end(); ) {
      if(M_ABS(i_now - pc_it->second.i_time_last_seen) > 5*1e9) {
        pc_it = _m_status_hw.erase(pc_it);
      } else {
        pc_it++;
      }
    }
}

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



      for(int ii=0; ii < C_NB_VOLTAGE_STATUS; ii++) {
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
      for(int ii=0; ii < C_NB_SHT_STATUS; ii++)
      {
        if(ii < 2) {
          int i_error = 0;
          uint32_t i_offset = i_offset_config;
          i_offset += C_NB_VOLTAGE_RX32*C_NB_CMD_IIC_READ_ADS7828_CHANNEL;// READ VOLTAGE STATUS
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
      for(int ii=0; ii <  (C_NB_TEMP_FPGA); ii++)
      {
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
      char const * ac_fpga_name[]           ={"FPGA-VCCINT", "FPGA-VCCAUX", "FPGA-VCCBRAM", "FPGA-MaxINT", "FPGA-MaxAUX", "FPGA-MaxBRAM",  "FPGA-MinINT", "FPGA-MinAUX", "FPGA-MinBRAM", "AVDD", "AVVD2", "DVDD", "DRVDD", "2_AVDD", "2AVDD2", "CORE", "TERM"};
      double af_scale[C_NB_VOLTAGE_FPGA]    ={           3,             3,              3,             3,             3,              3,              3,             3,              3,      3,       6,      3,       4,        3,        6,      2,      2 };
      double af_offset[C_NB_VOLTAGE_FPGA]   ={           0,             0,              0,             0,             0,              0,              0,             0,              0,      0,       0,      0,       0,        0,        0,      0,      0 };
      double af_min_value[C_NB_VOLTAGE_FPGA]={         0.8,           1.7,            0.8,           0.8,           1.7,            0.8,            0.8,           1.7,            0.8,    1.7,     2.9,    1.3,     1.7,      1.7,      2.9,    0.85,   1.1 };
      double af_max_value[C_NB_VOLTAGE_FPGA]={         0.9,           1.9,            0.9,           0.9,           1.9,            0.9,            0.9,           1.9,            0.9,    1.9,     3.1,    1.5,     1.9,      1.9,      3.1,    0.95,   1.3 };

      for(int ii=0; ii < C_NB_VOLTAGE_FPGA; ii++)
      {
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

  if(in_b_debug && M_ABS(f_get_time_ns64() - _i_time_iic_display) > 1e8)
  {
    _i_time_iic_display = f_get_time_ns64();
    std::cout << "\t (" << s_status_hw.i_error_cnt << ")"<< std::endl;
    for(int ii=0; ii < C_NB_TEMP_FPGA; ii++) {
      std::cout << s_status_hw.astr_fpga_name[ii] << "(" << std::setw(6)
      << std::setprecision(4) << s_status_hw.af_fpga_temperature[ii] << ") ";
    }
    std::cout << std::endl;
    for(int ii=0; ii < C_NB_SHT_STATUS; ii++) {
      std::cout << s_status_hw.astr_sht_name[ii] << " : H° " <<  s_status_hw.af_sht_humidity[ii] << "\t T° " << s_status_hw.af_sht_temperature[ii] << std::endl;
    }
    std::cout << std::endl;
    for(int ii=0; ii < C_NB_VOLTAGE_STATUS; ii++) {
      std::cout << s_status_hw.astr_voltages_name[ii] << "(" << std::setw(6)
      << std::setprecision(4) << s_status_hw.af_voltages[ii] << ") ";
    }
    std::cout << std::endl;
    for(int ii=0; ii < C_NB_VOLTAGE_FPGA; ii++) {
      std::cout << s_status_hw.af_fpga_voltages[ii] << "(" << std::setw(6)
      << std::setprecision(4) << s_status_hw.astr_fpga_voltages_name[ii] << ") ";
    }
    std::cout << std::endl;
  }

out_err:
  if(b_error) {
    return EC_FAILURE;
  } else {
    return EC_SUCCESS;
  }
}

int CT_RTAM_EVAL::f_iic_status_update(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
  M_ASSERT(in_rpc_node->has(rtam::E_ID_SYSTEM_UID));
  M_ASSERT(in_rpc_node->has(rtam::E_ID_HW_UID));
  bool b_debug = true;

  uint32_t i_hw_uid = in_rpc_node->get(rtam::E_ID_HW_UID)->get_data<uint32_t>();
  if(in_rpc_node->has(rtam::E_ID_HW_GPI)) {
    _m_gpi_hw[i_hw_uid] = in_rpc_node->get(rtam::E_ID_HW_GPI)->get_data<uint32_t>();
  }
  auto & s_status_hw = _m_status_hw[i_hw_uid];

  if(f_iic_status_update_rx32(in_rpc_node, i_hw_uid, b_debug)) {
    if(s_status_hw.i_error_cnt < C_DEVICE_ERROR_MAX) {
      s_status_hw.i_error_cnt++;
    }
  } else {
    s_status_hw.i_error_cnt = 0;
  }

  s_status_hw.i_time_last_seen = f_get_time_ns64();

  return EC_SUCCESS;
}

M_PLUGIN_CORE(CT_RTAM_EVAL, "rtam-eval-sample", M_STR_VERSION);