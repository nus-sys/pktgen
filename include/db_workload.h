#ifndef _DB_WORKLOAD_H_
#define _DB_WORKLOAD_H_

#include <vector>
#include <string>

#include <chrono>
#include <string.h>

#include "properties.h"
#include "core/generator.h"
#include "core/discrete_generator.h"
#include "core/counter_generator.h"
#include "workload.h"
#include "utils.h"

enum Operation {
    INSERT = 1,
    READ,
    UPDATE,
    SCAN,
    READMODIFYWRITE
};

struct db_ts {
	uint64_t send_start;
	uint64_t completion_time;
};

struct db_message {
    uint64_t op;
    uint64_t send_start;
    uint64_t completion_time;
    char cmd[256];
} __attribute__((__packed__));

class DBWorkload : public Workload {
public:
    /// 
    /// The name of the property for the field length distribution.
    /// Options are "uniform", "zipfian" (favoring short records), and "constant".
    ///
    static const std::string FIELD_LENGTH_DISTRIBUTION_PROPERTY;
    static const std::string FIELD_LENGTH_DISTRIBUTION_DEFAULT;
    
    /// 
    /// The name of the property for the length of a field in bytes.
    ///
    static const std::string FIELD_LENGTH_PROPERTY;
    static const std::string FIELD_LENGTH_DEFAULT;
    
    /// 
    /// The name of the property for the proportion of read transactions.
    ///
    static const std::string READ_PROPORTION_PROPERTY;
    static const std::string READ_PROPORTION_DEFAULT;
    
    /// 
    /// The name of the property for the proportion of update transactions.
    ///
    static const std::string UPDATE_PROPORTION_PROPERTY;
    static const std::string UPDATE_PROPORTION_DEFAULT;
    
    /// 
    /// The name of the property for the proportion of insert transactions.
    ///
    static const std::string INSERT_PROPORTION_PROPERTY;
    static const std::string INSERT_PROPORTION_DEFAULT;
    
    /// 
    /// The name of the property for the proportion of scan transactions.
    ///
    static const std::string SCAN_PROPORTION_PROPERTY;
    static const std::string SCAN_PROPORTION_DEFAULT;
    
    ///
    /// The name of the property for the proportion of
    /// read-modify-write transactions.
    ///
    static const std::string READMODIFYWRITE_PROPORTION_PROPERTY;
    static const std::string READMODIFYWRITE_PROPORTION_DEFAULT;
    
    /// 
    /// The name of the property for the the distribution of request keys.
    /// Options are "uniform", "zipfian" and "latest".
    ///
    static const std::string REQUEST_DISTRIBUTION_PROPERTY;
    static const std::string REQUEST_DISTRIBUTION_DEFAULT;
    
    ///
    /// The name of the property for adding zero padding to record numbers in order to match 
    /// string sort order. Controls the number of 0s to left pad with.
    ///
    static const std::string ZERO_PADDING_PROPERTY;
    static const std::string ZERO_PADDING_DEFAULT;

    /// 
    /// The name of the property for the max scan length (number of records).
    ///
    static const std::string MAX_SCAN_LENGTH_PROPERTY;
    static const std::string MAX_SCAN_LENGTH_DEFAULT;
    
    /// 
    /// The name of the property for the scan length distribution.
    /// Options are "uniform" and "zipfian" (favoring short scans).
    ///
    static const std::string SCAN_LENGTH_DISTRIBUTION_PROPERTY;
    static const std::string SCAN_LENGTH_DISTRIBUTION_DEFAULT;

    /// 
    /// The name of the property for the order to insert records.
    /// Options are "ordered" or "hashed".
    ///
    static const std::string INSERT_ORDER_PROPERTY;
    static const std::string INSERT_ORDER_DEFAULT;

    static const std::string INSERT_START_PROPERTY;
    static const std::string INSERT_START_DEFAULT;
    
    static const std::string RECORD_COUNT_PROPERTY;
    static const std::string OPERATION_COUNT_PROPERTY;

    ///
    /// Initialize the scenario.
    /// Called once, in the main client thread, before any operations are started.
    ///
    virtual void Init(const Properties &p);
    
    virtual std::string BuildValues(void);
    virtual std::string BuildUpdate(void);
    
    virtual size_t OpCount() { return op_count_; }
    virtual size_t RecordCount() { return record_count_; }
    virtual std::string NextSequenceKey();
    virtual std::string NextTransactionKey();
    virtual Operation NextOperation() { return op_chooser_.Next(); }
    virtual size_t NextScanLength() { return scan_len_chooser_->Next(); }

