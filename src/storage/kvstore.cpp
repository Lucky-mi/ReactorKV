// src/storage/kvstore.cpp
#include "storage/kvstore.h"

#include "base/logger.h"

namespace kvstore {

KVStore::KVStore(int maxLevel) : skiplist_(maxLevel) {
    LOG_INFO << "KVStore initialized with maxLevel=" << maxLevel;
}

KVStore::~KVStore() {
    LOG_INFO << "KVStore destroyed, size=" << skiplist_.size();
}

bool KVStore::put(const std::string& key, const std::string& value) {
    if (key.empty()) {
        LOG_WARN << "KVStore::put - empty key is not allowed";
        return false;
    }
    bool isNew = skiplist_.insert(key, value);
    LOG_DEBUG << "KVStore::put key=" << key << " isNew=" << isNew;
    return isNew;
}

bool KVStore::get(const std::string& key, std::string& value) const {
    if (key.empty()) {
        return false;
    }
    bool found = skiplist_.search(key, value);
    LOG_DEBUG << "KVStore::get key=" << key << " found=" << found;
    return found;
}

bool KVStore::del(const std::string& key) {
    if (key.empty()) {
        return false;
    }
    bool removed = skiplist_.remove(key);
    LOG_DEBUG << "KVStore::del key=" << key << " removed=" << removed;
    return removed;
}

bool KVStore::exists(const std::string& key) const {
    if (key.empty()) {
        return false;
    }
    return skiplist_.contains(key);
}

int KVStore::size() const {
    return skiplist_.size();
}

void KVStore::clear() {
    skiplist_.clear();
    LOG_INFO << "KVStore cleared";
}

bool KVStore::save(const std::string& filepath) const {
    if (filepath.empty()) {
        LOG_ERROR << "KVStore::save - empty filepath";
        return false;
    }

    bool success = skiplist_.dumpFile(filepath);
    if (success) {
        LOG_INFO << "KVStore saved to " << filepath << ", size=" << skiplist_.size();
    } else {
        LOG_ERROR << "KVStore save failed: " << filepath;
    }
    return success;
}

bool KVStore::load(const std::string& filepath) {
    if (filepath.empty()) {
        LOG_ERROR << "KVStore::load - empty filepath";
        return false;
    }

    // 先清空现有数据
    clear();

    bool success = skiplist_.loadFile(filepath);
    if (success) {
        LOG_INFO << "KVStore loaded from " << filepath << ", size=" << skiplist_.size();
    } else {
        LOG_ERROR << "KVStore load failed: " << filepath;
    }
    return success;
}

void KVStore::dump() const {
    skiplist_.displayList();
}

}  // namespace kvstore
