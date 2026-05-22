/***********************************************************************
 ** main.cc
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
 * @file main.cc
 * Main function of BML parser.
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @date 2021
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <c/common.h>
#include <c/debug.h>
#include <c/third_sha256.h>
#include <cpp/debug.hh>
#include <cpp/string.hh>
#include <bml_node.hh>

#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>
#include <experimental/filesystem>

#ifdef FF_MATIO
#include <matio.h>
#endif

/***********************************************************************
 * Defines
 ***********************************************************************/
enum ET_MODE {
  E_MODE_ANALYSIS=0x00,
  E_MODE_DISPATCH=0x01,
  E_MODE_MATIO=0x02,
};
/***********************************************************************
 * Types
 ***********************************************************************/


 using namespace std;
 using namespace boost::program_options;
 namespace fs = std::experimental::filesystem;

#if defined(_WIN32)
inline void sync() {};
#endif


const char * f_print_available(uint8_t const & in) {
  switch(in) {
    case 0:
      return "NO";
      break;
    case 255:
      return "Unknown";
      break;
    default:
      return "YES";
      break;
  }
}

std::ofstream goutfile;
int f_debug_va_local(void * in_v_arg, time_t in_s_time, const char * in_str_file,
                     int in_i_line, const char * in_str_func, enum ET_DEBUG_TYPE in_e_level,
                     const char *in_str_format, va_list in_s_ap) {

  FILE * fd;
  char str_buffer[4096];
  int i_nb_char = 0;
  std::string pc_type;

  switch (in_e_level) {
  case E_DEBUG_TYPE_FATAL:
    pc_type = "FATAL ERROR";
    fd = stderr;
    break;
  case E_DEBUG_TYPE_CRITICAL:
    pc_type = "CRITICAL ERROR";
    fd = stderr;
    break;
  case E_DEBUG_TYPE_WARNING:
    pc_type = "WARN";
    fd = stderr;
    break;
  case E_DEBUG_TYPE_IMP:
    pc_type = "IMP";
    fd = stdout;
    break;
  case E_DEBUG_TYPE_GOOD:
    pc_type = "GOOD";
    fd = stdout;
    break;
  case E_DEBUG_TYPE_INFO:
    pc_type = "INFO";
    fd = stdout;
    break;
  default:
    M_ABORT();
    break;
  }

  /* print file info */
  i_nb_char += snprintf(&str_buffer[i_nb_char],
      sizeof(str_buffer) - i_nb_char, "%s:", pc_type.c_str());

  /* print format with args */
  i_nb_char += vsnprintf(&str_buffer[i_nb_char],
      sizeof(str_buffer) - i_nb_char, in_str_format, in_s_ap);


  /* print new line */
  goutfile << str_buffer << std::endl;

  return 0;
}


struct ST_DEBUG gs_debug_local = { NULL, f_debug_va_local, NULL };



 std::string gstr_none = "";
 struct ST_STATS {
   uint64_t i_first_time;
   uint64_t i_last_time;

   uint64_t i_max_diff_time;
   uint64_t i_cnt;
   uint64_t i_size_acc;
   uint64_t i_size;

 public:
   ST_STATS() {
     i_first_time = 0;
     i_last_time = 0;
     i_max_diff_time = 0;
     i_cnt = 0;
     i_size_acc = 0;
     i_size = 0;
   }

   virtual ~ST_STATS() = default;
 };

 class CT_DBNAME {
   /*!
   * BML ID database.
   * Database used for ID checking.
   */
   std::map<uint32_t, std::string> _m_id_db;
   std::map<std::string, uint32_t> _m_id_inv_db;

 public:
   CT_DBNAME (std::string const & in_str_file) {
     if(in_str_file.size()) {
       try {
         _DBG << "Loading "<< in_str_file;
         std::ifstream file(in_str_file);
         std::string str;
         while (std::getline(file, str)) {
         str = f_string_replace(str,"</key><value>","=");
         str = f_string_replace(str,"<id><key>","");
         str = f_string_replace(str,"</value></id>","");
         //_DBG << str << "\n";
         auto v_str = f_string_split(str,"=");
         if(v_str.size() == 2) {
           std::istringstream converter(v_str[1]);
           uint32_t i_value;
           converter >> std::hex >> i_value;
           //_DBG << _V(i_value) << _V(v_str[0]);
           _m_id_db[i_value] = v_str[0];
           _m_id_inv_db[v_str[0]] = i_value;
         }
         }
         _DBG << "Found "<<_m_id_db.size() << " nodes";
       } catch(...) {
         _CRIT << "Unable to open db file:" << in_str_file;
       }
     }

       //
   }

   std::string const & f_id_name(uint32_t in_i_id) {
    auto pc_it = _m_id_db.find(in_i_id);
    if(pc_it != _m_id_db.end()) {
     return pc_it->second;
    } else {
     //_CRIT << "ID "<< std::hex << in_i_id << " does not exists";
     _m_id_db[in_i_id] = f_string_format("0x%08X", in_i_id);
     return _m_id_db[in_i_id];
    }
   }

   uint32_t const f_name2id(std::string const & in_i_str, bool in_b_no_error = false) {
    auto pc_it = _m_id_inv_db.find(in_i_str);
    if(pc_it != _m_id_inv_db.end()) {
     return pc_it->second;
    } else {
        _WARN << "ID "<< in_i_str << " not found in database, fallback to generaton";
        //            std::string str_tmp = f_string_format("echo -n %s | sha256sum | cut -c1-8", in_i_str.c_str());
        //            std::string str_id = f_misc_get_output(str_tmp);
        std::string str_id;
        {
            char buffer[65] = {0};
            sha256_easy_hash_hex(in_i_str.data(), in_i_str.size(), buffer);
            str_id = std::string(buffer, 8);
        }
        uint32_t i_id = std::stoul(str_id, 0, 16);
        _DBG << in_i_str << " = "<< std::hex << i_id;
        _m_id_inv_db[in_i_str] = i_id;
        _m_id_db[i_id] = in_i_str;
        return i_id;
    }
    }
};


class CT_ID {
protected:
    CT_DBNAME & _c_db;
public:
    CT_ID(CT_DBNAME & in_c_db) : _c_db(in_c_db) {

    }

    virtual ~CT_ID() {} // Pour dynamic_cast
    virtual uint32_t f_get_secondary_id(bml::node<uint32_t, std::shared_ptr> & in_rpc_node) {
        return 0;
    }
    virtual int f_analysis_handle_packet(bml::node<uint32_t, std::shared_ptr> & in_rpc_node, std::shared_ptr<ST_STATS> & in_ps_stat) {
        return EC_SUCCESS;
    }
    virtual void f_analysis_display_stats(std::shared_ptr<ST_STATS> & in_ps_stat) {

    }
    virtual std::shared_ptr<ST_STATS> f_analysis_new_stat(void) {
        return std::shared_ptr<ST_STATS>(new ST_STATS());
    }

