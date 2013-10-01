#include "sql_priv.h"
#include "sql_class.h"
#include "ha_ranking.h"
#include "probes_mysql.h"
#include "sql_plugin.h"

handlerton* ranking_hton;

#ifdef HAVE_PSI_INTERFACE

static PSI_mutex_key ex_key_mutex_Ranking_share_mutex;

static PSI_mutex_info all_ranking_mutexes[] = {
  { &ex_key_mutex_Ranking_share_mutex, "ha_ranking::share_t::mutex", 0 }
};

#endif

ha_ranking::share_t::share_t()
{
  thr_lock_init(&lock);
  mysql_mutex_init(ex_key_mutex_Ranking_share_mutex, &mutex,
                   MY_MUTEX_INIT_FAST);
}

ha_ranking::share_t::~share_t()
{
  thr_lock_delete(&lock);
  mysql_mutex_destroy(&mutex);
}

ha_ranking::share_t* ha_ranking::_get_share()
{
  DBUG_ENTER("ha_ranking::get_share()");

  lock_shared_ha_data();

  share_t* s = static_cast<share_t*>(get_ha_share_ptr());
  if (s == NULL) {
    s = new share_t;
    if (s != NULL) {
      set_ha_share_ptr(s);
    }
  }

  unlock_shared_ha_data();
  DBUG_RETURN(s);
}

ha_ranking::ha_ranking(handlerton* hton, TABLE_SHARE* table_arg)
  : handler(hton, table_arg), share(NULL)
{
}

ha_ranking::~ha_ranking()
{
}

const char** ha_ranking::bas_ext() const
{
  static const char* basexts[] = {
    // ".rnk",
    NullS
  };

  DBUG_ENTER("ha_ranking::bas_ext");
  DBUG_RETURN(basexts);
}

int ha_ranking::open(const char* name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_ranking::open");

  if ((share = _get_share()) == NULL)
    DBUG_RETURN(1);
  thr_lock_data_init(&share->lock, &lock, NULL);

  DBUG_RETURN(0);
}

int ha_ranking::close()
{
  DBUG_ENTER("ha_ranking::close");
  DBUG_RETURN(0);
}

int ha_ranking::rnd_init(bool scan)
{
  DBUG_ENTER("ha_ranking::rnd_init");
  DBUG_RETURN(0);
}

int ha_ranking::rnd_end()
{
  DBUG_ENTER("ha_ranking::rnd_end");
  DBUG_RETURN(0);
}

int ha_ranking::rnd_next(uchar* buf)
{
  DBUG_ENTER("ha_ranking::rnd_next");
  MYSQL_READ_ROW_START(table_share->db.str, table_share->table_name.str, TRUE);

  int rc = HA_ERR_END_OF_FILE;

  MYSQL_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}

void ha_ranking::position(const uchar* record)
{
  DBUG_ENTER("ha_ranking::position");
  DBUG_VOID_RETURN;
}

int ha_ranking::rnd_pos(uchar *buf, uchar *pos)
{
  DBUG_ENTER("ha_ranking::rnd_pos");
  MYSQL_READ_ROW_START(table_share->db.str, table_share->table_name.str,
                       TRUE);

  int rc = HA_ERR_WRONG_COMMAND;

  MYSQL_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}

int ha_ranking::info(uint flag)
{
  DBUG_ENTER("ha_ranking::info");
  stats.records = 0;
  DBUG_RETURN(0);
}

int ha_ranking::external_lock(THD* thd, int lock_type)
{
  DBUG_ENTER("ha_ranking::external_lock");
  DBUG_RETURN(0);
}

THR_LOCK_DATA** ha_ranking::store_lock(THD* thd, THR_LOCK_DATA** to,
                                       thr_lock_type lock_type)
{
  if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK) {
    lock.type=lock_type;
  }
  *to++= &lock;
  return to;
}

int ha_ranking::create(const char* name, TABLE* form,
                       HA_CREATE_INFO* create_info)
{
  DBUG_ENTER("ha_ranking::create");
  DBUG_RETURN(0);
}

handler* ha_ranking::_create_handler(handlerton* hton, TABLE_SHARE* table,
                                     MEM_ROOT* mem_root)
{
  DBUG_ENTER("ha_ranking::_create_handler");
  handler* h = new (mem_root) ha_ranking(hton, table);
  DBUG_RETURN(h);
}

int ha_ranking::init_plugin(void* p)
{
  DBUG_ENTER("ha_ranking::init_plugin");

#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register("ranking", all_ranking_mutexes,
                       array_elements(all_ranking_mutexes));
#endif

  ranking_hton = (handlerton*)p;
  ranking_hton->state  = SHOW_OPTION_YES;
  ranking_hton->create = ha_ranking::_create_handler;
  ranking_hton->flags  = HTON_CAN_RECREATE;

  DBUG_RETURN(0);
}

static st_mysql_storage_engine ranking_storage_engine = {
  MYSQL_HANDLERTON_INTERFACE_VERSION
};

mysql_declare_plugin(ranking)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &ranking_storage_engine,
  "RANKING",
  "Kazuho Oku, DeNA Co., Ltd",
  "Ranking storage engine",
  PLUGIN_LICENSE_GPL,
  ha_ranking::init_plugin,
  NULL,
  0x0001, // version
}
mysql_declare_plugin_end;
