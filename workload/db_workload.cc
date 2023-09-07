#include "db_workload.h"
#include "properties.h"

#include "core/const_generator.h"
#include "core/uniform_generator.h"
#include "core/zipfian_generator.h"
#include "core/scrambled_zipfian_generator.h"
#include "core/skewed_latest_generator.h"

const std::string DBWorkload::FIELD_LENGTH_DISTRIBUTION_PROPERTY = "field_len_dist";
const std::string DBWorkload::FIELD_LENGTH_DISTRIBUTION_DEFAULT = "constant";

const std::string DBWorkload::FIELD_LENGTH_PROPERTY = "fieldlength";
const std::string DBWorkload::FIELD_LENGTH_DEFAULT = "100";

const std::string DBWorkload::REQUEST_DISTRIBUTION_PROPERTY = "requestdistribution";
const std::string DBWorkload::REQUEST_DISTRIBUTION_DEFAULT = "uniform";

const std::string DBWorkload::MAX_SCAN_LENGTH_PROPERTY = "maxscanlength";
const std::string DBWorkload::MAX_SCAN_LENGTH_DEFAULT = "1000";

const std::string DBWorkload::SCAN_LENGTH_DISTRIBUTION_PROPERTY = "scanlengthdistribution";
const std::string DBWorkload::SCAN_LENGTH_DISTRIBUTION_DEFAULT = "uniform";

const std::string DBWorkload::READ_PROPORTION_PROPERTY = "readproportion";
const std::string DBWorkload::READ_PROPORTION_DEFAULT = "0.95";

const std::string DBWorkload::UPDATE_PROPORTION_PROPERTY = "updateproportion";
const std::string DBWorkload::UPDATE_PROPORTION_DEFAULT = "0.05";

const std::string DBWorkload::INSERT_PROPORTION_PROPERTY = "insertproportion";
const std::string DBWorkload::INSERT_PROPORTION_DEFAULT = "0.0";

const std::string DBWorkload::SCAN_PROPORTION_PROPERTY = "scanproportion";
const std::string DBWorkload::SCAN_PROPORTION_DEFAULT = "0.0";

const std::string DBWorkload::READMODIFYWRITE_PROPORTION_PROPERTY = "readmodifywriteproportion";
const std::string DBWorkload::READMODIFYWRITE_PROPORTION_DEFAULT = "0.0";

const std::string DBWorkload::INSERT_ORDER_PROPERTY = "insertorder";
const std::string DBWorkload::INSERT_ORDER_DEFAULT = "hashed";

const std::string DBWorkload::INSERT_START_PROPERTY = "insertstart";
const std::string DBWorkload::INSERT_START_DEFAULT = "0";

const std::string DBWorkload::RECORD_COUNT_PROPERTY = "recordcount";
const std::string DBWorkload::OPERATION_COUNT_PROPERTY = "operationcount";

inline void DBWorkload::Init(const Properties &p) {
    field_len_generator_ = GetFieldLenGenerator(p);

    std::string request_dist = p.GetProperty(REQUEST_DISTRIBUTION_PROPERTY,
                                            REQUEST_DISTRIBUTION_DEFAULT);

    double read_proportion = std::stod(p.GetProperty(READ_PROPORTION_PROPERTY,
                                                    READ_PROPORTION_DEFAULT));
    double update_proportion = std::stod(p.GetProperty(UPDATE_PROPORTION_PROPERTY,
                                                    UPDATE_PROPORTION_DEFAULT));
    double insert_proportion = std::stod(p.GetProperty(INSERT_PROPORTION_PROPERTY,
                                                    INSERT_PROPORTION_DEFAULT));
    double scan_proportion = std::stod(p.GetProperty(SCAN_PROPORTION_PROPERTY,
                                                    SCAN_PROPORTION_DEFAULT));
    double readmodifywrite_proportion = std::stod(p.GetProperty(READMODIFYWRITE_PROPORTION_PROPERTY, 
                                                    READMODIFYWRITE_PROPORTION_DEFAULT));

    int max_scan_len = std::stoi(p.GetProperty(MAX_SCAN_LENGTH_PROPERTY,
                                            MAX_SCAN_LENGTH_DEFAULT));
    std::string scan_len_dist = p.GetProperty(SCAN_LENGTH_DISTRIBUTION_PROPERTY,
                                            SCAN_LENGTH_DISTRIBUTION_DEFAULT);

    int insert_start = std::stoi(p.GetProperty(INSERT_START_PROPERTY,
                                            INSERT_START_DEFAULT));

    key_generator_ = new CounterGenerator(insert_start);

    if (read_proportion > 0) {
        op_chooser_.AddValue(READ, read_proportion);
    }
    if (update_proportion > 0) {
        op_chooser_.AddValue(UPDATE, update_proportion);
    }
    if (insert_proportion > 0) {
        op_chooser_.AddValue(INSERT, insert_proportion);
    }
    if (scan_proportion > 0) {
        op_chooser_.AddValue(SCAN, scan_proportion);
    }
    if (readmodifywrite_proportion > 0) {
        op_chooser_.AddValue(READMODIFYWRITE, readmodifywrite_proportion);
    }

    if (request_dist == "uniform") {
        key_chooser_ = new UniformGenerator(0, record_count_ - 1);
    } else if (request_dist == "zipfian") {
        int op_count = std::stoi(p.GetProperty(OPERATION_COUNT_PROPERTY));
        int new_keys = (int)(op_count * insert_proportion * 2); // a fudge factor
        key_chooser_ = new ScrambledZipfianGenerator(record_count_ + new_keys);
    } else if (request_dist == "latest") {
        key_chooser_ = new SkewedLatestGenerator(insert_key_sequence_);
    } else {
        throw Exception("Unknown request distribution: " + request_dist);
    }

    if (scan_len_dist == "uniform") {
        scan_len_chooser_ = new UniformGenerator(1, max_scan_len);
    } else if (scan_len_dist == "zipfian") {
        scan_len_chooser_ = new ZipfianGenerator(1, max_scan_len);
    } else {
        throw Exception("Distribution not allowed for scan length: " + scan_len_dist);
    }
}

std::string DBWorkload::BuildValues(void) {
    std::string value;
    value.append(field_len_generator_->Next(), RandomPrintChar());
    return value;
}

std::string DBWorkload::BuildUpdate(void) {
    std::string value;
    value.append(field_len_generator_->Next(), RandomPrintChar());
    return value;
}

inline std::string DBWorkload::BuildKeyName(uint64_t key_num) {
    if (!ordered_inserts_) {
        key_num = Hash(key_num);
    }
    std::string key_num_str = std::to_string(key_num);
    int zeros = zero_padding_ - key_num_str.length();
    zeros = std::max(0, zeros);
    return std::string("user").append(zeros, '0').append(key_num_str);
}

Generator<uint64_t> * DBWorkload::GetFieldLenGenerator(const Properties &p) {
    std::string field_len_dist = p.GetProperty(FIELD_LENGTH_DISTRIBUTION_PROPERTY,
                                            FIELD_LENGTH_DISTRIBUTION_DEFAULT);
    int field_len = std::stoi(p.GetProperty(FIELD_LENGTH_PROPERTY,
                                            FIELD_LENGTH_DEFAULT));
    if (field_len_dist == "constant") {
        return new ConstGenerator(field_len);
    } else if (field_len_dist == "uniform") {
        return new UniformGenerator(1, field_len);
    } else if (field_len_dist == "zipfian") {
        return new ZipfianGenerator(1, field_len);
    } else {
        throw Exception("Unknown field length distribution: " + field_len_dist);
    }
}