    virtual int f_dispatch_handle_packet(
        bml::node<uint32_t, std::shared_ptr> & in_rpc_node,
        std::shared_ptr<ST_STATS> & in_ps_stat,
        std::string const & in_str_folder
        ) {
        return EC_SUCCESS;
    }
};



  struct ST_STATS_PING : public ST_STATS {
      uint32_t i_stat_sample_loss_cnt;
      uint32_t i_stat_sample_error_cnt;
      uint32_t i_stat_missing_trigger_cnt;
      uint32_t i_stat_missing_ring_cnt;
      uint32_t i_stat_reboot_cnt;
      uint32_t i_stat_hw_uid_misssing_cnt;
      uint32_t i_stat_ext_time_from_pps_error_cnt;
      uint32_t i_stat_ext_time_not_updated_cnt;
      uint32_t i_stat_ext_time_w_from_pps_unstable_cnt;
      uint32_t i_last_ext_time_w_from_pps;

      uint16_t i_last_trigger_uid;
      uint16_t i_last_ring_id;
      uint64_t i_last_time_error;
      uint64_t i_last_sample_loss;
      uint64_t i_last_ext_time;
      uint64_t i_last_ext_time_update;
      uint64_t i_last_nb_hw_uid;
      uint64_t i_last_prf;

      uint8_t i_last_data_format;
      uint8_t i_last_data_complex;
      uint32_t i_last_data_nb_sample;
      uint32_t i_last_data_nb_beam;
      double i_last_data_scale;
      double i_last_data_full_scale;
      double i_last_data_cutoff;
      double i_last_data_fdemod;

      uint8_t i_stat_ext_time_available;
      uint8_t i_stat_pps_available;

      ST_STATS_PING() : ST_STATS() {
        i_stat_sample_loss_cnt = 0;
        i_stat_sample_error_cnt = 0;
        i_last_trigger_uid = 0;
        i_last_ring_id = 0;
        i_stat_missing_trigger_cnt = 0;
        i_stat_missing_ring_cnt = 0;
        i_stat_reboot_cnt = 0;
        i_last_time_error = 0;
        i_last_sample_loss = 0;
        i_stat_hw_uid_misssing_cnt = 0;
        i_stat_ext_time_from_pps_error_cnt = 0;
        i_stat_ext_time_not_updated_cnt = 0;
        i_stat_ext_time_w_from_pps_unstable_cnt = 0;
        i_last_ext_time = 0;
        i_last_ext_time_update = 0;
        i_last_ext_time_w_from_pps = 0;
        i_last_prf = 0;
        i_last_nb_hw_uid = 0;
        i_stat_ext_time_available = 0;
        i_stat_pps_available = 0;
        i_last_data_format = 0;
        i_last_data_scale = 0;
        i_last_data_full_scale = 0;
        i_last_data_cutoff = 0;
        i_last_data_fdemod = 0;
        i_last_data_complex = 0;
        i_last_data_nb_beam = 0;
        i_last_data_nb_sample = 0;
      }
  };


class CT_DISPATCH_BML {
    std::list<std::pair<std::string, std::shared_ptr<bml::node_file_writer<std::shared_ptr>>>> _l_dispatch;

public:

    ~CT_DISPATCH_BML() {
        _l_dispatch.clear();
        _WARN << "Flushing all ";
    }

    std::shared_ptr<bml::node_file_writer<std::shared_ptr>> f_bml_open(std::string const & in_str_file) {
//        bool b_found = false;
        std::shared_ptr<bml::node_file_writer<std::shared_ptr>> pc_writer = NULL;

        for(auto & pc_pair : _l_dispatch) {
            if(pc_pair.first == in_str_file) {
                pc_writer = pc_pair.second;
            }
        }

        if(!pc_writer) {
            _WARN << "Creating "<< in_str_file;
            pc_writer = std::shared_ptr<bml::node_file_writer<std::shared_ptr>>(new bml::node_file_writer<std::shared_ptr>(in_str_file));
            auto c_pair = std::pair<std::string, std::shared_ptr<bml::node_file_writer<std::shared_ptr>>>(in_str_file, pc_writer);
            _l_dispatch.push_back(c_pair);
        }

        while(_l_dispatch.size() > 10) {
            _WARN << "Flushing "<<_l_dispatch.front().first;
            _l_dispatch.pop_front();
        }

        M_ASSERT(pc_writer);
        return pc_writer;
    }

};

class CT_ID_RAW :  public CT_ID {
    bool _b_text;

public:
    CT_ID_RAW(CT_DBNAME & in_c_db, bool in_b_text) : CT_ID(in_c_db) {
        _b_text = in_b_text;
    }

    virtual ~CT_ID_RAW() {} // Pour dynamic_cast

    int f_dispatch_handle_packet(
        bml::node<uint32_t, std::shared_ptr> & in_rpc_node,
        std::shared_ptr<ST_STATS> & in_ps_stat,
        std::string const & in_str_folder
        ) {

        uint32_t in_i_id = in_rpc_node.get_id();
        std::string str_name_id = _c_db.f_id_name(in_i_id);


        std::string str_name = f_string_format("%s/raw_%s.bin", in_str_folder.c_str(), f_string_replace(str_name_id,":","_").c_str());

        {
            ofstream appendFile(str_name, std::ofstream::out | ios_base::app);
            if (appendFile.fail()) {
                cerr << "Unable to open file for writing." << endl;
                sync();
                exit(1);
            }
            if(in_rpc_node.get_size()) {
                if(_b_text) {
                    std::string str_tmp(in_rpc_node.mmap<char>(), in_rpc_node.get_size());
                    appendFile << str_tmp << '\n';
                } else {
                    appendFile.write(in_rpc_node.mmap<char>(), in_rpc_node.get_size());
                }
                appendFile.close();
            }
        }

        return EC_SUCCESS;
    }
};

class CT_ID_PING :  public CT_ID {
    uint32_t _i_id_ping_info;
    uint32_t _i_id_ping_system_uid;
    uint32_t _i_id_ping_trigger_uid;
    uint32_t _i_id_ping_missing;
    uint32_t _i_id_sample_error;
    uint32_t _i_id_ping_ring_id;
    uint32_t _i_id_ext_time_from_pps;
    uint32_t _i_id_ext_time;
    uint32_t _i_id_hw_time;
    uint32_t _i_id_hw_uid;
    uint32_t _i_id_ping_data_scale;
    uint32_t _i_id_ping_data_full_scale;
    uint32_t _i_id_ping_data_cutoff;
    uint32_t _i_id_ping_data_fdemod;
    uint32_t _i_id_ping_data_format;
    uint32_t _i_id_ping_data_complex;
    uint32_t _i_id_ping_data_nb_beam;
    uint32_t _i_id_ping_data_nb_sample;
    uint32_t _i_id_ext_time_w_from_pps;

