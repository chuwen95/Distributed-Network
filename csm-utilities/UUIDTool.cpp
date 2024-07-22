//
// Created by root on 10/9/23.
//

#include "UUIDTool.h"

#include "stduuid/uuid.h"

using namespace csm::utilities;

std::string UUIDTool::generate()
{
    std::random_device rd;
    auto seed_data = std::array<int, 6>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::ranlux48_base generator(seq);

    uuids::basic_uuid_random_generator<std::ranlux48_base> gen(&generator);
    uuids::uuid id = gen();
    return to_string(id);
}