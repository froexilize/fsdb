#include "gmock/gmock.h"
#include <filesystem>
#include <fsdb/fsdb.hpp>
#include <fstream>

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

TEST(FsdbInit, InitSetsName) {
	Fsdb fsdb;
	fsdb.init("not_default");
	EXPECT_STREQ(fsdb.get_name().c_str(), "not_default");
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
    auto db_rec_path = fsdb.get_db_record_path(key);
    EXPECT_TRUE(std::filesystem::exists(db_rec_path));
}

TEST(FsdbObtain, ObtainGetsCorrectValue) {
	Fsdb fsdb;
	fsdb.init();
	const std::string key1 = "super key1";
	const std::string value1 = "value1";
	const std::string key2 = "super key2";
	const std::string value2 = "value2";
	fsdb.insert(key1, value1.c_str(), value1.length());
	fsdb.insert(key2, value2.c_str(), value2.length());
	unsigned char *b1 = nullptr;
	unsigned char *b2 = nullptr;
	auto r1 = fsdb.obtain(key1, &b1);
	auto r2 = fsdb.obtain(key2, &b2);
	EXPECT_TRUE(r1);
	EXPECT_TRUE(r2);
	// TODO: Bug: SIGSEGV on nullptr

	EXPECT_EQ(memcmp(value1.c_str(), b1, value1.length()), 0);
	EXPECT_EQ(memcmp(value2.c_str(), b2, value2.length()), 0);
	delete[] b1;
	delete[] b2;
}

TEST(FsdbObtain, ObtainingunexistingValueReturnsFalse) {
	Fsdb fsdb;
	fsdb.init();
	const std::string key = "non_existing";
	unsigned char *value = nullptr;
	EXPECT_FALSE(fsdb.obtain(key, &value));
	EXPECT_TRUE(value == nullptr);
	delete[] value;
}

TEST(FsdbDel, DelRemovesValue) {
	Fsdb fsdb;
	fsdb.init();
	const std::string key = "my key";
	const std::string value = "my value";
	unsigned char *buffer = nullptr;
	EXPECT_FALSE(fsdb.obtain(key, &buffer));
	// TODO: Memory leak if not nullptr, but who knows?
	EXPECT_TRUE(buffer == nullptr);
	fsdb.insert(key, value.c_str(), value.length());
	EXPECT_TRUE(fsdb.obtain(key, &buffer));
	fsdb.del(key);
	EXPECT_FALSE(fsdb.obtain(key, &buffer));
	delete[] buffer;
}

TEST(FsdbDel, DelRemovesEmptyDirectories) {
	Fsdb fsdb;
	fsdb.init();
	const std::string key = "blah key";
	const std::string value = "blah value";
	auto db_rec_dir_path = fsdb.get_db_record_path(key).parent_path();
	EXPECT_FALSE(std::filesystem::exists(db_rec_dir_path));
	fsdb.insert(key, value.c_str(), value.length());
	EXPECT_TRUE(std::filesystem::exists(db_rec_dir_path));
	fsdb.del(key);
	EXPECT_FALSE(std::filesystem::exists(db_rec_dir_path));
}

TEST(FsdbDel, DelNotRemovesNotEmptyDirectories) {
	Fsdb fsdb;
	fsdb.init();
	const std::string key1 = "blah key1";
	const std::string key2 = "blah key2";
	const std::string value = "blah value";
	unsigned char *buffer = nullptr;
	auto db_rec_path1 = fsdb.get_db_record_path(key1).parent_path();
	auto db_rec_path2 = fsdb.get_db_record_path(key2).parent_path();
	EXPECT_EQ(db_rec_path1, db_rec_path2);
	EXPECT_FALSE(std::filesystem::exists(db_rec_path1));
	fsdb.insert(key1, value.c_str(), value.length());
	fsdb.insert(key2, value.c_str(), value.length());
	EXPECT_TRUE(std::filesystem::exists(db_rec_path1));
	fsdb.del(key1);
	EXPECT_TRUE(std::filesystem::exists(db_rec_path1));
	fsdb.del(key2);
	EXPECT_FALSE(std::filesystem::exists(db_rec_path2));
}