    CT_DISPATCH_BML _c_bml;

    std::map<uint32_t, std::ofstream> _mof_files;
public:
    CT_ID_PING(CT_DBNAME & in_c_db) : CT_ID(in_c_db) {
        _i_id_ping_info            = _c_db.f_name2id("master::plugins::fec::E_ID_PING_INFO");
        _i_id_ping_system_uid      = _c_db.f_name2id("master::plugins::fec::E_ID_SYSTEM_UID");
        _i_id_ping_trigger_uid     = _c_db.f_name2id("master::plugins::fec::E_ID_TRIGGER_UID");
        _i_id_ping_missing         = _c_db.f_name2id("master::plugins::fec::E_ID_SAMPLE_MISSING");
        _i_id_sample_error         = _c_db.f_name2id("master::plugins::fec::E_ID_SAMPLE_ERROR");
        _i_id_ping_ring_id         = _c_db.f_name2id("master::plugins::fec::E_ID_RING_ID");
        _i_id_ext_time_from_pps    = _c_db.f_name2id("master::plugins::fec::E_ID_EXT_TIME_FROM_PPS");
        _i_id_ext_time_w_from_pps  = _c_db.f_name2id("master::plugins::fec::E_ID_EXT_TIME_WITH_FROM_PPS");
        _i_id_ext_time             = _c_db.f_name2id("master::plugins::fec::E_ID_EXT_TIME");
        _i_id_hw_time              = _c_db.f_name2id("master::plugins::fec::E_ID_HW_TIME");
        _i_id_hw_uid               = _c_db.f_name2id("master::plugins::fec::E_ID_HW_UID");
        _i_id_ping_data_complex    = _c_db.f_name2id("master::plugins::fec::E_ID_COMPLEX");
        _i_id_ping_data_format     = _c_db.f_name2id("master::plugins::fec::E_ID_FORMAT");
        _i_id_ping_data_nb_sample  = _c_db.f_name2id("master::plugins::fec::E_ID_NB_SAMPLE");
        _i_id_ping_data_nb_beam    = _c_db.f_name2id("master::plugins::fec::E_ID_NB_BEAM");
        _i_id_ping_data_scale      = _c_db.f_name2id("master::plugins::fec::E_ID_SCALE");
        _i_id_ping_data_full_scale = _c_db.f_name2id("master::plugins::fec::E_ID_FULL_SCALE");
        _i_id_ping_data_cutoff     = _c_db.f_name2id("master::plugins::fec::E_ID_FREQ_CUTOFF");
        _i_id_ping_data_fdemod     = _c_db.f_name2id("master::plugins::fec::E_ID_FREQ_DEMOD");

    }

    virtual ~CT_ID_PING() {} // Pour dynamic_cast

