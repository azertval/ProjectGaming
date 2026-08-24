/**
 * @file test_tensor.cpp
 * @brief Tests unitaires du conteneur tensoriel `aisolver::Tensor<T>` (LOT-ANNEXE-01, TACHE-02).
 */

#include <stdexcept>

#include <gtest/gtest.h>

#include "AiSolver/Math/Tensor.h"
#include "Core/Diagnostics/Assert.h"

/**
 * @brief Un tenseur nouvellement construit a la forme demandée, la bonne taille et des éléments
 * initialisés à zéro.
 * \castest{<b>Tensor : construction et forme.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `Tensor<float>({2, 3})`.<br/>2. Lire `shape()`, `size()` et chaque
 * élément.<br/>
 * \tattendu `shape() == {2, 3}`, `size() == 6`, tous les éléments valent `0.0f`.}
 */
TEST(TensorTest, ConstructionEtForme) {
    aisolver::Tensor<float> tensor({2, 3});

    EXPECT_EQ(tensor.rank(), 2u);
    EXPECT_EQ(tensor.shape(), (std::vector<std::size_t>{2, 3}));
    EXPECT_EQ(tensor.size(), 6u);

    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            EXPECT_FLOAT_EQ(tensor.at({i, j}), 0.0f);
        }
    }
}

/**
 * @brief Écrire à une position puis la relire renvoie la valeur écrite ; les autres positions
 * restent inchangées.
 * \castest{<b>Tensor : indexation en lecture/écriture.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un tenseur `2x3`.<br/>2. Écrire une valeur à `at({1, 2})`.<br/>3. Relire
 * cette position et une autre.<br/>
 * \tattendu La position écrite renvoie la valeur posée, les autres restent à `0.0f`.}
 */
TEST(TensorTest, Indexation) {
    aisolver::Tensor<float> tensor({2, 3});

    tensor.at({1, 2}) = 42.0f;

    EXPECT_FLOAT_EQ(tensor.at({1, 2}), 42.0f);
    EXPECT_FLOAT_EQ(tensor.at({0, 0}), 0.0f);
    EXPECT_FLOAT_EQ(tensor.at({1, 0}), 0.0f);
}

/**
 * @brief La convention row-major place `at({1, 0})` à l'offset linéaire `3` pour une forme
 * `{2, 3}` (dernière dimension contiguë), pas `1`.
 * \castest{<b>Tensor : row-major vérifié explicitement.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un tenseur `2x3`.<br/>2. Écrire une valeur connue via `data()[3]`.<br/>
 * 3. Relire via `at({1, 0})`.<br/>
 * \tattendu `at({1, 0})` lit exactement l'offset `3` du tampon linéaire.}
 */
TEST(TensorTest, RowMajorExplicite) {
    aisolver::Tensor<float> tensor({2, 3});

    EXPECT_EQ(tensor.strides(), (std::vector<std::size_t>{3, 1}));

    tensor.data()[3] = 7.0f;
    EXPECT_FLOAT_EQ(tensor.at({1, 0}), 7.0f);
}

/**
 * @brief Une vue (`view()`) partage le même tampon que l'original : écrire dans la vue modifie
 * l'original à la position correspondante.
 * \castest{<b>Tensor : une vue partage le tampon avec l'original.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `a` de forme `{2, 3}`.<br/>2. `b = a.view({6})`.<br/>3. Écrire dans `b`
 * via `at({k})`.<br/>
 * \tattendu La valeur lue dans `a` à la position correspondante change aussi.}
 */
TEST(TensorTest, VuePartageLeTampon) {
    aisolver::Tensor<float> a({2, 3});
    aisolver::Tensor<float> b = a.view({6});

    EXPECT_EQ(b.shape(), (std::vector<std::size_t>{6}));

    b.at({4}) = 9.0f;

    EXPECT_FLOAT_EQ(a.at({1, 1}), 9.0f);
}

/**
 * @brief `clone()` produit une copie profonde : écrire dans la copie ne modifie pas l'original.
 * \castest{<b>Tensor : `clone()` ne partage pas le tampon.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `a` de forme `{2, 3}`.<br/>2. `c = a.clone()`.<br/>3. Écrire dans
 * `c`.<br/>
 * \tattendu `a` reste inchangé à la position correspondante.}
 */
TEST(TensorTest, CloneNePartagePas) {
    aisolver::Tensor<float> a({2, 3});
    aisolver::Tensor<float> c = a.clone();

    c.at({0, 0}) = 5.0f;

    EXPECT_FLOAT_EQ(a.at({0, 0}), 0.0f);
    EXPECT_FLOAT_EQ(c.at({0, 0}), 5.0f);
}

/**
 * @brief `at()` avec un mauvais nombre d'indices ou un indice hors bornes déclenche
 * `PROJECTGAMING_ASSERT`.
 * \castest{<b>Tensor : assertion sur indexation invalide.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un tenseur `2x3`.<br/>2. Appeler `at()` avec un nombre d'indices
 * incorrect, puis avec un indice hors bornes.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué dans les deux cas.}
 */
TEST(TensorTest, AssertionIndexationInvalide) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    aisolver::Tensor<float> tensor({2, 3});

    // Le gestionnaire lève pour interrompre avant tout accès hors-bornes au tampon.
    core::setAssertionHandler([](const char*, const char*, const char*, int) {
        throw std::runtime_error("precondition");
    });

    EXPECT_THROW({ [[maybe_unused]] float& v = tensor.at({0}); }, std::runtime_error);
    EXPECT_THROW({ [[maybe_unused]] float& v = tensor.at({0, 5}); }, std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief `view()` avec un nombre d'éléments incompatible avec la forme d'origine déclenche
 * `PROJECTGAMING_ASSERT`.
 * \castest{<b>Tensor : assertion sur vue de volume incompatible.</b><br/>
 * \tcat Unitaire · Tenseurs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un tenseur `2x3` (6 éléments).<br/>2. Appeler `view({4})` (4 éléments,
 * volume different).<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué.}
 */
TEST(TensorTest, AssertionVueVolumeIncompatible) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    aisolver::Tensor<float> tensor({2, 3});

    core::setAssertionHandler([](const char*, const char*, const char*, int) {
        throw std::runtime_error("precondition");
    });

    EXPECT_THROW(
        { [[maybe_unused]] aisolver::Tensor<float> view = tensor.view({4}); }, std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}
