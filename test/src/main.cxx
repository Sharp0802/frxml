#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <frxml.h>
#include <libxml2/libxml/parser.h>
#include <pugixml.hpp>
#include <benchmark/benchmark.h>
#include <tinyxml2.h>
#include <sstream>
#include <rapidxml/rapidxml.hpp>

#include "randomxml.h"

class CustomMemoryManager : public benchmark::MemoryManager
{
public:
    size_t m_allocated  = 0;
    size_t m_nAllocated = 0;

    void Start() BENCHMARK_OVERRIDE
    {
        m_allocated  = 0;
        m_nAllocated = 0;
    }

    void Stop(Result& result) BENCHMARK_OVERRIDE
    {
        result.num_allocs     = static_cast<int64_t>(m_nAllocated);
        result.max_bytes_used = static_cast<int64_t>(m_allocated);
    }
};

static CustomMemoryManager g_memoryManager;

extern "C" void* __real_malloc(size_t size);

extern "C" void* __wrap_malloc(size_t size)
{
    g_memoryManager.m_allocated += size;
    g_memoryManager.m_nAllocated++;

    return __real_malloc(size);
}

void* operator new(std::size_t size)
{
    return __wrap_malloc(size);
}

void operator delete(void* p) noexcept
{
    free(p);
}

std::map<size_t, std::string> g_vXML;

static void PrepareXMLs()
{
    static bool first = true;
    if (!first) return;
    first = false;

    for (auto i = 50; i <= 1000; i += 10)
    {
        pugi::xml_document doc;
        pugi::xml_node     root = doc.append_child("root");

        auto copy = i;

        GenerateXML(root, copy, 300);

        std::stringstream ss;
        doc.print(ss);

        auto str = ss.str();

        g_vXML.emplace(str.size(), str);
    }
}

static void BenchmarkArguments(benchmark::internal::Benchmark* bm)
{
    PrepareXMLs();
    for (const auto& [size, _]: g_vXML)
        bm->Args({ static_cast<int64_t>(size) });
}

static std::string& PrepareBenchmark(benchmark::State& state)
{
    return g_vXML[state.range(0)];
}

static void BM_frxml(benchmark::State& state)
{
    auto str = PrepareBenchmark(state);

    for (auto _: state)
    {
        frxml::doc doc{ std::string_view(str.data(), str.size()) };
        if (!doc)
            state.SkipWithError(std::to_string(doc.exception()));
        //if (!doc.validate())
        //    state.SkipWithError(std::to_string(doc.exception()));
    }
}

static void BM_libxml2(benchmark::State& state)
{
    auto str = PrepareBenchmark(state);

    for (auto _: state)
    {
        auto doc = xmlReadMemory(str.data(), str.size(), nullptr, nullptr, XML_PARSE_HUGE);
        if (!doc)
            state.SkipWithError("Failed to parsing");
        else
            xmlFreeDoc(doc);
    }
}

static void BM_rapidxml(benchmark::State& state)
{
    auto str = PrepareBenchmark(state);

    for (auto _: state)
    {
        // coping is required: rapidxml changes source string
        std::string copy = str;

        try
        {
            rapidxml::xml_document<> doc;
            doc.parse<rapidxml::parse_full>(copy.data());
        }
        catch (const std::exception& e)
        {
            state.SkipWithError(str);
        }
    }
}

static void BM_pugixml(benchmark::State& state)
{
    auto str = PrepareBenchmark(state);

    for (auto _: state)
    {
        pugi::xml_document doc;
        if (auto result = doc.load_string(str.data(), pugi::parse_full | pugi::encoding_utf8); !result)
            state.SkipWithError(result.description());
    }
}

static void BM_tinyxml2(benchmark::State& state)
{
    auto str = PrepareBenchmark(state);

    for (auto _: state)
    {
        tinyxml2::XMLDocument a;
        a.Parse(str.data(), str.size());
        if (a.Error())
            state.SkipWithError(a.ErrorStr());
    }
}

#define ITER 3

BENCHMARK(BM_rapidxml)->Apply(BenchmarkArguments)->Iterations(ITER);
BENCHMARK(BM_frxml)->Apply(BenchmarkArguments)->Iterations(ITER);
BENCHMARK(BM_pugixml)->Apply(BenchmarkArguments)->Iterations(ITER);
BENCHMARK(BM_tinyxml2)->Apply(BenchmarkArguments)->Iterations(ITER);
BENCHMARK(BM_libxml2)->Apply(BenchmarkArguments)->Iterations(ITER);

int main(int argc, char** argv)
{
    srand(time(nullptr));

    xmlMemSetup(free, __wrap_malloc, realloc, strdup);
    pugi::set_memory_management_functions(__wrap_malloc, free);

    benchmark::RegisterMemoryManager(&g_memoryManager);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;

    benchmark::RunSpecifiedBenchmarks();
    benchmark::RegisterMemoryManager(nullptr);
    benchmark::Shutdown();

    return 0;
}