    uint32_t f_get_secondary_id(bml::node<uint32_t, std::shared_ptr> & in_rpc_node){
        try {
            return in_rpc_node.get(_i_id_ping_info)->get(_i_id_ping_system_uid)->get_data<uint32_t>();
        } catch (...) {
            return 0;
        }
    }

#if 0
int f_analysis_handle_packet(bml::node<uint32_t, std::shared_ptr> & in_rpc_node, std::shared_ptr<ST_STATS> & in_ps_stat) {
    if(in_rpc_node.has(_i_id_ping_info)) {
      auto pc_node = in_rpc_node.get(_i_id_ping_info);

      if(pc_node->has(_i_id_ping_data_complex)) {
        ps_stats->i_stat_ext = pc_node->get(_i_id_ping_data_complex)->get_data<int32_t>()
      }
    }
#endif

    int f_analysis_handle_packet(bml::node<uint32_t, std::shared_ptr> & in_rpc_node, std::shared_ptr<ST_STATS> & in_ps_stat) {
        auto ps_stats =  dynamic_cast<ST_STATS_PING*>(in_ps_stat.get());
        uint16_t i_trigger_uid = -1;

        //try {
        if(in_rpc_node.has(_i_id_ping_trigger_uid)) {
            i_trigger_uid = in_rpc_node.get(_i_id_ping_trigger_uid)->get_data<uint64_t>();
            int16_t i_diff =  int16_t(i_trigger_uid)-int16_t(ps_stats->i_last_trigger_uid);

            if(i_trigger_uid != ps_stats->i_last_trigger_uid) {
                ps_stats->i_last_ring_id = 0;
            }

            if(i_diff < 0 && (i_trigger_uid < 10)) {
                _CRIT << "Detected REBOOT" << _V(i_diff) << _V(ps_stats->i_last_time);
                ps_stats->i_stat_reboot_cnt ++;
            } else if(i_diff > 1) {
                _CRIT << "Missing Trigger" << _V(f_get_secondary_id(in_rpc_node)) << _V(ps_stats->i_last_time) << _V(i_diff);
                ps_stats->i_stat_missing_trigger_cnt ++;
                ps_stats->i_last_time_error = ps_stats->i_last_time;
            }
            ps_stats->i_last_trigger_uid = i_trigger_uid;

        } else {
            _CRIT << "Missing Trigger UID" << _V(f_get_secondary_id(in_rpc_node))<< _V(ps_stats->i_last_time);
            ps_stats->i_stat_reboot_cnt = -1;
            ps_stats->i_stat_missing_trigger_cnt = -1;
        }

        if(in_rpc_node.has(_i_id_ping_ring_id)) {
            uint16_t i_ring_id = in_rpc_node.get(_i_id_ping_ring_id)->get_data<int16_t>();
            int16_t i_diff =  int16_t(i_ring_id)-int16_t(ps_stats->i_last_ring_id);
            if(i_diff > 1) {
                ps_stats->i_stat_missing_ring_cnt ++;
                ps_stats->i_last_time_error = ps_stats->i_last_time;
                _CRIT << "Missing Ring" << _V(f_get_secondary_id(in_rpc_node)) << _V(i_diff) << _V(ps_stats->i_last_time);
            }
            ps_stats->i_last_ring_id = i_ring_id;
        } else {
            ps_stats->i_stat_missing_ring_cnt = -1;
        }

        if(in_rpc_node.has(_i_id_ping_info)) {
            auto pc_node = in_rpc_node.get(_i_id_ping_info);

            if(pc_node->has(_i_id_ping_data_complex)) {
                ps_stats->i_last_data_complex = pc_node->get(_i_id_ping_data_complex)->get_data<uint8_t>();
            }

            if(pc_node->has(_i_id_ping_data_format)) {
                uint8_t i_format = pc_node->get(_i_id_ping_data_format)->get_data<uint8_t>();
                switch(i_format) {
                case 0:
                    ps_stats->i_last_data_format = 1;
                    break;
                case 1:
                    ps_stats->i_last_data_format = 2;
                    break;
                case 2:
                    ps_stats->i_last_data_format = 4;
                    break;
                }
            }

            if(pc_node->has(_i_id_ping_data_nb_beam)) {
                ps_stats->i_last_data_nb_beam = pc_node->get(_i_id_ping_data_nb_beam)->get_data<uint32_t>();
            }

            if(pc_node->has(_i_id_ping_data_nb_sample)) {
                ps_stats->i_last_data_nb_sample = pc_node->get(_i_id_ping_data_nb_sample)->get_data<uint32_t>();
            }

            if(pc_node->has(_i_id_ping_data_scale)) {
                ps_stats->i_last_data_scale = pc_node->get(_i_id_ping_data_scale)->get_data<double>();
            }

            if(pc_node->has(_i_id_ping_data_full_scale)) {
                ps_stats->i_last_data_full_scale = pc_node->get(_i_id_ping_data_full_scale)->get_data<double>();
            }

            if(pc_node->has(_i_id_ping_data_cutoff)) {
                ps_stats->i_last_data_cutoff = pc_node->get(_i_id_ping_data_cutoff)->get_data<double>();
            }

            if(pc_node->has(_i_id_ping_data_fdemod)) {
                ps_stats->i_last_data_fdemod = pc_node->get(_i_id_ping_data_fdemod)->get_data<double>();
            }

            if(pc_node->has(_i_id_ping_missing)) {
                if(pc_node->get(_i_id_ping_missing)->get_data<int32_t>()) {
                    ps_stats->i_stat_sample_loss_cnt++;
                    ps_stats->i_last_time_error = ps_stats->i_last_time;
                    _CRIT << "Sample Missing" << _V(i_trigger_uid)<< _V(f_get_secondary_id(in_rpc_node)) <<_V(pc_node->get(_i_id_ping_missing)->get_data<int32_t>());
                    ps_stats->i_last_sample_loss = true;
                } else {
                    ps_stats->i_last_sample_loss = false;
                }
            } else {
                ps_stats->i_stat_sample_loss_cnt = -1;
                ps_stats->i_last_sample_loss = true;
            }

            if(pc_node->has(_i_id_sample_error)) {
                if(pc_node->get(_i_id_sample_error)->get_data<int32_t>()) {
                    ps_stats->i_stat_sample_error_cnt++;
                    ps_stats->i_last_time_error = ps_stats->i_last_time;
                    _CRIT << "Sample Loss" << _V(i_trigger_uid)<< _V(f_get_secondary_id(in_rpc_node))<<_V(pc_node->get(_i_id_sample_error)->get_data<int32_t>());
                }
            } else {
                ps_stats->i_stat_sample_error_cnt = -1;
            }

            /* HW UID */
            if(pc_node->has(_i_id_hw_uid)) {
                auto v_uids = pc_node->find(_i_id_hw_uid);
                if (ps_stats->i_last_nb_hw_uid) {
                    if (ps_stats->i_last_nb_hw_uid != v_uids.size()) {
                        ps_stats->i_stat_hw_uid_misssing_cnt++;
                        ps_stats->i_last_time_error = ps_stats->i_last_time;
                    }
                }
                if(v_uids.size() > ps_stats->i_last_nb_hw_uid) {
                    ps_stats->i_last_nb_hw_uid = v_uids.size();
                }

                if(v_uids.size()) {
                    auto pc_hw_uid = v_uids[0]->second;

                    uint64_t i_ext_time_w_from_pps = 0;
                    uint64_t i_ext_time_from_pps = 0;
                    uint64_t i_ext_time = 0;
                    uint64_t i_hw_time = 0;

                    if(pc_hw_uid->has(_i_id_ext_time_from_pps)) {
                        i_ext_time_from_pps = pc_hw_uid->get(_i_id_ext_time_from_pps)->get_data<uint64_t>();
                    }
                    if(pc_hw_uid->has(_i_id_ext_time_w_from_pps)) {
                        i_ext_time_w_from_pps = pc_hw_uid->get(_i_id_ext_time_w_from_pps)->get_data<uint64_t>();
                    }
                    if(pc_hw_uid->has(_i_id_ext_time)) {
                        i_ext_time = pc_hw_uid->get(_i_id_ext_time)->get_data<uint64_t>();
                    }
                    if(pc_hw_uid->has(_i_id_hw_time)) {
                        i_hw_time = pc_hw_uid->get(_i_id_hw_time)->get_data<uint64_t>();
                    }
                    /*if(f_get_secondary_id(in_rpc_node) == 0) {
                _DBG << _V(i_trigger_uid)<< _V(f_get_secondary_id(in_rpc_node)) << _V(i_ext_time_from_pps) << _V(i_ext_time) << _V(i_hw_time) << _V(pc_hw_uid->get(_i_id_ext_time)->get_size()) <<std::hex<< _V(pc_hw_uid->get_data<uint32_t>()) << _V(_i_id_ext_time);
              }*/
                    if(i_ext_time_from_pps > 125e6*1.1) {
                        if(i_ext_time) {
                            _CRIT << "Ext time form pps is above 1s"  << _V(i_trigger_uid)<< _V(f_get_secondary_id(in_rpc_node)) << _V(i_ext_time_from_pps);
                            ps_stats->i_stat_ext_time_from_pps_error_cnt++;
                            ps_stats->i_last_time_error = ps_stats->i_last_time;
                        }
                        ps_stats->i_stat_pps_available = 0;
                    } else {
                        ps_stats->i_stat_pps_available = 1;
                    }

                    if(ps_stats->i_last_ext_time) {
                        if(i_ext_time != ps_stats->i_last_time) {
                            if(ps_stats->i_last_ext_time_update) {
                                int64_t i_diff = int64_t(i_hw_time) - int64_t(ps_stats->i_last_ext_time_update);
                                if(i_diff > 125e6*1.1) {
                                    _CRIT << "ext time not updated for 1s"  << _V(i_trigger_uid)<< _V(f_get_secondary_id(in_rpc_node)) << _V(i_diff) << _V(i_hw_time) << _V(ps_stats->i_last_ext_time_update);
                                    ps_stats->i_stat_ext_time_not_updated_cnt++;
                                    ps_stats->i_last_time_error = ps_stats->i_last_time;
                                }
                            }

                            ps_stats->i_last_ext_time_update = i_hw_time;
                        }
                    }

                    if(ps_stats->i_last_ext_time_w_from_pps) {
                        int64_t i_prf = int64_t(i_ext_time_w_from_pps) - int64_t(ps_stats->i_last_ext_time_w_from_pps);
                        if(ps_stats->i_last_prf) {
                            if((i_prf-ps_stats->i_last_prf) > 0.1 * ps_stats->i_last_prf) {
                                _CRIT << "Prf is unstable"  << _V(i_trigger_uid)<< _V(f_get_secondary_id(in_rpc_node)) << _V(i_prf) << _V(ps_stats->i_last_prf);
                                ps_stats->i_stat_ext_time_w_from_pps_unstable_cnt++;
                                ps_stats->i_last_time_error = ps_stats->i_last_time;
                            }
                        }
                        ps_stats->i_last_prf = i_prf;
                    }

                    if(i_ext_time_w_from_pps) {
                        ps_stats->i_last_ext_time_w_from_pps = i_ext_time_w_from_pps;
                    }
                    if(i_ext_time) {
                        ps_stats->i_last_ext_time = i_ext_time;

                        if (ps_stats->i_stat_pps_available) {
                            ps_stats->i_stat_ext_time_available = 1;
                        } else {
                            ps_stats->i_stat_ext_time_available = -1;
                        }
                    } else {
                        if (ps_stats->i_stat_pps_available) {
                            ps_stats->i_stat_ext_time_available = 0;
                        } else {
                            ps_stats->i_stat_ext_time_available = -1;
                        }
                    }
                }
            }
        } else {
            _CRIT << "Missing E_ID_PING_INFO";
            ps_stats->i_stat_sample_error_cnt = -1;
            ps_stats->i_stat_sample_loss_cnt = -1;
            ps_stats->i_stat_missing_ring_cnt = -1;
            ps_stats->i_stat_missing_trigger_cnt = -1;
            ps_stats->i_stat_reboot_cnt = -1;
        }
        return EC_SUCCESS;

    }
    void f_analysis_display_stats(std::shared_ptr<ST_STATS> & in_ps_stat) {
        auto ps_stats =  dynamic_cast<ST_STATS_PING*>(in_ps_stat.get());
        std::string str_tmp;
        if(ps_stats->i_last_time_error) {
            f_string_human_readable_time(str_tmp, ps_stats->i_last_time-ps_stats->i_last_time_error);
        } else {
            str_tmp = "N/A";
        }

        //_DBG << " - nb node:        " << s_res.i_cnt ;
        _DBG << " - reboot cnt:      " << ps_stats->i_stat_reboot_cnt ;
        _DBG << " - sample loss:     " << ps_stats->i_stat_sample_loss_cnt ;
        _DBG << " - error cnt:       " << ps_stats->i_stat_sample_error_cnt ;
        _DBG << " - missing trigger: " << ps_stats->i_stat_missing_trigger_cnt ;
        _DBG << " - missing ring:    " << ps_stats->i_stat_missing_ring_cnt ;
        _DBG << " - missing hw uid:  " << ps_stats->i_stat_hw_uid_misssing_cnt ;
        _DBG << " - pps:             " << f_print_available(ps_stats->i_stat_pps_available);
        _DBG << " - ext_time:        " << f_print_available(ps_stats->i_stat_ext_time_available);
        _DBG << " - ovf exttime pps: " << ps_stats->i_stat_ext_time_from_pps_error_cnt ;
        _DBG << " - ovf exttime upd: " << ps_stats->i_stat_ext_time_not_updated_cnt ;
        _DBG << " - prf not stable : " << ps_stats->i_stat_ext_time_w_from_pps_unstable_cnt ;
        _DBG << " - data:            " << ps_stats->i_last_data_nb_sample << " x "<< ps_stats->i_last_data_nb_beam << " x " << (ps_stats->i_last_data_complex?"2":"1") << " x "<< (int)ps_stats->i_last_data_format;
        _DBG << " - scale:           " << std::setprecision(3) << ps_stats->i_last_data_scale << " mv/lsb";
        _DBG << " - full scale:      " << std::setprecision(3) << ps_stats->i_last_data_full_scale << " mv/lsb";
        _DBG << " - cutoff:          " << std::setprecision(7) << ps_stats->i_last_data_cutoff << " Hz";
        _DBG << " - fdemod:          " << std::setprecision(10) << ps_stats->i_last_data_fdemod << " Hz";
        _DBG << " - time from error: " << str_tmp ;
    }

    std::shared_ptr<ST_STATS> f_analysis_new_stat(void) {
        return std::shared_ptr<ST_STATS>(new ST_STATS_PING());
    }

    int f_dispatch_handle_packet(
        bml::node<uint32_t, std::shared_ptr> & in_rpc_node,
        std::shared_ptr<ST_STATS> & in_ps_stat,
        std::string const & in_str_folder
        ) {
        auto ps_stats =  dynamic_cast<ST_STATS_PING*>(in_ps_stat.get());
        std::string str_name = f_string_format("%s/ping_%02d_%06d_%02d.bml", in_str_folder.c_str(), ps_stats->i_stat_reboot_cnt, ps_stats->i_last_trigger_uid, ps_stats->i_last_ring_id);
        _DBG << "DISPATCH "<<_V(str_name);

        auto pc_bml_writer = _c_bml.f_bml_open(str_name);
        in_rpc_node.to_writer(*pc_bml_writer.get());


        if(0 && in_rpc_node.get_size() && (ps_stats->i_last_sample_loss == false)) {
            std::vector<uint64_t> vi_abs2(ps_stats->i_last_data_nb_beam, 0);
            uint32_t i_sec_uid = f_get_secondary_id(in_rpc_node);
            {
                uint64_t i_expected_size = ps_stats->i_last_data_nb_sample * ps_stats->i_last_data_nb_beam * (ps_stats->i_last_data_complex?2:1) * (int)ps_stats->i_last_data_format;
                if(in_rpc_node.get_size() != i_expected_size) {
                    _CRIT << "Wrong ping Size" << _V(in_rpc_node.get_size()) << _V(i_expected_size);
                    exit(-1);
                }
                if((ps_stats->i_last_data_format == 2) && (ps_stats->i_last_data_complex == 1)) {
                    int16_t * pi_tmp = in_rpc_node.mmap<int16_t>();
                    int64_t i_real;
                    int64_t i_imag;
                    for(unsigned int i_sample=ps_stats->i_last_data_nb_sample/2; i_sample < ps_stats->i_last_data_nb_sample; i_sample++) {
                        for(unsigned int i_beam=0; i_beam < ps_stats->i_last_data_nb_beam; i_beam++) {
                            i_real = *pi_tmp;
                            pi_tmp++;
                            i_imag = *pi_tmp;
                            pi_tmp++;
                            uint64_t i_abs2 = i_real*i_real+i_imag*i_imag;
                            vi_abs2[i_beam] += i_abs2;
                        }
                    }
                } // Complex && int16_t

                if((ps_stats->i_last_data_format == 2) && (ps_stats->i_last_data_complex == 0)) {
                    int16_t * pi_tmp = in_rpc_node.mmap<int16_t>();
                    int64_t i_real;
                    for(unsigned int i_sample=ps_stats->i_last_data_nb_sample/2; i_sample < ps_stats->i_last_data_nb_sample; i_sample++) {
                        for(unsigned int i_beam=0; i_beam < ps_stats->i_last_data_nb_beam; i_beam++) {
                            i_real = *pi_tmp;
                            pi_tmp++;
                            uint64_t i_abs2 = i_real*i_real;
                            vi_abs2[i_beam] += i_abs2;
                        }
                    }
                } // Complex && int16_t

                {
                    std::string str_name_noise = f_string_format("%s/noise_%d.txt", in_str_folder.c_str(), i_sec_uid);
                    bool b_exist = (_mof_files.find(i_sec_uid) != _mof_files.end());
                    _DBG << "NOISE CALCULATION "<<_V(str_name_noise);

                    if(!b_exist) {
                        _mof_files[i_sec_uid].open(str_name_noise, std::ofstream::out);

                        if (_mof_files[i_sec_uid].fail()) {
                            cerr << "Unable to open file for writing." << endl;
                            exit(1);
                        }
                    }
                }

                auto & outfile = _mof_files[i_sec_uid];

                for(unsigned int i_beam=0; i_beam < ps_stats->i_last_data_nb_beam; i_beam++) {
                    uint64_t i_tmp = vi_abs2[i_beam];
                    double f_tmp = double(i_tmp/ps_stats->i_last_data_nb_sample)*ps_stats->i_last_data_scale*ps_stats->i_last_data_scale;
                    outfile << f_string_format("%03d\t%06d\t%03d",ps_stats->i_stat_reboot_cnt, ps_stats->i_last_trigger_uid, ps_stats->i_last_ring_id)
                            << f_string_format("\t%03d\t%04.1f",i_beam, 10.0*log10(f_tmp))
                            << f_string_format("\t%08.1f\t%08.1f",ps_stats->i_last_data_cutoff, ps_stats->i_last_data_fdemod)
                            << std::endl;
                }

            }
        }


        return EC_SUCCESS;
    }


};

class CT_ID_SPECIFIC {
    CT_DBNAME & _c_db;
    std::map<uint32_t, CT_ID*> _m_id_specifics;
public:

