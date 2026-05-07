/***********************************************************************
 ** rtam-eval_record.cc
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
 * @file rtam-eval_record.cc
 * Record part of Plugin RTAM_EVAL.
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @date 2018
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <rtam-eval.hh>
#include "api.hh"
using namespace std;
using namespace master::plugins::rtam_eval;


const char * CT_RTAM_EVAL::f_record_print_state(enum ET_RECORD_STATE const in_e_state) {
  switch(in_e_state) {
    case E_RECORD_STATE_IDLE:
      return "IDLE";
    case E_RECORD_STATE_RUNNING:
      return "RUN ";
    case E_RECORD_STATE_STARTING:
      return "STAR";
    case E_RECORD_STATE_STOPPING:
      return "STOP";
    default:
      return "UNKN";
  }
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

int CT_RTAM_EVAL::f_record_fsm() {
  switch(_e_record_state_current) {
    case E_RECORD_STATE_IDLE: {
      switch(_e_record_state_wanted) {
        case E_RECORD_STATE_RUNNING:
          _e_record_state_current = E_RECORD_STATE_RUNNING;//E_RECORD_STATE_STARTING;
          _str_record_file_current = _str_record_file_wanted;
          f_record_start();
          break;
        default:
          break;
      }
      break;
    }
    case E_RECORD_STATE_RUNNING: {
      switch(_e_record_state_wanted) {
        case E_RECORD_STATE_IDLE:
          _e_record_state_current = E_RECORD_STATE_STOPPING;
          _str_record_file_current = "";
          f_record_stop();
          break;
        default:
          break;
      }
      break;
    }
    case E_RECORD_STATE_STARTING:
		case E_RECORD_STATE_STOPPING:
		{
			if(M_ABS(f_get_time_ns64() - _i_rtam_timeout_start) > C_TIMEOUT_CONFIGURE_SECS*1e9) {
        _e_record_state_current = E_RECORD_STATE_ERROR;
        _i_record_error_cnt++;
      }
      break;
    }
    case E_RECORD_STATE_ERROR: {
      switch(_e_record_state_wanted) {
        case E_RECORD_STATE_RUNNING:
        case E_RECORD_STATE_IDLE:
				{
        /* On Error going back to IDLE */
        _e_record_state_current = E_RECORD_STATE_IDLE;
        _str_record_file_current = "";
        f_record_stop();
        break;
				}
        default:
        break;
      }
      break;
    }
  }
  return EC_SUCCESS;
}

int CT_RTAM_EVAL::f_record_check_config(std::string const & in_rstr_file) {
  //WARN("config record Raw begin , in_str_file = %s",in_rstr_file.c_str());
  switch(_e_record_state_current) {
    case E_RECORD_STATE_IDLE: {
      if(in_rstr_file.size()) {
        _e_record_state_wanted = E_RECORD_STATE_RUNNING;
        _str_record_file_wanted = in_rstr_file;
      }
      break;
    }
    case E_RECORD_STATE_ERROR:
		case E_RECORD_STATE_RUNNING:
		{
        if(!in_rstr_file.size()) {
          _e_record_state_wanted = E_RECORD_STATE_IDLE;
          _str_record_file_wanted = "";
        } else if(in_rstr_file != _str_record_file_current) {
          _e_record_state_wanted = E_RECORD_STATE_RUNNING;
          _str_record_file_wanted = in_rstr_file;
        }
			break;
    }
    default:
      break;
  }
  //WARN("RECORD :: current = %s ; wanted = %s",f_record_print_state(_e_record_state_current),f_record_print_state(_e_record_state_wanted));
  return EC_SUCCESS;
}


