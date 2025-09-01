#include <array>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "ubench.h"
#include "frxml/frxml.h"

struct testcase {
  int fd = -1;
  std::string_view content = {nullptr, 0};
};

constexpr size_t N_TESTCASES = 2;

std::array<testcase, N_TESTCASES> testcases;

void dispose() {
  for (auto i = 0; i < N_TESTCASES; i++) {
    auto [fd, content] = testcases[i];

    if (content.data()) {
      munmap(const_cast<char*>(content.data()), content.size());
      testcases[i].content = {nullptr, 0};
    }

    if (fd != -1) {
      close(fd);
      testcases[i].fd = -1;
    }
  }
}

void prepare() {
  char base[PATH_MAX];
  if (readlink("/proc/self/exe", base, sizeof(base)) == -1) {
    perror("/proc/self/exe");
    return;
  }

  const auto dir = dirname(base);

  char buffer[PATH_MAX];

  bool success = false;
  for (auto i = 0; i < N_TESTCASES; ++i) {
    success = false;

    const auto size = snprintf(buffer, sizeof buffer, "%s/%d.xml", dir, i);
    buffer[size] = 0;

    const auto fd = open(buffer, O_RDONLY);
    if (fd == -1) {
      perror(buffer);
      break;
    }

    testcases[i].fd = fd;

    struct stat st{};
    if (fstat(fd, &st) < 0) {
      perror(buffer);
      break;
    }

    const auto p = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
      perror(buffer);
      break;
    }

    testcases[i].content = std::string_view(static_cast<char*>(p), st.st_size);
    success = true;
  }
  if (!success) {
    dispose();
  }
}

void prepare(ubench::args &args) {
  args = {0, 1};
}

void cb(std::vector<uint8_t> &context, const void *p, const size_t len) {
  const auto offset = context.size();
  context.resize(offset + len);
  memcpy(context.data() + offset, p, len);
}

void dummy_cb(void *, const void *, size_t) {
}

void dump_cb(void *, const void *p, size_t) {
  frxml::dump(std::cout, p);
}

[[clang::noinline]]
void BM_frxml_zero_alloc(const ubench::arg arg) {
  frxml::parse<dummy_cb, void*>(testcases[arg].content, nullptr);
}

[[clang::noinline]]
void BM_frxml_one_alloc(const ubench::arg arg) {
  std::vector<uint8_t> buffer;
  frxml::parse<cb, std::vector<uint8_t>>(testcases[arg].content, buffer);
}

BENCHMARK(BM_frxml_zero_alloc).prepare(prepare);
BENCHMARK(BM_frxml_one_alloc).prepare(prepare);

int main() {
  prepare();

  frxml::parse<dump_cb, void*>(testcases[0].content, nullptr);

  //ubench::print(ubench::run());
  dispose();
  return 0;
}