    ~CT_ID_SPECIFIC() {
        for (auto & rpc_it : _m_id_specifics) {
            delete rpc_it.second;
        }
        _m_id_specifics.clear();
        _WARN << "Clearing CT_ID_SPECIFIC";

    }

    CT_ID_SPECIFIC(CT_DBNAME & in_c_db) : _c_db(in_c_db) {
        _m_id_specifics[in_c_db.f_name2id("master::plugins::fec::E_ID_PING")]         = dynamic_cast<CT_ID*>(new CT_ID_PING(in_c_db));
        _m_id_specifics[                     0xD5E5FCE8                     ]         = dynamic_cast<CT_ID*>(new CT_ID_RAW(in_c_db, false));
        _m_id_specifics[in_c_db.f_name2id("master::plugins::s18::E_ID_INS",    true)] = dynamic_cast<CT_ID*>(new CT_ID_RAW(in_c_db, false));
        _m_id_specifics[in_c_db.f_name2id("master::plugins::record::E_ID_LOG", true)] = dynamic_cast<CT_ID*>(new CT_ID_RAW(in_c_db, false));
        _m_id_specifics[in_c_db.f_name2id("master::plugins::fec::E_ID_ZDA",    true)] = dynamic_cast<CT_ID*>(new CT_ID_RAW(in_c_db, true));
    }

