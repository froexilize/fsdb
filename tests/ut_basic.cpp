#include "gmock/gmock.h"
#include <filesystem>
#include <fsdb/fsdb.hpp>

TEST(FsdbInit, ReturnsFalseIfNotInitialized) {
	Fsdb fsdb;
	bool result = fsdb.initialized();
	EXPECT_FALSE(result);
}

TEST(FsdbInit, ReturnsTrueIfInitialized) {
	Fsdb fsdb;
	fsdb.init();
	bool result = fsdb.initialized();
	EXPECT_TRUE(result);
}

TEST(FsdbInit, InitReturnsSameAsInitialized) {
	Fsdb fsdb;
	bool result = fsdb.init();
	bool expected = fsdb.initialized();
	EXPECT_EQ(result, expected);
}

class MockFsdb : public Fsdb {
public:
    MOCK_METHOD0(init, void());
    MOCK_METHOD0(deinit, void());
};

TEST(FsdbInit, InitShouldBeCalled) {
    auto fsdb = std::make_shared<MockFsdb>();
    EXPECT_CALL(*fsdb, init())
        .WillOnce(::testing::Invoke([&fsdb]() {
            fsdb->Fsdb::init();
        }));
    fsdb->init();
}

TEST(FsdbInit, InitSetsDefaultName) {
    Fsdb fsdb;
    fsdb.init();
    EXPECT_STREQ(fsdb.get_name().c_str(), "default");
}

TEST(FsdbInit, InitDbCreatesDirectory) {
    Fsdb fsdb;
    auto db_name = fsdb.get_name();
    if (std::filesystem::exists(db_name)) {
		std::filesystem::remove_all(db_name); //< preparing for test
    }
    EXPECT_FALSE(std::filesystem::exists(db_name));
    fsdb.init();
    EXPECT_TRUE(std::filesystem::exists(db_name));
}

TEST(FsdbInit, DeinitUnsetInitState) {
    Fsdb fsdb;
    fsdb.init();
    fsdb.deinit();
    EXPECT_FALSE(fsdb.initialized());
}

TEST(FsdbInsert, FailedToInsertWithoutInit) {
    Fsdb fsdb;
    const std::string key = "key";
    const std::string value = "value";
    EXPECT_FALSE(fsdb.insert(key, value.c_str(), value.length()));
}

TEST(FsdbInsert, CorrectInsertionReturnsTrue) {
    Fsdb fsdb;
    fsdb.init();
    const std::string key = "key";
    const std::string value = "value";
    EXPECT_TRUE(fsdb.insert(key, value.c_str(), value.length()));
}

TEST(FsdbInsert, InsertionCreatesFile) {
    Fsdb fsdb;
    fsdb.init();
    const std::string key = "key";
    const std::string value = "value";
    EXPECT_TRUE(fsdb.insert(key, value.c_str(), value.length()));
    std::filesystem::path key_file = fsdb.get_name();
    key_file.append(key);
    EXPECT_TRUE(std::filesystem::exists(key_file));
}

