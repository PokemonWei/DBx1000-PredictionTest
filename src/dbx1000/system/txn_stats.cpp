#include "txn_stats.h"

void TxnStats::clearStats(EachTxnStats * txn_stats)
{
    txn_stats->cpu_time = 0;
    txn_stats->me_size = 0;
    txn_stats->read_size = 0;
    txn_stats->read_count = 0;
    txn_stats->write_count = 0;
    txn_stats->write_size = 0;
    //txn_stats->io_time = 0;
}

void TxnStats::init()
{
    txn_infor_map.clear();
}

void TxnStats::add_stats(txnid_t txn_id,TXN_STATS_TYPE type,void * value)
{
    if(txn_infor_map.find(txn_id)==txn_infor_map.end())
    {
        bool id_not_found = true;
        assert(!id_not_found);
    }
    EachTxnStats * txn_stats = txn_infor_map[txn_id];
    map<key_type,uint32_t> * keys = NULL;
    switch (type)
    {
        case CPU_TIME:
            txn_stats->cpu_time += *((double*)value);
            break;
        case MEMORY:
            txn_stats->me_size += *((UInt64*)value);
            break;
        case READ_KEY:
            keys = &txn_stats->read_keys[*value]++;
            break;
        case WRITE_KEY:
            keys = &txn_stats->write_keys[*value]++;
            break;
        case SCAN_KEY:
            SCANInfo * s = (ScanInfo*)value;
            scan_keys[s->scan_key] += s->scan_len; 
            break;
        case ACCESS_TYPE:
            txn_stats->type = *value;
            break;
        case START_TIME:
            txn_stats->start_time = *value;
        default:
            bool type_not_found = true;
            ASSERT(!type_not_found);
            break;
    }
    if(keys != NULL)
    {
        if (keys->find(*value) == keys.end()) 
        {
            (*keys)[*value] = 0;
        }
        (*key)[*value]++;
    }
}

bool TxnStats::add_txn(txnid_t txn_id)
{
    if(txn_infor_map.find(txn_id) != txn_infor_map.end())
    {
        return false;
    }
    EachTxnStats * temp_stats = (EachTxnStats*)mem_allocator.alloc(sizeof(EachTxnStats),0);
    clearStats(temp_stats);
    txn_infor_map[txn_id] = temp_stats; 
    return true;
}

void TxnStats::final_type_cal(uint8_t * final_type,access_t rtype)
{
    uint8_t temp_type = 0;
    switch (rtype)
    {
        case RD:
            temp_type = 1;
            break;
        case WR:
            temp_type = 2;
            break;
        case SCAN:
            temp_type = 3;
            break;
        default:
            assert(false);
            break;
    }
    if((temp_type<3 && *final_type <3) || (temp_type>=3 && *final_type>=3))
    {
        *final_type = temp_type>*final_type ? temp_type:*final_type；
    }
    else
    {
        if(final_type < 3) //temp_type = 3
        {
            *final_type = *final_type + 3;
        }
        else //temptype < 3 && finaltype>=3
        {
            *final_type = *final_type - 3;
            *final_type = temp_type>*final_type ? temp_type:*final_type；
            *final_type = *final_type + 3;
        }
    }
}


void TxnStats::txn_finish(txn_man * txn,base_query* query,RC rc,uint64_t timespan,uint64_t start_time)
{
    txnid_t txn_id = txn->get_txn_id();
    add_txn(txn_id);
    add_stats(txn_id,RESULT,rc);
    add_stats(txn_id,CPU_TIME,timespan);
    add_stats(txn_id,START_TIME,start_time);

#if WROKLOAD == YCSB
    uint8_t final_type = 0;

    ycsb_query * now_query = (ycsb_query*)query;
    ycsb_request * req = now_query->requests;
    for(int i=0; i < now_query->request_cnt; i++)
    {
        final_type_cal(&final_type,req[i]->rtype);
        if(rtype == RD)
        {
            add_stats(txn_id,READ_KEY,req[i]->key);
        }
        else if (rtype == WR) 
        {
            add_stats(txn_id,WRITE_KEY,req[i]->key);
        }
        else if(type == SCAN)
        {
            ScanInfo s;
            s.scan_key = req[i]->key;
            s.scan_len = req[i]->scan_len;
            add_stats(txn_id,SCAN_KEY,&s);
        }
    }

    TxnType txn_type;
    switch (final_type)
    {
        case 1:
            txn_type = READ;
            break;
        case 2:
            txn_type = WR;
            break;
        case 3:
            txn_type = SCAN;
            break;
        case 4:
            txn_type = SCAN_READ;
            break;
         case 5:
            txn_type = SCAN_WR;
            break;
        default:
            assert(false);
            break;
    }

    add_stats(txn_id,TXN_TYPE,&txn_type);
#endif

}

