/***********************************************************************
 ** config.hh
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
#ifndef RTAM_EVAL_CONFIG_HH_
#define RTAM_EVAL_CONFIG_HH_

/**
 * @file config.hh
 * Class de conversion de la configuration Systeme à la configuration
 * plot.
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @author SEAGNAL (luc.remont@seagnal.fr)
 * @date 2018
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <host.hh>
#include <rtam-eval/api.hh>

#define C_MAX_DB_ANTENNA (220)
#define C_MAX_DB_HYDRO (190)
#define C_SIZE_TABLE (64)
#define C_MIN_TABLE (46e3)
#define C_STEP_TABLE (1e3)
#define C_FREQ_CLK (125e6)
/***********************************************************************
* Types
***********************************************************************/

namespace master {
namespace plugins {
namespace rtam_eval {

class CT_RTAM_EVAL_CONFIG {
  /* Noeud de configuration */
  CT_GUARD<CT_PORT_NODE> _pc_node;

  ET_ANTENNA_MODE _e_antenna_mode;

public:
  CT_RTAM_EVAL_CONFIG(
    CT_GUARD<CT_PORT_NODE> const & in_pc_node
  );
  /* Etat ON/OFF du plot */
  bool f_is_up();
  ET_ANTENNA_MODE f_get_antenna_mode();
};

}
}
}
#endif /* RTAM_EVAL_CONFIG_HH_ */
