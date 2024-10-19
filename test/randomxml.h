
#pragma once

#include <random>
#include <string>
#include <pugixml.hpp>

std::string RandomString(int length);
void GenerateXML(pugi::xml_node& parent, int& node_count, int depth);
