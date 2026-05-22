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
#include <rtam/api.hh>
#include "api.hh"

using namespace std;
using namespace master::plugins::rtam_eval;
namespace rtam_eval = master::plugins::rtam_eval;
namespace rtam = master::plugins::fec;

#define TWOPI (2.0*M_PI)
#define RAND (rand())/((double) RAND_MAX)
#define RANDN (sqrt(-2.0*log(RAND))*cos(TWOPI*RAND))

// static const uint32_t gai_board_single[] = {
// 		0xC0, //Carte distribution Atterissement
// 		0xC1, //Carte distribution Plot
// 		0x81, 0x02, //Cartes Face 0
// };


// static const uint32_t gai_board_pod[] = {
// 		0xC0, //Carte distribution Atterissement
// 		0xC1, //Carte distribution Plot
// 		0x81, 0x02, //Cartes Face 0
// 		0x83, 0x04, //Cartes Face 1
// 		0x85, 0x06, //Cartes Face 2
// 		0x87, 0x08, //Cartes Face 3
// 		0x89, 0x0A, //Cartes Face 4
// 		0x8B, 0x0C  //Cartes Face 5
// };

/* Reset all status.
 */
void CT_RTAM_EVAL::f_status_reset(void) {
  CT_GUARD_LOCK c_guard(_c_status_lock);
	_m_status_hw.clear();
}

/* Constructor.
 */
extern "C" {
extern char **environ;
}


