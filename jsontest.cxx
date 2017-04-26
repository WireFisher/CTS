#include <iostream>
#include <cstdio>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

int main()
{
    std::ifstream ifp("demo.cfg");
    json j;
    std::string filename;
    int num;

    ifp >> j;
    //num = j["grid_pair_auto"]["grid_filename"].size();
    //filename = j["grid_pair_auto"]["grid_filename"][0];
    //printf("the Dimension is: %d\n", dimen);
    //printf("num: %d\n", num);
    //printf("grid_filename: %s\n", filename.c_str());
    std::string str_obj;
    str_obj = j["strategy"][1]["fuck"];
    return 0;

}
