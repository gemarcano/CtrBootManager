#include <gtest/gtest.h>
#include <config.h>

TEST(ctrbm_config_test, config_init)
{
	ctrbm_config config;
	ctrbm_config_init(&config);

	EXPECT_EQ((time_t)0, config.timeout);
	EXPECT_EQ((uint8_t)0, config.autobootfix);
	EXPECT_EQ((uint8_t)0, config.default_entry);
	EXPECT_EQ((uint32_t)0, config.menu_key);
	EXPECT_EQ((size_t)0, config.n_entries);
}

TEST(ctrbm_config_test, config_entry_init)
{
	ctrbm_config_entry entry;
	ctrbm_config_entry_init(&entry, "Title", "/", 0xFF, 0x12000);
	
	ASSERT_STREQ("Title", entry.title);
	ASSERT_STREQ("/", entry.path);
	EXPECT_EQ((uint32_t)0xFF, entry.auto_key);
	EXPECT_EQ((uintptr_t)0x12000, entry.offset);
	ctrbm_config_entry_destroy(&entry);
}

TEST(ctrbm_config_test, config_add_entry)
{
	ctrbm_config config;
	ctrbm_config_init(&config);
	ctrbm_config_add_entry(&config, "Title", "/", 0xFF, 0x12000);

	ASSERT_STREQ("Title", config.entries[0].title);
	ASSERT_STREQ("/", config.entries[0].path);
	EXPECT_EQ((uint32_t)0xFF, config.entries[0].auto_key);
	EXPECT_EQ((uintptr_t)0x12000, config.entries[0].offset);
	
	EXPECT_EQ(1u, config.n_entries);

	ctrbm_config_add_entry(&config, "Title2", "/2", 0xF1, 0x12001);
	ASSERT_STREQ("Title2", config.entries[1].title);
	ASSERT_STREQ("/2", config.entries[1].path);
	EXPECT_EQ((uint32_t)0xF1, config.entries[1].auto_key);
	EXPECT_EQ((uintptr_t)0x12001, config.entries[1].offset);

	EXPECT_EQ(2u, config.n_entries);
	ctrbm_config_destroy(&config);
}

TEST(ctrbm_config_test, config_destroy)
{
	ctrbm_config config;
	ctrbm_config_init(&config);
	ctrbm_config_add_entry(&config, "Title", "/", 0xFF, 0x12000);
	ctrbm_config_destroy(&config);

	EXPECT_EQ((time_t)0, config.timeout);
	EXPECT_EQ((uint8_t)0, config.autobootfix);
	EXPECT_EQ((uint8_t)0, config.default_entry);
	EXPECT_EQ((uint32_t)0, config.menu_key);
	EXPECT_EQ((size_t)0, config.n_entries);
}

TEST(ctrbm_config_test, config_remove_entry)
{
	ctrbm_config config;
        ctrbm_config_init(&config);
        ctrbm_config_add_entry(&config, "Title", "/", 0xFF, 0x12000);
        ctrbm_config_add_entry(&config, "Title2", "/2", 0xF1, 0x12001);
        
	ctrbm_config_remove_entry(&config, 0, NULL);
	ASSERT_STREQ("Title2", config.entries[0].title);
        ASSERT_STREQ("/2", config.entries[0].path);
        EXPECT_EQ((uint32_t)0xF1, config.entries[0].auto_key);
        EXPECT_EQ((uintptr_t)0x12001, config.entries[0].offset);
	EXPECT_EQ(1u, config.n_entries);

	ctrbm_config_destroy(&config);
}

TEST(ctrbm_config_test, config_set_defaults)
{
	ctrbm_config config;
        ctrbm_config_init(&config);
	ctrbm_config_set_defaults(&config);

	EXPECT_EQ((time_t)3, config.timeout);
	EXPECT_EQ((uint8_t)8, config.autobootfix);
	EXPECT_EQ((uint8_t)0, config.default_entry);
	EXPECT_EQ((uint32_t)4, config.menu_key);
	EXPECT_EQ((size_t)0, config.n_entries);

	ctrbm_config_destroy(&config);
}

TEST(ctrbm_config_test, config_read_from_disk)
{
	ctrbm_config config;
        ctrbm_config_init(&config);

	ctrbm_config_read_from_disk(&config, "boot_1.cfg");

	EXPECT_EQ((time_t)5, config.timeout);
	EXPECT_EQ((uint8_t)3, config.autobootfix);
	EXPECT_EQ((uint8_t)1, config.default_entry);
	EXPECT_EQ((uint32_t)2, config.menu_key);
	EXPECT_EQ((size_t)3, config.n_entries);

	ASSERT_STREQ("rxTools", config.entries[0].title);
	ASSERT_STREQ("/rxTools/sys/code.bin", config.entries[0].path);
	EXPECT_EQ((uint32_t)0, config.entries[0].auto_key);
	EXPECT_EQ((uintptr_t)0x12000, config.entries[0].offset);

	ASSERT_STREQ("PastaCFW", config.entries[1].title);
	ASSERT_STREQ("/3ds/PastaCFW/PastaCFW.3dsx", config.entries[1].path);
	EXPECT_EQ((uint32_t)0, config.entries[1].auto_key);
	EXPECT_EQ((uintptr_t)0, config.entries[1].offset);

	ASSERT_STREQ("HomeBrewMenu", config.entries[2].title);
	ASSERT_STREQ("/boot_hb.3dsx", config.entries[2].path);
	EXPECT_EQ((uint32_t)0, config.entries[2].auto_key);
	EXPECT_EQ((uintptr_t)0, config.entries[2].offset);


	ctrbm_config_destroy(&config);
}

TEST(ctrbm_config_test, config_write_to_disk)
{
	ctrbm_config config;
        ctrbm_config_init(&config);

	ctrbm_config_set_defaults(&config);
	ctrbm_config_add_entry(&config, "rxToolz", "/was", 5u, (uintptr_t)0x44);

	ctrbm_config_write_to_disk(&config, "boot_tmp.cfg");
	ctrbm_config_destroy(&config);

	ctrbm_config_init(&config);
	ctrbm_config_read_from_disk(&config, "boot_tmp.cfg");

	EXPECT_EQ((time_t)3, config.timeout);
	EXPECT_EQ((uint8_t)8, config.autobootfix);
	EXPECT_EQ((uint8_t)0, config.default_entry);
	EXPECT_EQ((uint32_t)4, config.menu_key);
	EXPECT_EQ((size_t)1, config.n_entries);

	ASSERT_STREQ("rxToolz", config.entries[0].title);
	ASSERT_STREQ("/was", config.entries[0].path);
	EXPECT_EQ((uint32_t)5, config.entries[0].auto_key);
	EXPECT_EQ((uintptr_t)0x44, config.entries[0].offset);

	ctrbm_config_destroy(&config);
	remove("boot_tmp.cfg");
}

int main (int argc, char *argv[])
{
	printf("Running main() from gtest_main.cc\n");
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