CT_RTAM_EVAL::CT_RTAM_EVAL(void) : CT_CORE() {
	D("Creating RTAM_EVAL CORE [%p]", this);

	/* Print env variables */
	{

		int i_index = 1;
		char *pc_s = *environ;

		for (; pc_s; i_index++) {
			std::string str_tmp(pc_s);

			if(str_tmp.find("TEST_") != std::string::npos) {
				_WARN << "TEST VARIABLES: " << str_tmp;
			}
			if(str_tmp.find("DEBUG_") != std::string::npos) {
				_WARN << "DEBUG VARIABLES: " << str_tmp;
			}



			/* Get env value */
			pc_s = *(environ+i_index);
		}
	}

	std::cout << "-----------------------------------------------------------------------------------------------" << std::endl;
	std::cout << "ANTENNA=[antenne, antenne2,      : Modèle de l'antenne\nmaquette16, maquette24, 2Xeval16]\n" << endl;
	std::cout << "TEST_RTAM=[Num]                  : Numéro de test à executer" << std::endl;
	std::cout << "TEST_TRIGGER_DURATION=[0-5000]   : Durée du trigger (ms)" << std::endl;
	std::cout << "TEST_PULSE_LENGTH=[0-200]        : Durée du pulse (ms)" << std::endl;
	std::cout << "TEST_PULSE_FSTART=[10000-300000] : Fréquence de démarrage du pulse (Hz)" << std::endl;
	std::cout << "TEST_PULSE_FEND=[10000-300000]   : Fréquence de démarrage du pulse (Hz)" << std::endl;
	std::cout << "TEST_PULSE_AMPLITUDE=[0-100]     : Amplitude du pulse (%%)" << std::endl;
	std::cout << "TEST_ADC=[0-7]                   : Numérisation d'une voie non démodulée par ADC" << std::endl;
	std::cout << "TEST_TONE=[0/1]                  : Numérisation d'un test tone analogique" << std::endl;
	std::cout << "TEST_CHANNEL_ID=[0/1]            : Numérisation d'un test d'identification des voies ADC HW" << std::endl;
	std::cout << "TEST_ADC_ID=[0/1]                : Numérisation d'un test d'identification des voies ADC SW" << std::endl;
	std::cout << "TEST_TERM_Z1=[0/1]               : Choix du mode de terminaison des ADC Z1" << std::endl;
	std::cout << "TEST_TERM_Z2=[0/1]               : Choix du mode de terminaison des ADC Z2" << std::endl;
	std::cout << "TEST_TERM_Z1_Z2=[0/1]            : Choix du mode de terminaison des ADC Z1//Z2" << std::endl;
	std::cout << "TEST_TERM_OPEN=[0/1]             : Choix du mode de terminaison des ADC Ouvert" << std::endl;
	std::cout << "TEST_TRIGGER_DC=[0/1]            : Set DC trigger mode instead of AC" << std::endl;
	std::cout << "TEST_ACQ_CONTINUOUS=[0/1]        : Acquisition en mode continu (necessaire avec TEST_TRIGGER_DURATION = 0)" << std::endl;
	std::cout << "TEST_FULL_SCALE=[1/1000]         : Choix de la pleine échelle de numérisation (mV)" << std::endl;
	std::cout << "TEST_NO_TX=[0/1]                 : Desactive l'émission" << std::endl;
	std::cout << "TEST_NO_PULSER=[0/1]             : Desactive l'usage des pulseurs" << std::endl;
	std::cout << "TEST_NO_VGA=[0/1]                : Desactive l'usage des vga" << std::endl;
	std::cout << "TEST_NO_IIC=[0/1]                : Desactive la lecture IIC" << std::endl;
	std::cout << "TEST_SET_MASTER=[Num]            : ID de la carte MASTER" << std::endl;
  std::cout << "DEBUG_ON_RUNNING=[0/1]           : Active la lecture du lien de debug pendant l'acquisition" << std::endl;
	std::cout << "DEBUG_ON_ERROR=[0/1]             : Active la lecture du lien de debug en cas d'erreur" << std::endl;
  std::cout << "ENABLE_IIC_STATUS=[1]            : Active la lecture du lien de debug en cas d'erreur" << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------------" << std::endl;

  	/* Init random seed */
	srand (time(NULL));

	/* Create in port */
	f_port_create(E_PORT_RTAM, "rtam", M_PORT_BIND(CT_RTAM_EVAL, f_event_rtam, this));
	f_port_create(E_PORT_FE, "fe", M_PORT_BIND(CT_RTAM_EVAL, f_event_fe, this));

	/* Get unique id of device */
	if(getenv("PROC_MANAGER_ID")!= NULL) {
		_i_uid = atoll(getenv("PROC_MANAGER_ID"));
	} else {
		_i_uid = 0;
	}

	/* init id user rtam config */
	_i_user_id_config_rtam = 0;

 	_b_status_zda = false;
  	_b_status_pps = false;


	/* rapport of error rtam */
  _i_status_sample_missing_cnt = 0;
  _i_status_sample_lost_cnt = 0;
  _i_status_sample_error_cnt = 0;



	/* Default state wanted for audio*/
	_e_rtam_state_wanted = E_RTAM_STATE_OFF;
	_e_record_state_wanted = E_RECORD_STATE_IDLE;
	_e_rtam_state_current = E_RTAM_STATE_OFF;
	_e_record_state_current = E_RECORD_STATE_IDLE;


	/* Select antenna mode */
	_e_antenna_config = E_ANTENNA_CONFIG_MAQUETTE24;
	_b_antenna_passiv_trigger = false;
#if 1
	if(getenv("ANTENNA")!= NULL) {
		auto str_antenna = std::string(getenv("ANTENNA"));
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
#endif

	/* Init record */
  if(getenv("TEST_RECORD_DIR")!= NULL) {
    _str_record_folder = getenv("TEST_RECORD_DIR");
	} else {
    _str_record_folder = "/tmp";
	}

	_i_record_autosplit_gb = 1;
	if(getenv("TEST_FILENAME")!= NULL) {
		_str_record_file_current = getenv("TEST_FILENAME");
	} else {
		_str_record_file_current = "autorun";
	}
	_i_test_update_time = 0;
	_i_test_id = 0;
	_pc_writer = NULL;



	/* Context */
	_pc_context = CT_GUARD<CT_HOST_CONTEXT>(CT_HOST::host->f_new_context());
	M_ASSERT(_pc_context);
	_pc_context_test = CT_GUARD<CT_HOST_CONTEXT>(CT_HOST::host->f_new_context());
	M_ASSERT(_pc_context_test);
	D("RTAM_EVAL CORE created");
}

CT_RTAM_EVAL::~CT_RTAM_EVAL() {

}

void CT_RTAM_EVAL::f_test_autorun_off(void) {
	_WARN << "AUTOTEST ANTENNA OFF";

  CT_PORT_NODE_GUARD pc_node(E_ID_CONFIG_UPDATE);
  	pc_node->add(rtam_eval::E_ID_CONFIG_ANTENNA_MODE)->set_data<uint8_t>(E_ANTENNA_OFF);

	f_port_get(E_PORT_FE).f_receive(pc_node);
}


void CT_RTAM_EVAL::f_test_autorun_antenna(void) {
	_WARN << "AUTOTEST ANTENNA RUN";

  CT_PORT_NODE_GUARD pc_node(E_ID_CONFIG_UPDATE);
  	pc_node->add(rtam_eval::E_ID_CONFIG_ANTENNA_MODE)->set_data<uint8_t>(E_ANTENNA_AUTO);
	/* Enregistrement des données brutes */
	pc_node->add(rtam_eval::E_ID_CONFIG_RECORD_RAW_BASE_NAME)->set_data<std::string>(_str_record_file_current);
  std::string str_description_celerity = "<depth><value>   0.00 <celerity>1500.00</celerity></value><value>1000.00 <celerity>1500.00</celerity></value></depth>";

	f_port_get(E_PORT_FE).f_receive(pc_node);
}


int CT_RTAM_EVAL::f_post_init(void) {
	int ec;
	WARN("Starting RTAM-EVAL");


	/* AUTO TEST TRIGGER */
	auto pc_tmp = getenv("TEST_RTAM");
	bool b_need_test_thread = false;
	if(pc_tmp) {
		_i_test_id = atoi(pc_tmp);
		switch(_i_test_id) {
			/* TEST 0: RTAM LAUNCH + RECORD */
			case 0: {
			f_test_autorun_antenna();
			b_need_test_thread = false;
			break;
		}
			case 1: {
      f_test_autorun_antenna();
      b_need_test_thread = true;
      break;
    }
		}

		if(b_need_test_thread) {
			M_ASSERT(_pc_context_test);
			/** Start context */
			ec = _pc_context_test->f_start(M_CONTEXT_BIND(CT_RTAM_EVAL, f_run_tests, this));
			if (ec != EC_SUCCESS) {
				CRIT("Unable to start context");
				ec = EC_FAILURE;
				goto out_err;
			}
		}
	}

	ec = EC_SUCCESS;
	out_err: return ec;
}

int CT_RTAM_EVAL::f_post_profile(void) {
	int ec;
	M_ASSERT(_pc_context);

	/** Start context */
	ec = _pc_context->f_start(M_CONTEXT_BIND(CT_RTAM_EVAL, f_run, this));
	if (ec != EC_SUCCESS) {
		CRIT("Unable to start context");
		ec = EC_FAILURE;
		goto out_err;
	}

	ec = EC_SUCCESS;
	out_err: return ec;
}

int CT_RTAM_EVAL::f_pre_profile(void) {
	_pc_context->f_stop();
	_pc_context_test->f_stop();
	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_run_tests(CT_HOST_CONTEXT& inout_rc_context)
{

  switch(_i_test_id) {

		case 1: {
      static bool b_state = true;
      static uint64_t gi_time_start = f_get_time_ns64();


      if(f_get_time_ns64()- gi_time_start > 15e9 ) {
        if(b_state) {
          f_test_autorun_off();
          b_state = false;
        } else {
          f_test_autorun_antenna();
          b_state = true;
        }
        gi_time_start = f_get_time_ns64();
      }
      break;
    }
  }


	usleep(100e3);
	return EC_BYPASS;
}

int CT_RTAM_EVAL::f_run(CT_HOST_CONTEXT& inout_rc_context) {
	{
		CT_GUARD_LOCK c_guard(_c_config_lock);


		std::string str_record_raw_base_name;
		if(_pc_config_wanted) {
			if (_pc_config_wanted->has(rtam_eval::E_ID_CONFIG_RECORD_RAW_BASE_NAME)) {
				str_record_raw_base_name = _pc_config_wanted->get(rtam_eval::E_ID_CONFIG_RECORD_RAW_BASE_NAME)->get_data<std::string>();
			}

		}

		/* Activation / Desactivation de la relecture des données de fusion */
		{
			CT_GUARD_LOCK c_guard(_c_record_lock);
			f_record_check_config(str_record_raw_base_name);
			f_record_fsm();
		}


		/* Execute state machines */
		{
			CT_GUARD_LOCK c_guard(_c_rtam_lock);
			f_rtam_check_config();
			f_rtam_fsm();
		}

		/* Update status */
    {
      CT_GUARD_LOCK c_guard(_c_status_lock);
      f_iic_timeout();
    }

		/* Send status */
		f_status_send();

	}

	/* Execute each 100ms */
	usleep(100e3);

	return EC_SUCCESS;
}

/* Audio port*/
int CT_RTAM_EVAL::f_status_send(void) {
	CT_GUARD_LOCK c_guard(_c_status_lock);
	static uint64_t i_last = 0;
	static ET_RTAM_STATE e_rtam_state_current_last;
	static ET_RECORD_STATE e_record_state_current_last;

	if(getenv("ENABLE_IIC_STATUS")!= NULL) {
		if((f_get_time_ns64() - i_last) > 3*1e9)
		  f_iic_display_status();
	}

	if(
			((f_get_time_ns64() - i_last) > 3*1e9)
		|| (e_rtam_state_current_last != _e_rtam_state_current)
			|| (e_record_state_current_last != _e_record_state_current)
	) {

		std::cout  << std::setw(14) << " CURRENT:" <<
			std::setw(6) << "rtam: " << std::setw(4) << f_rtam_print_state(_e_rtam_state_current) <<
				std::setw(6) << "rec: " << std::setw(4)<< f_record_print_state(_e_record_state_current) <<std::endl;

		std::cout << std::setw(14) << " WANTED: " <<
			std::setw(6) << "rtam: " << std::setw(4) << f_rtam_print_state(_e_rtam_state_wanted) <<
				std::setw(6) << "rec: " << std::setw(4) << f_record_print_state(_e_record_state_wanted) << std::endl;
        i_last = f_get_time_ns64();

      }
	//_DBG << _V(_i_rtam_timeout_start);
	e_rtam_state_current_last = _e_rtam_state_current;
		e_record_state_current_last = _e_record_state_current;


	return EC_SUCCESS;
}


/* Front end control/command port.
 */
int CT_RTAM_EVAL::f_event_fe(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
	CT_GUARD_LOCK c_guard(_c_config_lock);

	switch(in_rpc_node->get_id()) {

	case E_ID_CONFIG_UPDATE: {
		_WARN << "Received config";

		/* Pod configuration */
		{
				CT_GUARD_LOCK c_guard(_c_rtam_lock);
				_pc_rtam_config_wanted = CT_GUARD<CT_RTAM_EVAL_CONFIG>(new CT_RTAM_EVAL_CONFIG(in_rpc_node));
		}

		_pc_config_wanted = in_rpc_node;
		break;
	}

	default: {
		_CRIT << "Default receive :" << CT_HOST::host->f_id_name(in_rpc_node->get_id());
		break;
	}

	}
	//out_err:
	return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_settings(CT_NODE & inout_rc_node) {
	int ec;
	ec = EC_SUCCESS;
	return ec;
}

M_PLUGIN_CORE(CT_RTAM_EVAL, "rtam-eval", M_STR_VERSION);
