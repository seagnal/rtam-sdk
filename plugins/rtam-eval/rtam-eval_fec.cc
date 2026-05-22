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

//#include <bml_node.h>

using namespace std;
using namespace master::plugins::rtam_eval;
namespace rtam = master::plugins::fec;


const char * CT_RTAM_EVAL::f_rtam_print_state(enum ET_RTAM_STATE const in_e_state) {
  switch(in_e_state) {
    case E_RTAM_STATE_OFF:
      return "OFF ";
    case E_RTAM_STATE_TEST:
      return "TEST";
    case E_RTAM_STATE_IDLE:
      return "IDLE";
    case E_RTAM_STATE_RUNNING:
      return "RUN ";
    case E_RTAM_STATE_CONFIGURED:
      return "CFGD";
    case E_RTAM_STATE_STOPPING:
      return "STPG";
    case E_RTAM_STATE_INITIALIZING:
      return "INIT";
    case E_RTAM_STATE_CONFIGURING:
      return "CFGG";
    case E_RTAM_STATE_UPDATING:
      return "UPDG";
    case E_RTAM_STATE_ERROR:
      return "ERR ";
    default:
      return "UNKN";
  }
}


/* Sonar control command & data port.
*/
int CT_RTAM_EVAL::f_event_rtam(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {


  switch(in_rpc_node->get_id()) {
        case rtam:: E_ID_VERSION: {
    _DBG << "VERSION received";
    break;
  }
        case rtam:: E_ID_PING: {
    // faire un calcul de debit
            f_rtam_handle_ping(in_rpc_node);
    //cpt++;

    break;
  }
        case rtam::E_ID_IIC: {

    CT_GUARD_LOCK c_guard(_c_status_lock);
    try {
      /* Update IIc */
      f_iic_status_update(in_rpc_node);
    } catch(...) {
      _CRIT << "Error during Status update";
    }
    break;
  }
        case rtam:: E_ID_READY: {
            CT_GUARD_LOCK c_guard(_c_rtam_lock);
    _WARN << "READY received";
    /* Handle id ready */
            f_rtam_handle_id_ready(in_rpc_node);
    break;
  }
        case rtam:: E_ID_CONFIG_STOPPED: {
            CT_GUARD_LOCK c_guard(_c_rtam_lock);
            if(_e_rtam_state_current == E_RTAM_STATE_STOPPING) {
              	_e_rtam_state_current = E_RTAM_STATE_IDLE;
    }
    break;
  }
        case rtam:: E_ID_CONFIG_READY: {
            CT_GUARD_LOCK c_guard(_c_rtam_lock);
            if(_e_rtam_state_current == E_RTAM_STATE_CONFIGURING) {
				_e_rtam_state_current = E_RTAM_STATE_CONFIGURED;
				_i_rtam_ping_count = 0;
				_i_rtam_timeout_start = f_get_time_ns64();
				/* RTAM is now ready to be started */
				f_rtam_start();
    }
    //f_status_reset(); Why ? Not be perform here version is erased
    break;
  }
  }
  return EC_SUCCESS;
}

void CT_RTAM_EVAL::f_rtam_clear_status(void) {
  /* Clear noise array */
  _m_noise.clear();

  /* Clear sample count status */
  _i_status_sample_lost_cnt = 0;
  _f_status_sample_lost_acc = 0;
}

int CT_RTAM_EVAL::f_rtam_version_update(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
  CT_GUARD_LOCK c_lock(_c_status_lock);
  // _m_status_hw.clear(); delete this

	auto s_res = in_rpc_node->get_childs(rtam::E_ID_HW_VERSION);
  for (auto & pc_it : s_res) {
		M_ASSERT(pc_it->has(rtam::E_ID_HW_UID));
		uint32_t i_hw_uid = pc_it->get(rtam::E_ID_HW_UID)->get_data<uint32_t>();
    std::string str_tmp = pc_it->get_data<std::string>();
    //_m_status_version_hw[i_hw_uid] = str_tmp;
    _m_status_hw[i_hw_uid].str_version_hw = str_tmp;
  }
  {
		M_ASSERT(in_rpc_node->has(rtam::E_ID_VERSION));
		_str_status_rtam_version = in_rpc_node->get(rtam::E_ID_VERSION)->get_data<std::string>();
  }

  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_init() {
//0x81 // BJ
//0x82 // Atterissement
//0xa0 // MODULE sfp
//0X40 // MODULE FERM
	/* Initialize RTAM config */
	//f_rtam_config_init();

  switch(_e_antenna_config) {
    case E_ANTENNA_CONFIG_MAQUETTE24: {
      _f_fs = 125e6/4; // 31.25 MHz
      break;
    }
    default: {
      _f_fs = 25e6;
      break;
    }
  }

  {
    bool b_init_jtag = false;
    if(getenv("DEBUG_ON_ERROR")) {
      b_init_jtag = true;
    }
    if(getenv("DEBUG_ON_RUNNING")) {
      b_init_jtag = true;
    }

    if(b_init_jtag) {
			CT_PORT_NODE_GUARD pc_node(rtam::E_ID_JTAG_INIT);
			f_port_get(E_PORT_RTAM).f_send(pc_node);
    }
  }

  bool b_init_pulser = false;
  if(getenv("TEST_NO_PULSER") && atoi(getenv("TEST_NO_PULSER"))) {
    b_init_pulser = false;
  }

  switch(_e_antenna_config) {
    case E_ANTENNA_CONFIG_MAQUETTE16:
    case E_ANTENNA_CONFIG_MAQUETTE24:
			_i_rtam_nb_tx = 1;
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

  /* Reset noise */
	f_rtam_clear_status();

	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_INIT);

  switch(_e_antenna_config) {
    case E_ANTENNA_CONFIG_MAQUETTE16:
    case E_ANTENNA_CONFIG_MAQUETTE24:
      {
        uint8_t i_id_board_pwm = _vi_rtam_boards[0];

          {
            CT_GUARD<CT_PORT_NODE> pc_syst = pc_node->add(rtam::E_ID_INIT_SYST);
            {
              auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
              pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_SYSTEM);
              pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(i_id_board_pwm);
              pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
            }
            if(b_init_pulser) {
              for(uint8_t i_tx=0; i_tx<_i_rtam_nb_tx; i_tx++) {
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
		case E_ANTENNA_CONFIG_ANTENNE2:
      {
			for(uint8_t i_id_board=0; i_id_board<_i_rtam_nb_system; i_id_board++) {
				CT_GUARD<CT_PORT_NODE> pc_syst = pc_node->add(rtam::E_ID_INIT_SYST);
        {

					auto pc_subsyst = pc_syst->add(rtam::E_ID_INIT_HW);
					pc_subsyst->add(rtam::E_ID_HW_TYPE)->set_data<uint8_t>(rtam::E_SYST_TYPE_SYSTEM);
					pc_subsyst->add(rtam::E_ID_HW_UID)->set_data<uint64_t>(_vi_rtam_boards[i_id_board]);
					pc_subsyst->add(rtam::E_ID_HW_INTERNAL_ID)->set_data<uint32_t>(0);
            }
          
          if(b_init_pulser) {
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
      break;
    }
		default:
		{
      //M_BUG();
      break;
    }
  }




	_WARN << "Initializing RTAM";
	f_port_get(E_PORT_RTAM).f_send(pc_node);
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
  if((getenv("TEST_TRIGGER_DC")!=NULL) && (atoi(getenv("TEST_TRIGGER_DC")))) {
    pc_node->add(rtam::E_ID_TRIGGER_LATCH_MODE)->set_data<uint8_t>(rtam::E_TRIGGER_LATCH_MODE_NONE);
  } else {
	  pc_node->add(rtam::E_ID_TRIGGER_LATCH_MODE)->set_data<uint8_t>(rtam::E_TRIGGER_LATCH_MODE_AC);
  }
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
      case E_ANTENNA_CONFIG_MAQUETTE24: {
				pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_ADC_MUX_INPUT);
        break;
      }
      default: {
				pc_node->add(rtam::E_ID_MUX)->set_data<uint8_t>(rtam::E_MUX_DECIM);
        break;
      }
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
      }
    }
  }

  /* Set cutoff frequency */
  {
    float f_freq_cuttoff = _f_fs/float(i_decim)*0.4;

    if(getenv("TEST_CUTOFF_FREQUENCY")!=NULL) {
      f_freq_cuttoff = atof(getenv("TEST_CUTOFF_FREQUENCY"));
    }
    _DBG << _V(f_freq_cuttoff);
		pc_node->add(rtam::E_ID_FREQ_CUTOFF)->set_data<double>(f_freq_cuttoff);
  }

  if((getenv("TEST_TERM_Z1") != NULL) && atoi(getenv("TEST_TERM_Z1"))) {
		pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_Z1);
  } else if ((getenv("TEST_TERM_Z2") != NULL)&& atoi(getenv("TEST_TERM_Z2"))) {
		pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_Z2);
  } else if ((getenv("TEST_TERM_OPEN") != NULL)&& atoi(getenv("TEST_TERM_OPEN"))) {
		pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_OPEN);
  } else if((getenv("TEST_TERM_Z1_Z2") != NULL)&& atoi(getenv("TEST_TERM_Z1_Z2"))) {
		pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_Z1_Z2);
  } else {
		pc_node->add(rtam::E_ID_TERM_MODE)->set_data<uint8_t>(rtam::E_ADC_TERM_MODE_OPEN);
  }

  /* Window mode */
  if ((getenv("TEST_ACQ_CONTINUOUS") != NULL)&& atoi(getenv("TEST_ACQ_CONTINUOUS"))) {
		pc_node->add(rtam::E_ID_ACQ_MODE)->set_data<uint8_t>(rtam::E_ADC_ACQ_MODE_CONTINUOUS);
  } else {
		pc_node->add(rtam::E_ID_ACQ_MODE)->set_data<uint8_t>(rtam::E_ADC_ACQ_MODE_WINDOWED);
  }

  /* Set Sampling samples */
  uint32_t i_nb_sample = f_duration*_f_fs/float(i_decim);

  if (getenv("TEST_NB_SAMPLE") != NULL) {
    i_nb_sample = atoi(getenv("TEST_NB_SAMPLE"));
  }

  if (i_nb_sample >= pow(2,19)) { // NB_SAMPLE is limited to 2**19-1 into FPGA
    _CRIT << "Nb samples was " << i_nb_sample << " and have been limited to 524287 (2**19-1) !";
    i_nb_sample = pow(2,19)-1;
  }
  {
    uint32_t i_spp = 124;
    if (getenv("TEST_SPP") != NULL) {
      i_spp = atoi(getenv("TEST_SPP"));
    }

    i_nb_sample = round(i_nb_sample/i_spp)*i_spp;

	  pc_node->add(rtam::E_ID_NB_SAMPLE)->set_data<uint32_t>(i_nb_sample);
  }
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

  #if 1
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
		switch(_e_antenna_config) {
      // FIXME NEED LOOP, BUG DUAL CONFIGURATION SYSTEM RX AND TX FOR PULSE
      default: {
				for(uint8_t i_tx = 0; i_tx < _i_rtam_nb_tx; i_tx++) {
					auto pc_pulse = pc_node->add(rtam::E_ID_PULSE);
					pc_pulse->add(rtam::E_ID_AFFINITY_SYSTEM)->set_data<uint64_t>(1 << (i_tx+_i_rtam_affinity_tx_offset));
					pc_pulse->add(rtam::E_ID_PULSER_MODE)->set_data<uint8_t>(rtam::E_PULSE_HFM);
					pc_pulse->add(rtam::E_ID_PULSER_FSTART)->set_data<float>(i_freq_start);
					pc_pulse->add(rtam::E_ID_PULSER_FEND)->set_data<float>(i_freq_end);
					//pc_pulse->add(rtam::E_ID_PULSER_TIME_WEIGHTING_ID)->set_data<uint8_t>(0);
					//pc_pulse->add(rtam::E_ID_PULSER_TIME_WEIGHTING_STEP)->set_data<float>(float(((float)i_pulse_length/1000.0)/64.0));
					pc_pulse->add(rtam::E_ID_PULSER_DURATION)->set_data<double>(double(i_pulse_length)/1000.0);
					pc_pulse->add(rtam::E_ID_PULSER_AMPLITUDE)->set_data<double>(double(i_amplitude)/100.0);
        }
        break;
      }
    }
  }
  #endif


  /****************************** IIC ******************************/
  if(getenv("TEST_NO_IIC") == NULL) {
    switch(_e_antenna_config) {
      case E_ANTENNA_CONFIG_2XEVAL16:
      case E_ANTENNA_CONFIG_ANTENNE1:
      case E_ANTENNA_CONFIG_ANTENNE2:
      case E_ANTENNA_CONFIG_MAQUETTE16:
			case E_ANTENNA_CONFIG_MAQUETTE24:
        {
				{
				auto pc_iic = pc_node->add(rtam::E_ID_IIC);
          pc_iic->copy(*f_iic_get_rx32());
				pc_iic->add(rtam::E_ID_AFFINITY_SYSTEM)->set_data<uint64_t>(0x03);
        }
        break;
      }
    }
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

  if(getenv("TEST_CH")!=NULL) {
    pc_node->add(rtam::E_ID_CHANNELS)->set_data<uint8_t>(rtam::E_CHANNELS_WHITE_LIST);
    std::string s = getenv("TEST_CH");
    std::vector<std::string> vec;
    f_string_split(s, ",", vec);
    for(uint32_t ii= 0; ii < vec.size(); ii ++) {
      std::vector<std::string> vec1;
      f_string_split(vec[ii], ":", vec1);
      if (vec1.size() == 1) {
        //std::cout << stoi(vec1[0]) << std::endl;
        pc_node->get(rtam::E_ID_CHANNELS)->add(rtam::E_ID_CHANNEL)->set_data<uint16_t>(stoi(vec1[0]));
      }
      if (vec1.size() == 2) {
        for(int32_t jj= stoi(vec1[0]); jj <= stoi(vec1[1]); jj ++) {
          //std::cout << jj << std::endl
          pc_node->get(rtam::E_ID_CHANNELS)->add(rtam::E_ID_CHANNEL)->set_data<uint16_t>(jj);
        }
      }
      if (vec1.size() == 3) {
        for(int32_t jj= stoi(vec1[0]); jj <= stoi(vec1[2]); jj += stoi(vec1[1]) ) {
          //std::cout << jj << std::endl;
          pc_node->get(rtam::E_ID_CHANNELS)->add(rtam::E_ID_CHANNEL)->set_data<uint16_t>(jj);
        }
      }
    }
  }

  /* Set vga values */
  if(getenv("TEST_NO_VGA") && atoi(getenv("TEST_NO_VGA"))) {
    _DBG << "NO VGA";
  } else {
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

#if 0
    for(uint32_t i_dac=0; i_dac<i_sz_vga; i_dac++) {
      _DBG << _V(i_dac) << _V(pi_tmp[i_dac]) << _V(i_step);
    }
#endif
  }

  return pc_config;
}

