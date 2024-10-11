#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <frxml.h>
#include <libxml2/libxml/parser.h>
#include <pugixml.hpp>
#include <ranges>
#include <algorithm>

timespec diff(timespec start, timespec end)
{
    timespec temp{};
    if (end.tv_nsec - start.tv_nsec < 0)
    {
        temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec  = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

double todouble(timespec ts)
{
    ts.tv_nsec /= 100;
    return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 10000000.0;
}

double benchmark(void (*fn)())
{
    timespec start{}, end{};
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    for (int i = 0; i < 1000; ++i) fn();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    return todouble(diff(start, end));
}

int main()
{
    static int     code = 0;
    static char    buf[1048576];
    static ssize_t size;

    std::cout << "Size (KB),\tDepth,\tFRXML (ms),\tPugiXML (ms),\tLibXml2 (DOM, ms)" << std::endl;

    for (std::string file: {
             "1, 3",
             "10, 3",
             "50, 3",
             "1, 10",
             "10, 10",
             "50, 10"
         })
    {
        const auto fd = open((file + ".xml").c_str(), O_RDONLY);
        if (fd < 0)
        {
            perror("open");
            continue;
        }

        size                = read(fd, buf, sizeof buf);
        buf[sizeof buf - 1] = 0;

        auto frxml = benchmark([]
        {
            if (const frxml::doc doc{ std::string_view(buf, size) }; !doc)
                code = doc.error().code;
        });

        double libxml;
        if (size > 10240)
        {
            libxml = -1;
        }
        else
        {
            libxml = benchmark([]
            {
                auto doc = xmlReadMemory(buf, size, nullptr, nullptr, 0);
                xmlFreeDoc(doc);
                xmlCleanupParser();
            });
        }

        auto pugixml = benchmark([]
        {
            pugi::xml_document doc;
            [[maybe_unused]]
                pugi::xml_parse_result result = doc.load_string(buf, pugi::parse_default | pugi::encoding_utf8);
        });

        std::cout << file << ",\t" << frxml << ",\t" << pugixml << ",\t" << libxml << std::endl;

        if (code != 0)
            std::cerr << "Exited with code: " << code << std::endl;

        close(fd);
    }

    return 0;
}