    CT_ID * f_get(uint32_t in_i_id) {
        auto pc_id_specific = _m_id_specifics.find(in_i_id);
        CT_ID * pc_specific = NULL;
        if(pc_id_specific != _m_id_specifics.end()) {
            pc_specific = pc_id_specific->second;
        }
        return pc_specific;
    }
};



class CT_ANALYSIS {
    CT_ID_SPECIFIC & _c_id_spec;
    CT_DBNAME & _c_db;
    std::map<std::pair<uint32_t,uint32_t>, std::shared_ptr<ST_STATS>> _m_id;
    std::string _str_dispatch_folder;
    uint32_t _i_matio_node_cnt = 0;

public:

    CT_ANALYSIS(CT_ID_SPECIFIC & in_c_id_spec, CT_DBNAME & in_c_db, std::string const & in_str_dispatch_folder) : _c_id_spec(in_c_id_spec), _c_db(in_c_db), _str_dispatch_folder(in_str_dispatch_folder){
        _i_matio_node_cnt=0;
    }

#ifdef FF_MATIO

    int f_fill_matio(matvar_t* in_pc_var, int in_p, bml::node<uint32_t, std::shared_ptr> & in_rpc_node) {
        const char *fieldnames[6] = { "id", "data", "childs", "offset", "size", "ext"  };
        std::string mystring = "Data";
        uint32_t i_id = in_rpc_node.get_id();
        uint64_t i_size = in_rpc_node.get_size();
        uint64_t i_offset = 0;

        size_t dim_id[2] = { 1, sizeof(i_id) }; //string dimension
        matvar_t *variable_id = Mat_VarCreate(fieldnames[0], MAT_C_UINT8, MAT_T_UINT8, 2, dim_id, (void*) &i_id, 0);
        Mat_VarSetStructFieldByName(in_pc_var, "id", in_p, variable_id);


        if(in_rpc_node.get_size()) {
            size_t dim_data[2] = { 1, in_rpc_node.get_size() }; //string dimension
            matvar_t *variable_data = Mat_VarCreate(fieldnames[1], MAT_C_UINT8, MAT_T_UINT8, 2, dim_data, (void*) in_rpc_node.mmap<char>(), 0);
            Mat_VarSetStructFieldByName(in_pc_var, "data", in_p, variable_data);
        }

        size_t dim_size[2] = { 1, 1 };
        matvar_t *variable_size = Mat_VarCreate(fieldnames[3], MAT_C_UINT64, MAT_T_UINT64, 2, dim_size, (void*) &i_size, 0);
        Mat_VarSetStructFieldByName(in_pc_var, "size", in_p, variable_size);




        size_t dim_offset[2] = { 1, 1 };
        matvar_t *variable_offset = Mat_VarCreate(fieldnames[3], MAT_C_UINT64, MAT_T_UINT64, 2, dim_offset, (void*) &i_offset, 0);
        Mat_VarSetStructFieldByName(in_pc_var, "offset", in_p, variable_offset);

        uint16_t i_nb_ext =0;
        while(in_rpc_node.has_ext(i_nb_ext)) {
            i_nb_ext++;
        }

        // TODO, this is an hack in order to get uint64_t without updating the BML library
        // - Add generic methods to bml_node.hh to get ext in various format.
        size_t dim_ext[2] = { i_nb_ext, 1 }; //string dimension
        matvar_t *variable_ext = Mat_VarCreate(fieldnames[0], MAT_C_CELL, MAT_T_CELL, 2, dim_ext, NULL, 0);
        for(int i=0; i<i_nb_ext; i++) {
            uint64_t i_ext = in_rpc_node.get_ext<uint64_t>(i);
            size_t dim_ext_val[2] = { 1, 1 }; //string dimension
            matvar_t *variable_ext_val = Mat_VarCreate(fieldnames[0], MAT_C_UINT64, MAT_T_UINT64, 2, dim_ext_val, (void*) &i_ext, 0);
            Mat_VarSetCell(variable_ext, i, variable_ext_val);
        }
        Mat_VarSetStructFieldByName(in_pc_var, "ext", in_p, variable_ext);

        const char *structname = "unused";
        auto childs = in_rpc_node.childs();
        size_t structdim[2] = { 1, childs.size() };
        matvar_t* matstruct_childs = Mat_VarCreateStruct(structname, 2, structdim, fieldnames, 6);
        int32_t i_child_id = 0;
        for(auto && pc_it: in_rpc_node.childs()) {
            f_fill_matio(matstruct_childs, i_child_id, *(pc_it->second));
            i_child_id++;
        }
        Mat_VarSetStructFieldByName(in_pc_var, "childs", in_p, matstruct_childs);

        return EC_SUCCESS;
    }

