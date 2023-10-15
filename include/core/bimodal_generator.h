//
//  bimodal_generator.h
//

#ifndef _BIMODAL_GENERATOR_H_
#define _BIMODAL_GENERATOR_H_

#include "generator.h"

#include <atomic>
#include <mutex>
#include <random>

class BimodalGenerator : public Generator<uint64_t> {
 public:

  using normal_dist   = std::normal_distribution<>;
  using discrete_dist = std::discrete_distribution<std::size_t>;

  std::array<normal_dist, 2> G_;
  std::discrete_distribution<std::size_t> w_;

  BimodalGenerator(uint64_t first_modal, double first_prob, uint64_t second_modal, double second_prob) {
    generator_.seed(rand());
  
    G_ = std::array<normal_dist, 2>{
        normal_dist{(double)first_modal, 0.1}, // mean, stddev of G[0]
        normal_dist{(double)second_modal, 0.1}, // mean, stddev of G[1]
    };

    w_ = discrete_dist{
        first_prob, // weight of G[0]
        second_prob, // weight of G[1]
    };
  }
  
  uint64_t Next();
  uint64_t Last();

private:
  std::mt19937_64 generator_;
  uint64_t last_int_;
};

inline uint64_t BimodalGenerator::Next() {
  auto index = w_(generator_);
  auto sample = G_[index](generator_);
  return (uint64_t)ceil(sample);
}

inline uint64_t BimodalGenerator::Last() {
  return last_int_;
}

#endif // _BIMODAL_GENERATOR_H_