    DBWorkload() :
        field_len_generator_(NULL), key_generator_(NULL), key_chooser_(NULL),
        scan_len_chooser_(NULL), insert_key_sequence_(3),
        ordered_inserts_(true), record_count_(0), op_count_(0), 
        nr_latency(0), nb_tx_(0), nb_rx_(0), nb_insert_(0), nb_read_(0), nb_update_(0), nb_scan_(0) {
            latencies = (struct db_ts *)calloc(131072, sizeof(struct db_ts));
    }
    
    virtual ~DBWorkload() {
        if (field_len_generator_) delete field_len_generator_;
        if (key_generator_) delete key_generator_;
        if (key_chooser_) delete key_chooser_;
        if (scan_len_chooser_) delete scan_len_chooser_;
    }

    uint64_t CurrentTime_nanoseconds() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint16_t GenerateNextReq(uint8_t * pkt, int len) {
        Operation nextOp = this->NextOperation();
        nb_tx_++;
        switch (nextOp) {
            case READ:
                TransactionRead(pkt, len);
                break;
            case UPDATE:
                TransactionUpdate(pkt, len);
                break;
            case INSERT:
                TransactionInsert(pkt, len);
                break;
            case SCAN:
                TransactionScan(pkt, len);
                break;
            case READMODIFYWRITE:
                TransactionReadModifyWrite(pkt, len);
                break;
            default:
                throw Exception("Operation request is not recognized!");
        }

        return (uint16_t)nextOp;
    }

    uint16_t RecordReply(uint8_t * buf) {
        struct db_message * msg;
        msg = (struct db_message *)buf;

        nb_rx_++;
        if (msg->op == INSERT) {
            nb_insert_++;
        } else if (msg->op == READ) {
            nb_read_++;
        } else if (msg->op == UPDATE) {
            nb_update_++;
        } else if (msg->op == SCAN) {
            nb_scan_++;
        }

        if (nr_latency < 131072) {
            latencies[nr_latency].send_start = msg->send_start;
            latencies[nr_latency].completion_time = CurrentTime_nanoseconds();
            nr_latency++;
        }

        return 0;
    }

    void PrintResult(uint64_t duration) {
        uint64_t send_start, completion_time, elapsed;
        std::ofstream result; // outs is an output stream of iostream class
        FILE * thp_result;
        char name[32];

        result.open("latency-" + std::to_string(sched_getcpu()) + ".txt") ; // connect outs to file outFile

        for (int i = 0; i < nr_latency; i++) {
            send_start = latencies[i].send_start;
            completion_time = latencies[i].completion_time;
            elapsed = latencies[i].completion_time - latencies[i].send_start;
            result << send_start << "\t" << completion_time << "\t" << elapsed << "\n";
        }

        result.close() ;    // closing the output file stream

        sprintf(name, "thp-%d.txt", sched_getcpu());
        thp_result = fopen(name, "w");
        if (!thp_result) {
            printf("Error!\n");
        }

        fprintf(thp_result, "%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n", 
            ((float)nb_rx_) * 1000.0 / duration, 
            ((float)nb_insert_) * 1000.0 / duration, 
            ((float)nb_read_) * 1000.0 / duration, 
            ((float)nb_update_) * 1000.0 / duration,
            ((float)nb_scan_) * 1000.0 / duration);
        fclose(thp_result);

        printf("CPU %02d| Duration: %lu s, Rx: %.4f (Kpps), INSERT: %.4f, READ: %.4f, UPDATE:%.4f, SCAN: %.4f, Tx: %.4f (Kpps)\n", 
                sched_getcpu(), duration / 1000000, 
                ((float)nb_rx_) * 1000.0 / duration, 
                ((float)nb_insert_) * 1000.0 / duration, 
                ((float)nb_read_) * 1000.0 / duration, 
                ((float)nb_update_) * 1000.0 / duration, 
                ((float)nb_scan_) * 1000.0 / duration, 
                ((float)nb_tx_) * 1000.0 / duration);
    }

protected:
    virtual void TransactionRead(uint8_t *, int);
    virtual void TransactionReadModifyWrite(uint8_t *, int);
    virtual void TransactionScan(uint8_t *, int);
    virtual void TransactionUpdate(uint8_t *, int);
    virtual void TransactionInsert(uint8_t *, int);

    static Generator<uint64_t> *GetFieldLenGenerator(const Properties &p);
    std::string BuildKeyName(uint64_t key_num);