int CT_RTAM_EVAL::f_rtam_config(void) {

  /* Initialisation de la configuration */
	auto pc_config = f_rtam_config_init();

  {
		auto pc_node = pc_config->get(rtam::E_ID_SCENARIO);
    if(getenv("TEST_SET_MASTER") == NULL) {
      switch(_e_antenna_config) {
        case E_ANTENNA_CONFIG_2XEVAL16: {
  				pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xFF);//(0xA1);
        _CRIT << "SET MASTER";
        break;
        }
        case E_ANTENNA_CONFIG_MAQUETTE24:
  			case E_ANTENNA_CONFIG_MAQUETTE16:
  			{
  				pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0x82);//(0xFF);
          _CRIT << "SET MASTER";
          break;
        }
        case E_ANTENNA_CONFIG_ANTENNE1:
  			case E_ANTENNA_CONFIG_ANTENNE2:
  			{
  				pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xFF);
          _CRIT << "SET MASTER";
          break;
        }
        default: {
          M_BUG();
          break;
        }
      }
    } else {
      uint32_t i_master = atoi(getenv("TEST_SET_MASTER"));
      pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(i_master);
    }
		_CRIT << "SET MASTER: " <<std::hex << pc_node->get(rtam::E_ID_TRIGGER_MASTER)->get_data<uint64_t>() ;
  }


	/* Transmission du noeud au plugin RTAM */
	f_port_get(E_PORT_RTAM).f_send(pc_config);

  return EC_SUCCESS;
}

