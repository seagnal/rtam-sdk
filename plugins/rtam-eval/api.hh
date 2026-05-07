/***********************************************************************
 ** event.hh
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
#ifndef RTAM_EVAL_AGGREGATE_EVENT_HH_
#define RTAM_EVAL_AGGREGATE_EVENT_HH_

/**
 * @file event.hh
 * Evenements du plugin RTAM_EVAL Aggregate.
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @date 2018
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <c/common.h>
#include <event_base.hh>

/* string is needed for struct generation with swig */
#include <string>
/***********************************************************************
 * Defines
 ***********************************************************************/
#ifndef ID_DEFINITION
#include <rtam-eval/api.autogen.hh>
#endif

namespace master {
namespace plugins {
namespace rtam_eval {

#ifdef ID_DEFINITION
enum ET_ID_RTAM_EVAL {
 E_ID_CONFIG_READER_RAW_BASE_NAME,
  E_ID_CONFIG_RECORD_RAW_BASE_NAME,
  E_ID_CONFIG_RX_BFU,
  E_ID_CONFIG_RX_BFU_RUN,
  E_ID_CONFIG_ANTENNA_MODE,
  
  /* HardWare Status */
  E_ID_OTHER_STATUS,
  E_ID_STATUS_ZDA,
  E_ID_STATUS_PPS,
  E_ID_STATUS_INS,
  E_ID_STATUS_FLOW,
  /* Threshold */
  E_ID_THRESHOLD_BFU,
    /* status distribution */
  E_ID_STATUS_BFU,
  E_ID_STATUS_HW,
  E_ID_STATUS_HW_VERSION,
  E_ID_STATUS_HW_SHT,
  E_ID_STATUS_HW_VOLTAGE_VALUE,
  E_ID_STATUS_HW_SHT_TEMPERATURE,
  E_ID_STATUS_HW_SHT_HUMIDITY,
  E_ID_STATUS_HW_SHT_NAME,
  E_ID_STATUS_HW_VOLTAGE,
  E_ID_STATUS_HW_VOLTAGE_NAME,
  E_ID_STATUS_HW_PA_ALARM,
  /* -- status FSM */
  E_ID_STATE_CURRENT_RTAM,
  E_ID_STATE_WANTED_RTAM,
  E_ID_STATE_CURRENT_BFU,
  E_ID_STATE_WANTED_BFU,
  E_ID_STATE_CURRENT_READER,
  E_ID_STATE_WANTED_READER,
  E_ID_STATE_CURRENT_RECORD,
  E_ID_STATE_WANTED_RECORD,
  E_ID_STATE_CURRENT_AUDIO,
  E_ID_STATE_WANTED_AUDIO,
  /* antenna and hydro consigne */
  E_ID_CONSIGNE_ANTENNA_DB,
  E_ID_CONSIGNE_HYDRO_DB,
  /* setting freq sampling */
  E_ID_CONFIG_RECORD_AUDIO_FREQ_SAMPLING,
  /* Audio wav config */
  E_ID_CONFIG_READER_MODE,
  E_ID_CONFIG_RECORD_MODE,
  E_ID_CONFIG_UPDATE,
  /* status load cpu and disk*/
  E_ID_BFU_REMAINING_DISK_RAW,
  E_ID_BFU_REMAINING_DISK_ROOT,
  E_ID_BFU_REMAINING_DISK_TMP,
  E_ID_BFU_REMAINING_DISK_LOG,
  E_ID_BFU_FREE_MEMORY,
  E_ID_BFU_LOAD_CPU,
  E_ID_BFU_NTPSTAT,
  E_ID_STATUS_MODE_ANTENNA,
  /* boards provided */
  E_ID_BOARDS_PROVIDED,
  E_ID_SYNC_DISTRI_BJ,
  E_ID_SYNC_DISTRI_ATTER,
  /* status information */
  E_ID_BFU_READER_PERCENT,
  E_ID_RTAM_LOST_CNT,
  E_ID_RTAM_ERROR_CNT,
  E_ID_RTAM_MISSING_CNT,
  E_ID_BFU_TIME_PROCESSING,
  E_ID_BFU_FALSE_TRACK_EMIT,
  E_ID_LOOP_SERVERS_TIME_PROCESSING,
  /*callback to receive E_ID_PING */
  E_ID_CALLBACK_PING,
};
#endif

enum ET_RTAM_STATE {
  E_RTAM_STATE_OFF = 0,
  E_RTAM_STATE_IDLE,
  E_RTAM_STATE_TEST,
  E_RTAM_STATE_RUNNING,
  E_RTAM_STATE_CONFIGURED,
  E_RTAM_STATE_STOPPING,
  E_RTAM_STATE_INITIALIZING,
  E_RTAM_STATE_CONFIGURING,
  E_RTAM_STATE_UPDATING,
  E_RTAM_STATE_ERROR,
};
enum ET_BFU_STATE {
  E_BFU_STATE_IDLE = 0,
  E_BFU_STATE_RUNNING,
  E_BFU_STATE_STARTING,
  E_BFU_STATE_STOPPING,
  E_BFU_STATE_UPDATING,
  E_BFU_STATE_ERROR,
};
enum ET_RECORD_STATE {
  E_RECORD_STATE_IDLE = 0,
  E_RECORD_STATE_RUNNING,
  E_RECORD_STATE_STARTING,
  E_RECORD_STATE_STOPPING,
  E_RECORD_STATE_ERROR,
};

enum ET_READER_STATE {
  E_READER_STATE_IDLE = 0,
  E_READER_STATE_RUNNING,
  E_READER_STATE_STARTING,
  E_READER_STATE_STOPPING,
  E_READER_STATE_ERROR,
};

enum ET_AUDIO_STATE {
  E_AUDIO_STATE_INIT = 0,
  E_AUDIO_STATE_CONNECTING,
  E_AUDIO_STATE_IDLE,
  E_AUDIO_STATE_STARTING,
  E_AUDIO_STATE_RUNNING,
  E_AUDIO_STATE_ERROR,
};

enum ET_RECORD_MODE {
  E_RECORD_OFF = 0,
  E_RECORD_FUSION,
  E_RECORD_RAW,
};

enum ET_READER_MODE {
  E_READER_OFF = 0,
  E_READER_FUSION,
  E_READER_RAW,
};

enum ET_ANTENNA_MODE {
  E_ANTENNA_AUTO = 0,
  E_ANTENNA_OFF,
};

enum ET_HYDRO_MODE {
  E_HYDRO_OFF,
  E_HYDRO_ANTENNA,
  E_HYDRO_ALONE,
  E_HYDRO_HF,
};

struct ST_CONFIG_PELAGOS{
  uint32_t i_level_min;
  uint32_t i_level_max;
  uint32_t i_step_lvl;
  float f_step_time;
};

struct ST_MAX_DEVICE{
  uint32_t i_level_max_hydro;
  uint32_t i_level_max_antenne;
};

struct ST_CONFIG_ANTENNA{
  float f_freq_start;
  float f_freq_end;
  float f_duration;
  float f_level;
  float f_freq_combo;
};


struct ST_CONFIG_TX{
  ST_CONFIG_ANTENNA as_config_antenna[6];
  float f_time;
  bool b_active;
  float f_hydro_level;
  std::string str_hydro_mode;
};
struct ST_CONFIG_RX{
  float f_time_duration_rx;
};

struct ST_CONFIG_POD{
  ST_CONFIG_TX s_config_tx;
  ST_CONFIG_RX s_config_rx;
  ET_ANTENNA_MODE e_antenna_mode;
  float f_recurrence_system;
};



}
}

}

#endif /* RTAM_EVAL_AGGREGATE_EVENT_HH_ */