    Generator<uint64_t> *field_len_generator_;
    Generator<uint64_t> *key_generator_;
    DiscreteGenerator<Operation> op_chooser_;
    Generator<uint64_t> *key_chooser_;
    Generator<uint64_t> *scan_len_chooser_;
    CounterGenerator insert_key_sequence_;
    bool ordered_inserts_;
    size_t record_count_;
    size_t op_count_;
    int zero_padding_;

    int nr_latency;
    struct db_ts * latencies;
    uint64_t nb_tx_;
    uint64_t nb_rx_;
    uint64_t nb_insert_;
    uint64_t nb_read_;
    uint64_t nb_update_;
    uint64_t nb_scan_;
};

inline std::string DBWorkload::NextSequenceKey() {
    uint64_t key_num = key_generator_->Next();
    return BuildKeyName(key_num);
}

inline std::string DBWorkload::NextTransactionKey() {
    uint64_t key_num;
    do {
        key_num = key_chooser_->Next();
    } while (key_num > insert_key_sequence_.Last());
    return BuildKeyName(key_num);
}

inline std::string Insert(const std::string &key, std::string &value) {
    std::string request;
    request.append("SET").append(" ").append(key).append(" ").append(value);
    return request;
}

inline std::string Read(const std::string &key) {
    std::string request;
    request.append("GET").append(" ").append(key);
    return request;
}

inline std::string Scan(const std::string &key, int len) {
    std::string request;
    std::string length = std::to_string(len);
    request.append("SCAN").append(" ").append(key).append(" ").append(length);
    return request;
}

inline void DBWorkload::TransactionRead(uint8_t * buf, int len) {
    const std::string &key = this->NextTransactionKey();
    struct db_message * msg = (struct db_message *)buf;

    // msg->op_code = READ;
    // memcpy(msg->key, key.c_str(), key.length());

    std::string cmd = Read(key);
    strcpy(msg->cmd, cmd.c_str());

    msg->op = READ;
    msg->send_start = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();;
    msg->completion_time = 0;
}

inline void DBWorkload::TransactionReadModifyWrite(uint8_t * buf, int len) {
    // const std::string &key = this->NextTransactionKey();
    // std::vector<DB::KVPair> result;

    // if (!this->read_all_fields()) {
    //     std::vector<std::string> fields;
    //     fields.push_back("field" + this->NextFieldName());
    //     db_.Read(table, key, &fields, result);
    // } else {
    //     db_.Read(table, key, NULL, result);
    // }

    // std::vector<DB::KVPair> values;
    // if (this->write_all_fields()) {
    //     this->BuildValues(values);
    // } else {
    //     this->BuildUpdate(values);
    // }
    // return db_.Update(table, key, values);
}

inline void DBWorkload::TransactionScan(uint8_t * buf, int len) {
    const std::string &key = this->NextTransactionKey();
    int scan_len = this->NextScanLength();
    struct db_message * msg = (struct db_message *)buf;
    // std::vector<std::vector<DB::KVPair>> result;

    std::string cmd = Scan(key, scan_len);
    strcpy(msg->cmd, cmd.c_str());

    msg->op = SCAN;
    msg->send_start = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();;
    msg->completion_time = 0;
}

inline void DBWorkload::TransactionUpdate(uint8_t * buf, int len) {
    const std::string &key = this->NextTransactionKey();
    std::string value = this->BuildValues();
    struct db_message * msg = (struct db_message *)buf;

    // msg->op_code = UPDATE;
    // memcpy(msg->key, key.c_str(), key.length());
    // memcpy(msg->value, value.c_str(), value.length());
    std::string cmd = Insert(key, value);
    strcpy(msg->cmd, cmd.c_str());

    msg->op = UPDATE;
    msg->send_start = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();;
    msg->completion_time = 0;
}

/* Insert is used to preload data */
inline void DBWorkload::TransactionInsert(uint8_t * buf, int len) {
    const std::string key = this->NextSequenceKey();
    std::string value = this->BuildValues();
    struct db_message * msg = (struct db_message *)buf;

    // msg->op_code = INSERT;
    // memcpy(msg->key, key.c_str(), key.length());
    // memcpy(msg->value, value.c_str(), value.length());
    std::string cmd = Insert(key, value);
    strcpy(msg->cmd, cmd.c_str());

    msg->op = INSERT;
    msg->send_start = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();;
    msg->completion_time = 0;
} 

#endif  /* _DB_WORKLOAD_H_ */