    int f_dispatch_matio(bml::node<uint32_t, std::shared_ptr> & in_rpc_node, std::string const & in_str_folder) {
        std::string str_name = f_string_format("%s/mat5_%04d_%08x.mat", in_str_folder.c_str(),_i_matio_node_cnt, in_rpc_node.get_id());
        _DBG << "Creating "<< str_name;
        _i_matio_node_cnt++;

        mat_t *matfp = NULL; //matfp contains pointer to MAT file or NULL on failure
        matfp = Mat_CreateVer(str_name.c_str(), NULL, MAT_FT_MAT5); //or MAT_FT_MAT4 / MAT_FT_MAT73

        const char *structname = "node";
        const char *fieldnames[6] = { "id", "data", "childs", "offset", "size", "ext"};
        size_t structdim0[2] = { 1, 1 }; // create 1 x p struct
        matvar_t* matstruct0 = Mat_VarCreateStruct(structname, 2, structdim0, fieldnames, 6); //main struct: Test

        f_fill_matio(matstruct0, 0, in_rpc_node);

        Mat_VarWrite(matfp, matstruct0, MAT_COMPRESSION_NONE);
        Mat_VarFree(matstruct0);
        Mat_Close(matfp);

        return EC_SUCCESS;
    }
#endif



    int f_handle_packet(bml::node<uint32_t, std::shared_ptr> & in_rpc_node, bool in_b_dispatch, bool in_b_dispatch_matio) {
        uint32_t i_id =  in_rpc_node.get_id() ;
        //_DBG << "Handling node: " << std::hex << i_id << " "<< _c_db.f_id_name(i_id);

        uint64_t i_record_time = 0;
        if(in_rpc_node.has_ext(0)) {
            in_rpc_node.get_ext<uint64_t>(0);
        }

        uint32_t i_second_key = 0;
        CT_ID * pc_id_spec = _c_id_spec.f_get(i_id);

        if(pc_id_spec) {
            i_second_key = pc_id_spec->f_get_secondary_id(in_rpc_node);
        }
        std::pair<uint32_t,uint32_t> c_key (i_id, i_second_key);

        auto pc_it = _m_id.find(c_key);

        if(pc_it == _m_id.end()) {
            if(pc_id_spec) {
                _m_id[c_key] = pc_id_spec->f_analysis_new_stat();
            } else {
                _m_id[c_key] = std::shared_ptr<ST_STATS>(new ST_STATS);
            }
        }

        auto & s_res = _m_id[c_key];
        if(s_res->i_cnt == 0) {
            s_res->i_first_time = i_record_time;
            s_res->i_size = in_rpc_node.get_size();;
            s_res->i_size_acc = in_rpc_node.get_size();
        } else {
            s_res->i_size_acc += in_rpc_node.get_size();
        }

        int64_t i_diff = int64_t(i_record_time)-int64_t(s_res->i_last_time);
        M_ASSERT(i_diff >= 0);
        if(i_diff > int64_t(s_res->i_max_diff_time) ) {
            s_res->i_max_diff_time = i_diff;
        }


        s_res->i_cnt += 1;
        s_res->i_last_time = i_record_time;

        if(pc_id_spec) {
            pc_id_spec->f_analysis_handle_packet(in_rpc_node, s_res);

            if(in_b_dispatch) {
                pc_id_spec->f_dispatch_handle_packet(in_rpc_node, s_res, _str_dispatch_folder);
            }
        }

#ifdef FF_MATIO
            if(in_b_dispatch_matio) {
                f_dispatch_matio(in_rpc_node, _str_dispatch_folder);
            }
#endif
        return EC_SUCCESS;
    }


