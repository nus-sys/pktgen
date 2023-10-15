#ifndef _DB_WORKLOAD_H_
#define _DB_WORKLOAD_H_

#include <vector>
#include <string>

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

struct db_message {
    uint8_t key[64];
    uint8_t value[1024];
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
        ordered_inserts_(true), record_count_(0), op_count_(0) {
    }
    
    virtual ~DBWorkload() {
        if (field_len_generator_) delete field_len_generator_;
        if (key_generator_) delete key_generator_;
        if (key_chooser_) delete key_chooser_;
        if (scan_len_chooser_) delete scan_len_chooser_;
    }

    uint16_t GenerateNextReq(uint8_t * pkt, int len) {
        Operation nextOp = this->NextOperation();
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
        return 0;
    }

    void PrintResult(uint64_t duration) {
        return;
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

inline void DBWorkload::TransactionRead(uint8_t * buf, int len) {
    const std::string &key = this->NextTransactionKey();
    struct db_message * msg = (struct db_message *)buf;

    // msg->op_code = READ;
    memcpy(msg->key, key.c_str(), key.length());
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
    // const std::string &key = this->NextTransactionKey();
    // int len = this->NextScanLength();
    // std::vector<std::vector<DB::KVPair>> result;
    // if (!this->read_all_fields()) {
    //     std::vector<std::string> fields;
    //     fields.push_back("field" + this->NextFieldName());
    //     return db_.Scan(table, key, len, &fields, result);
    // } else {
    //     return db_.Scan(table, key, len, NULL, result);
    // }
}

inline void DBWorkload::TransactionUpdate(uint8_t * buf, int len) {
    const std::string &key = this->NextTransactionKey();
    const std::string &value = this->BuildValues();
    struct db_message * msg = (struct db_message *)buf;

    // msg->op_code = UPDATE;
    memcpy(msg->key, key.c_str(), key.length());
    memcpy(msg->value, value.c_str(), value.length());
}

/* Insert is used to preload data */
inline void DBWorkload::TransactionInsert(uint8_t * buf, int len) {
    const std::string &key = this->NextSequenceKey();
    const std::string &value = this->BuildValues();
    struct db_message * msg = (struct db_message *)buf;

    // msg->op_code = INSERT;
    memcpy(msg->key, key.c_str(), key.length());
    memcpy(msg->value, value.c_str(), value.length());
} 

#endif  /* _DB_WORKLOAD_H_ */