void CT_RTAM_EVAL::f_rtam_handle_ping(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
	CT_GUARD_LOCK c_guard(_c_rtam_lock);
  int ec;		
  uint64_t i_ping_uid = in_rpc_node->get(rtam::E_ID_TRIGGER_UID)->get_data<uint64_t>();
  uint32_t i_system_uid = in_rpc_node->get(rtam::E_ID_SYSTEM_UID)->get_data<uint32_t>();
  

    _DBG << "+++++++++++++++++++++++++++ PING:" << std::hex << i_ping_uid << " SYS:"<< i_system_uid ;

	_i_rtam_time_last_ping_received = f_get_time_ns64();
	if(_e_rtam_state_current == E_RTAM_STATE_CONFIGURED) {
		_e_rtam_state_current = E_RTAM_STATE_RUNNING;
  }

  if (_pc_writer) {
    ec = in_rpc_node->to_writer(*_pc_writer);
    if (ec != EC_BML_SUCCESS) {
      _pc_writer = NULL;
      _str_record_file_current = "";
      _DBG << "Failed to write into record";
    }
	} else
		_CRIT << "Pointer to file not initialized ! Can not record data...";
}

/* Démarrage du plugin RTAM.
*/
int CT_RTAM_EVAL::f_rtam_start(void) {
	_i_rtam_first_time = f_get_time_ns64();
	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_SYNC_START);
	f_port_get(E_PORT_RTAM).f_send(pc_node);
  return EC_SUCCESS;
}

