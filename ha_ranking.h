#ifndef ha_ranking_h
#define ha_ranking_h

#include "my_global.h"
#include "thr_lock.h"
#include "handler.h"
#include "my_base.h"

class ha_ranking : public handler {

private:
  struct share_t : public Handler_share {
    mysql_mutex_t mutex;
    THR_LOCK lock;
    share_t();
    ~share_t();
  };

  THR_LOCK_DATA lock;
  share_t* share;

  share_t* _get_share();

public:
  ha_ranking(handlerton* hton, TABLE_SHARE* table_arg);
  ~ha_ranking();

  const char* table_type() const { return "RANKING"; }
  const char* index_type(uint keynr) { return "NONE"; }
  const char** bas_ext() const;
  ulonglong table_flags() const {
    return HA_NO_TRANSACTIONS
      | HA_REC_NOT_IN_SEQ
      | HA_STATS_RECORDS_IS_EXACT
      | HA_CAN_BIT_FIELD
      // | HA_BINLOG_ROW_CAPABLE
      // | HA_BINLOG_STMT_CAPABLE
      | HA_FILE_BASED
      | HA_NO_AUTO_INCREMENT
      | HA_HAS_RECORDS;
  }

  ulong index_flags(uint keynr, uint part, bool all_parts) const { return 0; }

  int open(const char* name, int mode, uint test_if_locked);
  int close();
  int rnd_init(bool scan);
  int rnd_end();
  int rnd_next(uchar* buf);
  int rnd_pos(uchar* buf, uchar* pos);
  void position(const uchar* record);
  int info(uint);
  int external_lock(THD* thd, int locktype);
  THR_LOCK_DATA** store_lock(THD* thd, THR_LOCK_DATA** to,
                             thr_lock_type lock_type);

  int create(const char* name, TABLE* form, HA_CREATE_INFO* create_info);

private:
  static handler* _create_handler(handlerton* hton, TABLE_SHARE* table,
                                  MEM_ROOT* mem_root);
public:
  static int init_plugin(void* p);
};

#endif
