#include <cstring>
#include <filesystem>
#include <fstream>
#include <pugixml.hpp>
#include <tinyxml2.h>
#include <benchmark/benchmark.h>

#include "frxml/frxml.h"

#define PARSE_XML(nm, ...) \
  static void BM_##nm(benchmark::State& state, const std::string& filepath) {         \
    std::string xml;                                                                  \
    uintmax_t file_size = 0;                                                          \
                                                                                      \
    try {                                                                             \
      file_size = std::filesystem::file_size(filepath);                               \
      if (std::ifstream file(filepath); file) {                                       \
        xml.assign(std::istreambuf_iterator(file), std::istreambuf_iterator<char>()); \
      } else {                                                                        \
        state.SkipWithError("Could not open file: " + filepath);                      \
        return;                                                                       \
      }                                                                               \
    } catch (const std::filesystem::filesystem_error& e) {                            \
      state.SkipWithError("Filesystem error: " + std::string(e.what()));              \
      return;                                                                         \
    }                                                                                 \
                                                                                      \
    state.SetLabel(std::to_string(file_size));                                        \
                                                                                      \
    for (auto _ : state) { __VA_ARGS__ }                                              \
                                                                                      \
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations() * file_size));    \
  }

static void cb(std::vector<uint8_t> *context, const frxml::Node *node) {
  const auto offset = context->size();
  context->resize(offset + node->size());
  memcpy(context->data() + offset, node, node->size());
}

static void dummy_cb(void *, const frxml::Node *) {
}

PARSE_XML(
  strlen, {
  benchmark::DoNotOptimize(strlen(xml.data()));
  })

PARSE_XML(
  frxml_dom, {
  std::vector<uint8_t> buffer;
  auto result = frxml::parse<cb>(xml, &buffer);
  benchmark::DoNotOptimize(result);
  benchmark::ClobberMemory();
  });

PARSE_XML(
  frxml_sax, {
  auto result = frxml::parse<dummy_cb, void>(xml, nullptr);
  benchmark::DoNotOptimize(result);
  benchmark::ClobberMemory();
  });

PARSE_XML(
  pugixml, {
  state.PauseTiming();
  std::string copy = xml;
  state.ResumeTiming();
  pugi::xml_document doc;
  auto result = doc.load_buffer_inplace(copy.data(), copy.size());
  benchmark::DoNotOptimize(result);
  benchmark::ClobberMemory();
  });

PARSE_XML(
  tinyxml2, {
  tinyxml2::XMLDocument doc;
  auto result = doc.Parse(xml.data(), xml.size());
  benchmark::DoNotOptimize(result);
  benchmark::ClobberMemory();
  });

const std::vector<std::string> xml_files{
  "321gone.xml",
  "mondial-3.0_pv.xml",
  "part_pv.xml",
  "reed_pv.xml",
  "SigmodRecord_pv.xml"
};

using BenchmarkFn = void(*)(benchmark::State &, const std::string &);

const std::vector<std::pair<std::string, BenchmarkFn>> benchmarks_to_run = {
  {"strlen", BM_strlen},
  {"frxml_dom", BM_frxml_dom},
  {"pugixml", BM_pugixml},
  {"frxml_sax", BM_frxml_sax},
  {"tinyxml2", BM_tinyxml2}
};

int main(int argc, char **argv) {
  for (const auto &[name, fn] : benchmarks_to_run) {
    for (const auto &path : xml_files) {
      std::string full_benchmark_name = name + "/" + path;
      benchmark::RegisterBenchmark(full_benchmark_name, fn, path);
    }
  }

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
