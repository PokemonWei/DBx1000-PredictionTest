/*collect some information for txn runing in the system*/

#ifndef TXN_STATS
#define TXN_STATS

#include "global.h"
#include "txn.h"
#include "mem_alloc.h"

#include <map>
#include <vector>

typedef uint64_t key_type;

enum MyTxnType
{
    TXN_READ,  //1
    TXN_WR,  //2
    TXN_SCAN,   //3
    SCAN_READ, // 1+3 
    SCAN_WR //2+3
};

struct EachTxnStats //map
{
    /*CPU time*/
    double cpu_time;
    /*memory size*/
    uint64_t me_size;
    /*WR*/
    map<key_type,uint32_t> read_keys;
    map<key_type,uint32_t> write_keys;
    map<key_type,uint32_t> scan_keys; // start_key and scan_len
    /*access type*/
    MyTxnType _type;
    /*result*/
    RC rc;
    /*start time*/
    double start_time;
};

enum TXN_STATS_TYPE
{
    CPU_TIME,
    MEMORY,
    READ_KEY,
    WRITE_KEY,
    SCAN_KEY,
    TXN_TYPE,
    RESULT,
    START_TIME
};

struct ScanInfo
{
    key_type scan_key;
    uint32_t scan_len;
};


typedef map<txnid_t,EachTxnStats*> TxnMap;

class TxnStats
{
    private:

    map<txnid_t,EachTxnStats*> txn_infor_map;

    bool insert_latch;

    public:

    void init();
    
    static void clearStats(EachTxnStats * txn_stats); // init each_txn_stats

    void add_stats(txnid_t txn_id,TXN_STATS_TYPE type,void * value);

    bool add_txn(txnid_t txn_id);

    void final_type_cal(uint8_t * final_type,access_t rtype);

    void txn_finish(txn_man * txn,base_query* query,RC rc,uint64_t timespan,uint64_t start_time);
    
    void stats_print();
};

extern TxnStats txn_stats;

#endif