/* Arret du plugin RTAM.
*/
int CT_RTAM_EVAL::f_rtam_stop(void) {
	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_SYNC_STOP);
	f_port_get(E_PORT_RTAM).f_send(pc_node);
  return EC_SUCCESS;
}
/* Fermeture du plugin RTAM.
*/
int CT_RTAM_EVAL::f_rtam_close(void) {
	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_CLOSE);
	f_port_get(E_PORT_RTAM).f_send(pc_node);
  return EC_SUCCESS;
}


int CT_RTAM_EVAL::f_rtam_jtag_trigger(void) {
	CT_PORT_NODE_GUARD pc_node(rtam::E_ID_JTAG_TRIGGER);
	f_port_get(E_PORT_RTAM).f_send(pc_node);
  return EC_SUCCESS;
}


int CT_RTAM_EVAL::f_rtam_fsm() {
	switch(_e_rtam_state_current) {
		case E_RTAM_STATE_OFF: {
			switch(_e_rtam_state_wanted) {
				case E_RTAM_STATE_RUNNING:
				case E_RTAM_STATE_IDLE:
				{
          _vi_boards_provided.clear();
					f_rtam_init();
					_e_rtam_state_current = E_RTAM_STATE_INITIALIZING;
					_i_rtam_timeout_start = f_get_time_ns64();
          break;
        }
        default:
          break;
      }
      break;
    }
		case E_RTAM_STATE_IDLE: {
			switch(_e_rtam_state_wanted) {
				case E_RTAM_STATE_RUNNING: {
					f_rtam_config();
					_e_rtam_state_current = E_RTAM_STATE_CONFIGURING;
					_i_rtam_timeout_start = f_get_time_ns64();
          break;
        }
				case E_RTAM_STATE_OFF: {
					f_rtam_close();
					_e_rtam_state_current = E_RTAM_STATE_OFF;
          break;
        }
        default:
          break;
      }
      break;
    }

		case E_RTAM_STATE_RUNNING: {
			switch(_e_rtam_state_wanted) {
				case E_RTAM_STATE_OFF:
				case E_RTAM_STATE_IDLE:
				{
					f_rtam_stop();
					_e_rtam_state_current = E_RTAM_STATE_STOPPING;
					_i_rtam_timeout_start = f_get_time_ns64();
          break;
        }
				case E_RTAM_STATE_UPDATING : {
					f_rtam_config();//f_rtam_update();
					_e_rtam_state_current = E_RTAM_STATE_RUNNING;//E_RTAM_STATE_UPDATING;
					_e_rtam_state_wanted = E_RTAM_STATE_RUNNING;
          break;
        }
				case E_RTAM_STATE_RUNNING: {
          if(!_b_antenna_passiv_trigger) {
						if(getenv("RTAM_BFU_DEBUG") == NULL){
							if(M_ABS(f_get_time_ns64()-_i_rtam_time_last_ping_received) > C_TIMEOUT_PING_SECS*1e9) {
                _WARN<< "TIMEOUT DETECTED ...";
								_e_rtam_state_current = E_RTAM_STATE_ERROR;
              }
            }
          }

          if(getenv("DEBUG_ON_RUNNING")) {
            static uint64_t i_time_last = 0;
            if(!i_time_last) {
              i_time_last = f_get_time_ns64();
            } else {
              if(M_ABS(f_get_time_ns64() - i_time_last) > 60*1e9) {
                  _WARN << "JTAG TRIGGER !";
								f_rtam_jtag_trigger();
                i_time_last = 0;
              }
            }
          }

          break;
        }
				default:
          break;
      }
      break;
    }

		case E_RTAM_STATE_UPDATING: {
      /* Going back to running immediatly */
			_e_rtam_state_current = E_RTAM_STATE_RUNNING;
      break;
    }

		case E_RTAM_STATE_STOPPING: {
			if(M_ABS(f_get_time_ns64() - _i_rtam_timeout_start) > C_TIMEOUT_STOP_SECS*1e9) {
        CRIT("Timeout on STOP");
				_e_rtam_state_current = E_RTAM_STATE_ERROR;
      }
      break;
    }
    case E_RTAM_STATE_CONFIGURED: {
      //break; NO BREAK (Ceci est voulu pour executer le timeout ci-dessous)
    }
    case E_RTAM_STATE_INITIALIZING:
    case E_RTAM_STATE_CONFIGURING:
    {

			if(M_ABS(f_get_time_ns64() - _i_rtam_timeout_start) > C_TIMEOUT_CONFIGURE_SECS*1e9) {
        if(getenv("DEBUG_ON_ERROR")) {
          static uint64_t i_time_last = 0;
          if(!i_time_last) {
            _WARN << "JTAG TRIGGER !";
						f_rtam_jtag_trigger();
            i_time_last = f_get_time_ns64();
          } else {
            if(M_ABS(f_get_time_ns64() - i_time_last) > C_SECOND_JTAG_REQUEST_SECS*1e9) {
              i_time_last = 0;
            }
          }
        } else {

					if(_b_antenna_passiv_trigger && (_e_rtam_state_current == E_RTAM_STATE_CONFIGURED)) {
            /* Nothing to do */
          } else {
            _CRIT << "ERROR DETECTED MOVED TO OFF STATE, NO RUNNING";
						_e_rtam_state_current = E_RTAM_STATE_ERROR;
						_i_rtam_error_cnt++;
          }
        }
      }

      break;
    }
		case E_RTAM_STATE_ERROR: {
			f_rtam_close();
			_e_rtam_state_current = E_RTAM_STATE_OFF;
      break;
    }
    default:
    break;
  }
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_check_config(void) {
	bool b_rtam_up = false;
	bool b_rtam_same_tx = false;
	bool b_rtam_same_rx = false;
	//bool b_rtam_same_mode = false;
	ET_ANTENNA_MODE e_rtam_antenna_mode = ET_ANTENNA_MODE::E_ANTENNA_OFF;

	if (_pc_rtam_config_wanted != NULL) {
		/* Check update on RTAM configuration */
		b_rtam_up = _pc_rtam_config_wanted->f_is_up();
		e_rtam_antenna_mode = _pc_rtam_config_wanted->f_get_antenna_mode();
		b_rtam_same_tx = true;
		b_rtam_same_rx = true;
  }
	//_DBG << _V(b_rtam_up) << _V(e_rtam_antenna_mode) << _V(f_rtam_print_state(_e_rtam_state_current));

	/** Wanted state of RTAM according to configuration */
	switch(_e_rtam_state_current) {
		case E_RTAM_STATE_OFF:
		case E_RTAM_STATE_IDLE:
		{
			if((b_rtam_up) && (e_rtam_antenna_mode != ET_ANTENNA_MODE::E_ANTENNA_OFF) ) {
				_e_rtam_state_wanted = E_RTAM_STATE_RUNNING;
				/* Store RTAM configuration */
				_pc_rtam_config_current = _pc_rtam_config_wanted;
      } else {
				_e_rtam_state_wanted = E_RTAM_STATE_IDLE;
      }
      break;
    }
		case E_RTAM_STATE_RUNNING: {
			if((!b_rtam_up) || (e_rtam_antenna_mode == ET_ANTENNA_MODE::E_ANTENNA_OFF)) {
				_e_rtam_state_wanted = E_RTAM_STATE_IDLE;
				_pc_rtam_config_current = _pc_rtam_config_wanted;
      } else {
				if(!b_rtam_same_rx) {
					_e_rtam_state_wanted = E_RTAM_STATE_UPDATING;
					/* Store RTAM configuration */
					_pc_rtam_config_current = _pc_rtam_config_wanted;
				} else if (!b_rtam_same_tx){
					_e_rtam_state_wanted = E_RTAM_STATE_UPDATING;
					/* Store RTAM configuration */
					_pc_rtam_config_current = _pc_rtam_config_wanted;
        }
      }
      break;
    }
    default:
    break;
  }
  return EC_SUCCESS;
}


/* Update version */
int CT_RTAM_EVAL::f_rtam_handle_id_ready(CT_GUARD<CT_PORT_NODE> const & in_rpc_node) {
	/* Reset all status on RTAM READY */
  f_status_reset();

  /* Mise à jour de la version */
	f_rtam_version_update(in_rpc_node);

  /* Waiting that all board are ready */
  if(in_rpc_node->get_data<uint8_t>()) {
    _WARN << "Moving to next state";
		if(_e_rtam_state_current == E_RTAM_STATE_INITIALIZING) {
      /* Move to Idle STATE */
			_e_rtam_state_current = E_RTAM_STATE_IDLE;
      /* Send status update */
      f_status_send();
    }
  }
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_rtam_update(void) {
	CT_PORT_NODE_GUARD pc_config(rtam::E_ID_CONFIG);
  //TODO Generation du noeud de configuration de mise à jour
	f_port_get(E_PORT_RTAM).f_send(pc_config);
  return EC_SUCCESS;
}


void CT_RTAM_EVAL::f_rtam_update_test(uint32_t in_i_id_test) {
  #if 1

  _WARN << "UPDATE TEST" << _V(in_i_id_test);
	if(_e_rtam_state_current != E_RTAM_STATE_RUNNING) {
		_FATAL << "RTAM should be running";
  }

	CT_GUARD<CT_PORT_NODE> pc_config = f_rtam_config_init();

  {
		auto pc_node = pc_config->get(rtam::E_ID_SCENARIO);
    if(getenv("TEST_SET_MASTER") == NULL) {
      switch(_e_antenna_config) {
        case E_ANTENNA_CONFIG_ANTENNE1: {
          pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xa8);
          _CRIT << "SET MASTER";
          break;
        }
        case E_ANTENNA_CONFIG_ANTENNE2: {
          pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xac);
          _CRIT << "SET MASTER";
          break;
        }
        case E_ANTENNA_CONFIG_MAQUETTE16:
        case E_ANTENNA_CONFIG_MAQUETTE24:
        {
          pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0x82);
          _CRIT << "SET MASTER";
          break;
        }
        case E_ANTENNA_CONFIG_2XEVAL16: {
          pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(0xA1);
          _CRIT << "SET MASTER";
          break;
        }
        default: {
          M_BUG();
          break;
        }
      }
    } else {
      uint32_t i_master = atoi(getenv("TEST_SET_MASTER"));
      pc_node->get(rtam::E_ID_TRIGGER_MASTER)->set_data<uint64_t>(i_master);
      _CRIT << "SET MASTER";
    }
    _CRIT << "SET MASTER: " <<std::hex << pc_node->get(rtam::E_ID_TRIGGER_MASTER)->get_data<uint64_t>() ;
  }

	auto pc_node = pc_config->get(rtam::E_ID_SCENARIO);
  float f_duration = 1.0;

	uint32_t i_decim = pc_node->get(rtam::E_ID_DECIMATE)->get_data<uint16_t>();
  uint32_t i_nb_sample = f_duration*_f_fs/float(i_decim);
  uint32_t i_pulse_length = 100+10*in_i_id_test;
  uint32_t i_freq_start = 60e3;
  uint32_t i_freq_end = 80e3;
  uint32_t i_amplitude = 70;

  if(in_i_id_test&0x1) {
    i_freq_start = 80e3;
    i_freq_end = 60e3;
  }


	pc_node->get(rtam::E_ID_TRIGGER_DURATION)->set_data<double>(f_duration*1.2);

	pc_node->get(rtam::E_ID_NB_SAMPLE)->set_data<uint32_t>(round(i_nb_sample/124)*124);

  if (getenv("TEST_NO_TX") == NULL) {

    /* Antenna Pulse */
		for(uint8_t i_tx = 0; i_tx < _i_rtam_nb_tx; i_tx++)
    {
		auto pc_pulse = pc_node->get(rtam::E_ID_PULSE, i_tx);
		pc_pulse->get(rtam::E_ID_AFFINITY_SYSTEM)->set_data<uint64_t>(1 << (i_tx+_i_rtam_affinity_tx_offset));
		pc_pulse->get(rtam::E_ID_PULSER_MODE)->set_data<uint8_t>(rtam::E_PULSE_HFM);
		pc_pulse->get(rtam::E_ID_PULSER_FSTART)->set_data<float>(i_freq_start);
		pc_pulse->get(rtam::E_ID_PULSER_FEND)->set_data<float>(i_freq_end);
		pc_pulse->get(rtam::E_ID_PULSER_DURATION)->set_data<double>(double(i_pulse_length)/1000.0);
		pc_pulse->get(rtam::E_ID_PULSER_AMPLITUDE)->set_data<double>(double(i_amplitude)/100.0);
    }

  }

  //TODO Generation du noeud de configuration de mise à jour
	M_ASSERT(pc_node->get(rtam::E_ID_NB_SAMPLE)->get_size() == sizeof(uint32_t));
	f_port_get(E_PORT_RTAM).f_send(pc_config);
  #endif
}



