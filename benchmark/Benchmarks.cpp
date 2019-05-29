#include "benchmark/benchmark.h"

#include <string>

/* Constants */
const int kNumReplicas[] = {3, 5};
const int kNumClients[] = {1, 2, 3};
const int kDropRates[] = {0, 1, 5};
const int kDropRateDenom = 1000; 

/* Benchmark Fixtures */

class TCPMenciusFixture : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State &state) {
  }

  void TearDown(const ::benchmark::State &state) {
  }
};

class UDPMenciusFixture : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State &state) {
  }

  void TearDown(const ::benchmark::State &state) {
  }
};

/* Benchmark Arguments */

static void TestArguments(benchmark::internal::Benchmark *b) {
  for (int num_replicas : kNumReplicas) {
    for (int num_clients : kNumClients) {
      for (int drop_rate : kDropRates) {
        b->Args({num_replicas, num_clients, drop_rate});
      }
    }
  }
}

/* Benchmarks */

BENCHMARK_DEFINE_F(TCPMenciusFixture, NoFailures)(benchmark::State &state) {
  auto num_replicas = state.range(0);
  auto num_clients = state.range(1);
  auto drop_rate = state.range(2) / kDropRateDenom;

  for (auto _ : state)
    std::string empty_string;
}
BENCHMARK_REGISTER_F(TCPMenciusFixture, NoFailures)->Apply(TestArguments);

BENCHMARK_DEFINE_F(TCPMenciusFixture, GradualFailures)(benchmark::State &state) {
  auto num_replicas = state.range(0);
  auto num_clients = state.range(1);
  auto drop_rate = state.range(2) / kDropRateDenom;

  for (auto _ : state)
    std::string empty_string;
}
BENCHMARK_REGISTER_F(TCPMenciusFixture, GradualFailures)->Apply(TestArguments);

BENCHMARK_DEFINE_F(TCPMenciusFixture, InstantFailures)(benchmark::State &state) {
  auto num_replicas = state.range(0);
  auto num_clients = state.range(1);
  auto drop_rate = state.range(2) / kDropRateDenom;

  for (auto _ : state)
    std::string empty_string;
}
BENCHMARK_REGISTER_F(TCPMenciusFixture, InstantFailures)->Apply(TestArguments);

BENCHMARK_DEFINE_F(UDPMenciusFixture, NoFailures)(benchmark::State &state) {
  auto num_replicas = state.range(0);
  auto num_clients = state.range(1);
  auto drop_rate = state.range(2) / kDropRateDenom;

  for (auto _ : state)
    std::string empty_string;
}
BENCHMARK_REGISTER_F(UDPMenciusFixture, NoFailures)->Apply(TestArguments);

BENCHMARK_DEFINE_F(UDPMenciusFixture, GradualFailures)(benchmark::State &state) {
  auto num_replicas = state.range(0);
  auto num_clients = state.range(1);
  auto drop_rate = state.range(2) / kDropRateDenom;

  for (auto _ : state)
    std::string empty_string;
}
BENCHMARK_REGISTER_F(UDPMenciusFixture, GradualFailures)->Apply(TestArguments);

BENCHMARK_DEFINE_F(UDPMenciusFixture, InstantFailures)(benchmark::State &state) {
  auto num_replicas = state.range(0);
  auto num_clients = state.range(1);
  auto drop_rate = state.range(2) / kDropRateDenom;

  for (auto _ : state)
    std::string empty_string;
}
BENCHMARK_REGISTER_F(UDPMenciusFixture, InstantFailures)->Apply(TestArguments);

BENCHMARK_MAIN();
