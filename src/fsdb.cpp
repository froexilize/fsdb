#include <assert.h>
#include <filesystem>
#include "fsdb/fsdb.hpp"

bool save_buffer_to_file(const char *cFileName,
                         const char *blob,
                         const size_t cBlobSz) {
    bool result = false;
    if (FILE *f = fopen(cFileName, "w+b")) {
        if(fwrite(blob, 1, cBlobSz, f) == cBlobSz) {
            result = true;
            // File write success
        }
        else {
            // File write error
        }
        fclose(f);
    }
    else {
        // file open error
    }
    return result;
}

bool Fsdb::init() {
	if (!init_db_directory()) {
	    return false; //< TODO: return error code
	}
    m_initialized = true;
	return initialized();
}

bool Fsdb::initialized() {
	return m_initialized;
}

bool Fsdb::deinit() {
    m_initialized = false;
    return true;
}

std::string Fsdb::get_name() {
    return m_name;
}

bool Fsdb::init_db_directory() {
    if (std::filesystem::exists(m_name)) {
		auto dir_entry = std::filesystem::directory_entry(m_name);
        return dir_entry.is_directory();
    }
    else {
        return std::filesystem::create_directories(m_name);
    }
}

std::filesystem::path Fsdb::get_db_record_path(std::string_view key) {
    std::filesystem::path key_path(m_name);
    return key_path.append(key);
}

bool Fsdb::insert(std::string_view key, const char *pdata, const size_t data_sz) {
    if (!initialized()) return false;
    auto key_path = get_db_record_path(key);
    //if (std::filesystem::exists(key_path)) {
    //    return false;
    //}
    if (!save_buffer_to_file(key_path.string().c_str(), "value", 5)) {
        return false;
    }
    return true;
}