    void f_display_stats(CT_DBNAME & in_c_db) {
        for(auto & pc_it : _m_id) {
            auto & s_res = pc_it.second;
            uint32_t i_id = pc_it.first.first;
            uint32_t i_id_second = pc_it.first.second;

            //_DBG << _V(i_id) << _V(s_res->i_cnt) << _V(s_res->i_first_time)<< _V(s_res->i_last_time) << _V(s_res->i_size);
            uint64_t i_diff = s_res->i_last_time - s_res->i_first_time;
            double f_diff = double(i_diff)/1e9;
            double f_rate = double(s_res->i_size_acc) / f_diff;
            std::string str_size;
            std::string str_rate;
            std::string str_first;
            std::string str_last;
            std::string str_max_time;
            double f_mean_time = double(i_diff)/1e9/double(s_res->i_cnt);

            f_string_human_readable_number(str_size, s_res->i_size, E_STRING_HUMAN_READABLE_MODE_DEFAULT);
            f_string_human_readable_number(str_rate, f_rate, E_STRING_HUMAN_READABLE_MODE_DEFAULT);
            f_string_human_readable_time(str_max_time, double(s_res->i_max_diff_time)/1e9);
            str_first = f_string_strftime_64ns("%c", s_res->i_first_time);
            str_last = f_string_strftime_64ns("%c",  s_res->i_last_time);

            _DBG << "STATS on node: " << std::hex << i_id << std::dec <<" "<< in_c_db.f_id_name(i_id) ;
            _DBG << " # Secondary key:   "<< i_id_second;
            _DBG << " - mean time:       "<< f_mean_time ;
            _DBG << " - mean frequency:  "<< 1.0/f_mean_time ;
            _DBG << " - size:            "<< str_size ;
            _DBG << " - max time:        "<< str_max_time ;
            _DBG << " - rate:            "<< str_rate <<"/s";
            _DBG << " - first packet:    " << str_first ;
            _DBG << " - last packet:     " << str_last ;
            _DBG << " - nb node:         " << s_res->i_cnt ;
            {
                // FIXE REPLACE with a dictionnary updated during packet handle
                CT_ID * pc_id = _c_id_spec.f_get(i_id);
                if(pc_id) {
                    pc_id->f_analysis_display_stats(s_res);
                }
            }
        }
    }
};



int f_parse_file(uint32_t in_i_mode, std::string const & in_str_file, CT_DBNAME & in_c_db, std::string const & in_str_dispatch_folder) {
    int ec = EC_SUCCESS;
    std::string str_ext, str_basename,
        str_basename_part, str_file, str_part, str_parent_path;

    CT_ID_SPECIFIC c_specific(in_c_db);
    CT_ANALYSIS c_analysis(c_specific, in_c_db, in_str_dispatch_folder);


    str_basename_part = fs::path(in_str_file).stem().string();
    str_ext = fs::path(in_str_file).extension().string();
    str_parent_path = fs::path(in_str_file).parent_path().string();
//    f_string_split_ext_and_base(str_basename_part,
//                                str_ext, in_str_file);
    M_ASSERT(str_ext == ".bml");

    /* Get part number of existing in file */
    f_string_split_on_last_delim(str_basename, str_part,
                                 str_basename_part, "_");

    int i_part_offset;
    if (str_part.size()) {
        _DBG << str_part;
        try {
            std::stringstream ss(str_part);
            ss >> i_part_offset;
            if(ss.fail()) {
                throw  _DBG << "Part string is not a number :"
                           << str_part;
            }
        } catch (...) {
            i_part_offset = -1;
        }
    } else {
        i_part_offset = -1;
    }

    uint32_t i_part = 0;
    bool b_continue = true;
    while(b_continue) {
        std::stringstream str_file_name;
        if(i_part_offset== -1){
            str_file_name << in_str_file;
            str_file = str_file_name.str();
        } else {
            str_file_name << str_basename.c_str() << "_" << std::setfill('0') << std::setw(str_part.size()) << i_part + i_part_offset << ".bml";
            str_file = ((fs::path(str_parent_path) / fs::path(str_file_name.str()))).string();
        }

        if(i_part_offset== -1){
            if(i_part == 1) {
                b_continue = false;
                ec = EC_SUCCESS;
                break;
            }
        }



        //_DBG << "Trying next part file " << str_file;
        /* If part exist, load it */
        if (fs::exists(fs::path(str_file))) {
            _DBG << "Opening next part file: " << str_file;

            bml::node_file_parser<std::shared_ptr> * pc_reader = NULL;
            try {
                pc_reader = new bml::node_file_parser<std::shared_ptr>(str_file);
                std::shared_ptr<bml::node<uint32_t, std::shared_ptr>> c_guard(new bml::node<uint32_t, std::shared_ptr>());

                while(1) {
                    ec = c_guard->from_parser(*pc_reader);
                    if (ec == EC_BML_NODATA) {
                        c_analysis.f_display_stats(in_c_db);
                        _DBG << "EOF";
                        i_part++;
                        break;
                    } else if (ec == EC_FAILURE) {
                        _CRIT << "Error during BML read";
                        b_continue = false;
                        ec = EC_FAILURE;
                        break;
                    } else {
                        ec = c_analysis.f_handle_packet(*c_guard, in_i_mode&E_MODE_DISPATCH, in_i_mode&E_MODE_MATIO);
                        if(ec != EC_SUCCESS) {
                            _CRIT << "Error during analysis";
                            b_continue = false;
                        }
                    }

                }

                delete pc_reader;
                pc_reader = NULL;
            } catch (...) {
                _CRIT << "Unable to open" << str_file;
                b_continue = false;
                ec = EC_FAILURE;
            }

            /* Free memory */
            if(pc_reader) {
                delete pc_reader;
            }
        } else {
            _DBG << "File " << str_file << " does not exist, stopping read";
            b_continue = false;
            ec = EC_SUCCESS;
        }
    }

    _DBG << "==== FINAL STATS ====" ;
    c_analysis.f_display_stats(in_c_db);
    if(ec != EC_SUCCESS) {
        _CRIT << "an error has occured during analysis";
    }

    return ec;
}


std::string f_get_directory (const std::string& in_str_path)
{
    size_t found = in_str_path.find_last_of("/\\");
    return(in_str_path.substr(0, found));
}

int main(int argc, char** argv)
{
    int i_mode{};
    string str_file{};
    string str_db{};
    string str_dispatch_folder{};


    //vector<std::string> vstr_ids{};
    //vector<string> vs{};

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
            ("mode,m", value<int>(&i_mode)->default_value(0), "mode de fonctionnement (0:analyse, 1:dipatch, 2:matio, 3:dispatch+matio)")
        //("float,f", value<float>(&f)->default_value(3.141f), "float value")
        ("file,f", value<string>(&str_file)->default_value("/tmp/record.bml"), "BML file name")
            ("db,d", value<string>(&str_db)->default_value(""), "Id database file name")
                ("out,o", value<string>(&str_dispatch_folder)->default_value(""), "Output folder")
        //("sec,s", value<vector<std::string>>(&vstr_ids), "list of sec ids")
        //("string_list,b", value<vector<string>>(&vs), "list of string values")
        ;

    variables_map vm{};
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.size() == 0 || vm.count("help"))
    {
        cout << desc << "\n";
        return 1;
    }


    /*  if(vm.count("sec"))
    for(auto it : vstr_ids)
      cout << "List of ints value was set to " << it << endl;*/
    /*
   if(vm.count("float")) cout << "Float value was set to " << f << endl;
   if(vm.count("string")) cout << "String value was set to \"" << s << "\"" << endl;

    if(vm.count("string_list"))
        for(auto& it : vs)
            cout << "List of strings value was set to \"" << it << "\"" << endl;
    */
        if(str_dispatch_folder.size() == 0) {
        str_dispatch_folder = str_file+".bml-parser";
//        mkdir(str_dispatch_folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        fs::create_directories(fs::path(str_dispatch_folder));
    }

    goutfile.open(str_dispatch_folder+"/bml_parser_log.txt", std::ios_base::out); // append instead of overwrite
    f_debug_register(&gs_debug_local);
    _DBG << "Version: "<< M_STR_VERSION;
    if(vm.count("mode")) _DBG << "Mode value was set to " << i_mode;
    if(vm.count("file")) _DBG << "File value was set to \"" << str_file << "\"";
    if(vm.count("db")) _DBG << "Db value was set to \"" << str_db << "\"";
    if(vm.count("out")) _DBG << "Out value was set to \"" << str_dispatch_folder << "\"";

    CT_DBNAME c_db_name(str_db);
    /*
    std::vector<uint32_t> vi_ids;
    for(auto & str_sec_id : vstr_ids) {
      uint32_t i_id = c_db_name.f_name2id(str_sec_id);
      if(i_id) {
        vi_ids.push_back(i_id);
      }
    }
    */

    {
        int ec;
        ec = f_parse_file(i_mode, str_file, c_db_name, str_dispatch_folder);
        return ec;
    }


}
