/***********************************************************************
 ** config.cc
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
 * @file config.cc
 * Class de configuration.
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 *
 * @date 2018
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/
#include "config.hh"
#include <rtam/api.hh>

namespace rtam = master::plugins::fec;

/***********************************************************************
 * Namespace
 ***********************************************************************/
using namespace master::plugins::rtam_eval;

/***********************************************************************
 * Types
 ***********************************************************************/

CT_RTAM_EVAL_CONFIG::CT_RTAM_EVAL_CONFIG(
		CT_GUARD<CT_PORT_NODE> const & in_pc_node
)
{
  _pc_node = in_pc_node;
  _e_antenna_mode = E_ANTENNA_OFF;
  if(_pc_node->has(E_ID_CONFIG_ANTENNA_MODE)) {
    _e_antenna_mode =  (ET_ANTENNA_MODE)_pc_node->get(E_ID_CONFIG_ANTENNA_MODE)->get_data<uint8_t>();
  }
}

/* Etat ON/OFF du plot */
bool CT_RTAM_EVAL_CONFIG::f_is_up() {

  return true ;
	//return _b_active;
}

/* Etat ON/OFF du plot */
ET_ANTENNA_MODE CT_RTAM_EVAL_CONFIG::f_get_antenna_mode() {
  return _e_antenna_mode ;
	//return _b_active;
}
