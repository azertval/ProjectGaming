#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "HMI/Editor/LevelFileOperations.h"

namespace {

// Fournit un dossier temporaire vierge par test (créé/supprimé automatiquement).
class LevelFileOps : public ::testing::Test {
protected:
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("pg_levelops_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
    }
};

}  // namespace

TEST_F(LevelFileOps, CreeUnNiveauValide) {
    const hmi::LevelFileOperations ops(dir);
    const hmi::FileOpResult result = ops.create("MonNiveau", 10, 6);
    ASSERT_TRUE(result.ok) << result.error;
    EXPECT_TRUE(std::filesystem::exists(result.path));
    EXPECT_EQ(ops.list().size(), 1U);
}

TEST_F(LevelFileOps, RefuseNomInvalideEtCollision) {
    const hmi::LevelFileOperations ops(dir);
    EXPECT_FALSE(ops.create("a/b", 10, 6).ok);   // barre oblique interdite
    EXPECT_TRUE(ops.create("Niveau", 10, 6).ok);
    EXPECT_FALSE(ops.create("Niveau", 10, 6).ok);  // collision de nom
}

TEST_F(LevelFileOps, RenommeEtDeplaceLeFichier) {
    const hmi::LevelFileOperations ops(dir);
    const hmi::FileOpResult created = ops.create("Ancien", 10, 6);
    ASSERT_TRUE(created.ok) << created.error;
    const hmi::FileOpResult renamed = ops.rename(created.path, "Nouveau");
    ASSERT_TRUE(renamed.ok) << renamed.error;
    EXPECT_FALSE(std::filesystem::exists(created.path));
    EXPECT_TRUE(std::filesystem::exists(renamed.path));
}

TEST_F(LevelFileOps, DupliqueSousUnNomUnique) {
    const hmi::LevelFileOperations ops(dir);
    const hmi::FileOpResult base = ops.create("Base", 10, 6);
    ASSERT_TRUE(base.ok) << base.error;
    const hmi::FileOpResult first = ops.duplicate(base.path);
    const hmi::FileOpResult second = ops.duplicate(base.path);
    ASSERT_TRUE(first.ok) << first.error;
    ASSERT_TRUE(second.ok) << second.error;
    EXPECT_NE(first.path, second.path);  // deux copies distinctes
    EXPECT_EQ(ops.list().size(), 3U);
}

TEST_F(LevelFileOps, SupprimeLeFichier) {
    const hmi::LevelFileOperations ops(dir);
    const hmi::FileOpResult created = ops.create("X", 10, 6);
    ASSERT_TRUE(created.ok) << created.error;
    EXPECT_TRUE(ops.remove(created.path).ok);
    EXPECT_FALSE(std::filesystem::exists(created.path));
}
