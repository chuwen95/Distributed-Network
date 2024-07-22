//
// Created by ChuWen on 2024/3/23.
//

#include "StorageInitializer.h"

#include "csm-utilities/Logger.h"

using namespace csm::initializer;

StorageInitializer::StorageInitializer(const std::string &path) :
    m_storagePath(path)
{}

int StorageInitializer::init()
{

}

csm::storage::Storage::Ptr StorageInitializer::storage()
{
    return m_storage;
}