
#include "randomxml.h"

std::string RandomString(int length)
{
    const std::string characters = "abcdefghijklmnopqrstuvwxyz";
    std::string       result;
    for (int i = 0; i < length; ++i) {
        result += characters[rand() % characters.size()];
    }
    return result;
}

void GenerateXML(pugi::xml_node& parent, int& node_count)
{
    if (node_count <= 0) return;

    // Create a new node with a random name
    std::string    node_name = "node_" + RandomString(5);
    pugi::xml_node new_node  = parent.append_child(node_name.c_str());

    // Optionally, add an attribute to the node
    std::string attr_name                        = "attr_" + RandomString(3);
    std::string attr_value                       = RandomString(5);
    new_node.append_attribute(attr_name.c_str()) = attr_value.c_str();

    // Decrease the remaining node count
    node_count--;

    // Recursively add child nodes
    int num_children = rand() % 4; // Random number of child nodes (0-3)
    for (int i = 0; i < num_children && node_count > 0; ++i) {
        GenerateXML(new_node, node_count);
    }
}
