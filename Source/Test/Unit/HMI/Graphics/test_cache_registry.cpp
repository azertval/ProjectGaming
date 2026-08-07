/**
 * @file test_cache_registry.cpp
 * @brief Tests unitaires du registre de mémoïsation/invalidation (LOT-43 TACHE-03).
 */

#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "HMI/Graphics/CacheRegistry.h"

namespace {

// Compte les appels au chargeur, pour verifier la memoisation sans dependre du type charge.
class CountingLoader {
public:
    explicit CountingLoader(std::optional<int> result) : _result(result) {}

    std::optional<int> operator()() {
        ++_callCount;
        return _result;
    }

    [[nodiscard]] int callCount() const noexcept {
        return _callCount;
    }

private:
    std::optional<int> _result;
    int _callCount = 0;
};

}  // namespace

TEST(CacheRegistryTest, ChargeUneSeuleFoisPourUneCleRepetee) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loader(42);

    const int* first = registry.getOrLoad("a", [&loader] { return loader(); });
    const int* second = registry.getOrLoad("a", [&loader] { return loader(); });

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(*first, 42);
    EXPECT_EQ(second, first) << "meme entree, pas de rechargement";
    EXPECT_EQ(loader.callCount(), 1);
}

TEST(CacheRegistryTest, InvalidateForceLeRechargementDeLaCleSeule) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loaderA(1);
    CountingLoader loaderB(2);

    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));
    registry.invalidate("a");

    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));

    EXPECT_EQ(loaderA.callCount(), 2) << "la cle invalidee doit relire";
    EXPECT_EQ(loaderB.callCount(), 1) << "les autres entrees sont conservees";
}

TEST(CacheRegistryTest, InvalidateAllViseTout) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loaderA(1);
    CountingLoader loaderB(2);
    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));
    ASSERT_EQ(registry.size(), 2U);

    registry.invalidateAll();

    EXPECT_EQ(registry.size(), 0U);
    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));
    EXPECT_EQ(loaderA.callCount(), 2);
    EXPECT_EQ(loaderB.callCount(), 2);
}

TEST(CacheRegistryTest, UnEchecEstMemoriseSansRetenterLeChargement) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loader(std::nullopt);

    const int* first = registry.getOrLoad("absent", [&loader] { return loader(); });
    const int* second = registry.getOrLoad("absent", [&loader] { return loader(); });

    EXPECT_EQ(first, nullptr);
    EXPECT_EQ(second, nullptr);
    EXPECT_EQ(loader.callCount(), 1) << "un echec deja constate ne doit pas relire le disque";
}

TEST(CacheRegistryTest, InvalidateSurUnEchecPermetUnNouvelEssai) {
    hmi::CacheRegistry<int> registry;
    CountingLoader failing(std::nullopt);
    static_cast<void>(registry.getOrLoad("asset", [&failing] { return failing(); }));
    registry.invalidate("asset");

    CountingLoader succeeding(7);
    const int* result = registry.getOrLoad("asset", [&succeeding] { return succeeding(); });

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 7);
}