int CT_RTAM_EVAL::f_compute_noise_variance(CT_GUARD<CT_PORT_NODE> const & in_rpc_node){
  //in_rpc_node->display();
	if(in_rpc_node->has(rtam::E_ID_PING_INFO)) {
		auto pc_ping_info = in_rpc_node->get(rtam::E_ID_PING_INFO);
    _CRIT<<_BLUE<<"CALCUL DES BRUIT DES CAPTEURS";
		M_ASSERT(pc_ping_info->has(rtam::E_ID_NB_SAMPLE));
		M_ASSERT(pc_ping_info->has(rtam::E_ID_NB_BEAM));
		M_ASSERT(pc_ping_info->has(rtam::E_ID_COMPLEX));
		uint64_t i_nb_sample = pc_ping_info->get(rtam::E_ID_NB_SAMPLE)->get_data<uint32_t>();
		uint64_t i_nb_beam = pc_ping_info->get(rtam::E_ID_NB_BEAM)->get_data<uint32_t>();
		uint8_t e_complex = pc_ping_info->get(rtam::E_ID_COMPLEX)->get_data<uint8_t>();
		double f_scale = pc_ping_info->get(rtam::E_ID_SCALE)->get_data<double>();
		//double f_freq_cutoff = pc_ping_info->get(rtam::E_ID_FREQ_CUTOFF)->get_data<double>();
		if((i_nb_beam <= 256) && (e_complex == rtam::E_COMPLEX_FORMAT_TIME_INTERLEAVED)) {
      double2_t s_acc[i_nb_beam];
      double2_t s_acc_square[i_nb_beam];
      int16_t * pi_data = in_rpc_node->mmap<int16_t>();

      for(uint32_t i_beam=0; i_beam<i_nb_beam;i_beam++) {
        s_acc[i_beam].x = 0;
        s_acc[i_beam].y = 0;
        s_acc_square[i_beam].x = 0;
        s_acc_square[i_beam].y = 0;
      }

      /* Estimate MEAN value */
      for(uint32_t i_sample=0; i_sample<i_nb_sample;i_sample++) {
        for(uint32_t i_beam=0; i_beam<i_nb_beam;i_beam++) {
          double2_t s_tmp;
          s_tmp.x = ((double) *pi_data)*f_scale;
          pi_data++;
          s_tmp.y = ((double) *pi_data)*f_scale;
          pi_data++;

          s_acc[i_beam].x +=  s_tmp.x;
          s_acc[i_beam].y +=  s_tmp.y;
        }
      }
      for(uint32_t i_beam=0; i_beam<i_nb_beam;i_beam++) {
        s_acc[i_beam].x /= i_nb_sample;
        s_acc[i_beam].y /= i_nb_sample;
			}


      /* Estimate VAR value */
          pi_data = in_rpc_node->mmap<int16_t>();
          for(uint32_t i_sample=0; i_sample<i_nb_sample;i_sample++) {
            for(uint32_t i_beam=0; i_beam<i_nb_beam;i_beam++) {
              double2_t s_tmp;
              s_tmp.x = ((double) *pi_data)*f_scale;
              pi_data++;
              s_tmp.y = ((double) *pi_data)*f_scale;
              pi_data++;

              s_acc_square[i_beam].x += (s_tmp.x-s_acc[i_beam].x)*(s_tmp.x-s_acc[i_beam].x)+(s_tmp.y-s_acc[i_beam].y)*(s_tmp.y-s_acc[i_beam].y);
              s_acc_square[i_beam].y +=  0;
            }
          }
          for(uint32_t i_beam=0; i_beam<i_nb_beam;i_beam++) {

            s_acc_square[i_beam].x /= i_nb_sample;
            s_acc_square[i_beam].y /= i_nb_sample;


            //_DBG << _V(f_scale) <<_V(s_acc[i_beam].x) <<_V(s_acc[i_beam].y) <<_V(s_acc_square[i_beam].x) <<_V(s_acc_square[i_beam].y);
          }

          /** Fill noise status */

          {
            //uint32_t i_uid = pc_uid->get_data<uint32_t>();
            //_m_noise[i_uid].resize(i_nb_beam);

            //for(uint32_t i_beam=0; i_beam<i_nb_beam;i_beam++) {
              //_DBG << _V(s_acc[i_beam].x) << _V(s_acc[i_beam].x) << _V(s_acc_square[i_beam].x) << _V(s_acc_square[i_beam].x);
              //double2_t s_tmp = s_acc_square[i_beam];

              /* Compute variance */
              //s_tmp.x -= s_acc[i_beam].x*s_acc[i_beam].x+s_acc[i_beam].y*s_acc[i_beam].y;
              //s_tmp.y -= 2*s_acc[i_beam].y*s_acc[i_beam].x;

              /* Compute absolute value*/
              //_m_noise[i_uid][i_beam] = 20*log10(sqrt(s_tmp.x))-10*log10(2*f_freq_cutoff);
            //}
          }
          //_DBG << _V(f_freq_cutoff);

        }
    }
  return EC_SUCCESS;
}
