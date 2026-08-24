/**
 * @file test_autodiff_backward.cpp
 * @brief Tests unitaires de `aisolver::autodiff::backward()` (LOT-ANNEXE-02, TACHE-03) : parcours
 * topologique inverse, accumulation sur nœud partagé, assertion sur racine non scalaire.
 */

#include <stdexcept>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Tensor.h"
#include "Core/Diagnostics/Assert.h"

using aisolver::Tensor;
using aisolver::autodiff::backward;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;

namespace {
constexpr float TOLERANCE = 1e-5f;

Tensor<float> scalar(float value) {
    Tensor<float> result({1});
    result.at({0}) = value;
    return result;
}
}  // namespace

/**
 * @brief `y = a + b` : après `backward(y)`, `a->grad` et `b->grad` valent chacun `1` (dérivée de
 * la somme par rapport à chaque terme).
 * \castest{<b>Backward : `y = a + b`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles scalaires.<br/>2. Calculer `y = add(a, b)`.<br/>
 * 3. Appeler `backward(y)`.<br/>
 * \tattendu `a->grad` et `b->grad` valent `1.0`.}
 */
TEST(BackwardTest, SommeDeDeuxFeuilles) {
    const NodePtr a = variable(scalar(2.0f));
    const NodePtr b = variable(scalar(3.0f));

    const NodePtr y = aisolver::autodiff::add(a, b);
    backward(y);

    EXPECT_NEAR(a->grad.at({0}), 1.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), 1.0f, TOLERANCE);
}

/**
 * @brief `y = a * b` : après `backward(y)`, `a->grad == b->value` et `b->grad == a->value` (règle
 * du produit), sur des tenseurs à un seul élément.
 * \castest{<b>Backward : `y = a * b`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles scalaires `3` et `5`.<br/>2. Calculer
 * `y = multiply(a, b)`.<br/>3. Appeler `backward(y)`.<br/>
 * \tattendu `a->grad == 5` (valeur de `b`), `b->grad == 3` (valeur de `a`).}
 */
TEST(BackwardTest, ProduitDeDeuxFeuilles) {
    const NodePtr a = variable(scalar(3.0f));
    const NodePtr b = variable(scalar(5.0f));

    const NodePtr y = aisolver::autodiff::multiply(a, b);
    backward(y);

    EXPECT_NEAR(a->grad.at({0}), b->value.at({0}), TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), a->value.at({0}), TOLERANCE);
    EXPECT_NEAR(a->grad.at({0}), 5.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), 3.0f, TOLERANCE);
}

/**
 * @brief `y = (a + b) + a` : le nœud `a` est réutilisé deux fois dans le graphe ; après
 * `backward(y)`, `a->grad` vaut `2` (somme des deux contributions), pas `1`.
 * \castest{<b>Backward : nœud réutilisé accumule ses contributions.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire deux feuilles scalaires.<br/>2. Calculer `ab = add(a, b)` puis
 * `y = add(ab, a)`.<br/>3. Appeler `backward(y)`.<br/>
 * \tattendu `a->grad` vaut `2.0` (une contribution via `ab`, une contribution directe) ;
 * `b->grad` vaut `1.0`.}
 */
TEST(BackwardTest, NoeudReutiliseAccumule) {
    const NodePtr a = variable(scalar(2.0f));
    const NodePtr b = variable(scalar(3.0f));

    const NodePtr ab = aisolver::autodiff::add(a, b);
    const NodePtr y = aisolver::autodiff::add(ab, a);
    backward(y);

    EXPECT_NEAR(y->value.at({0}), 7.0f, TOLERANCE);
    EXPECT_NEAR(a->grad.at({0}), 2.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), 1.0f, TOLERANCE);
}

/**
 * @brief `y = relu(matmul(w, x) + b)` (motif d'une couche dense, formes choisies pour que la
 * sortie soit scalaire) : `backward(y)` produit des gradients corrects sur `w`, `x`, `b`, vérifiés
 * contre un calcul de référence posé à la main.
 * \castest{<b>Backward : chaîne `matmul` + `add` + `relu`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `w = [[2, -1]]` (1x2), `x = [[3], [1]]` (2x1), `b = [[-1]]` (1x1), entrées
 * choisies pour que la somme reste strictement positive (hors du point non dérivable de
 * `relu`).<br/>2. Calculer `y = relu(add(matmul(w, x), b))`.<br/>3. Appeler `backward(y)`.<br/>
 * \tattendu `y->value == 4` ; `w->grad == [3, 1]` (= `x` transposé) ; `x->grad == [[2], [-1]]`
 * (= `w` transposé) ; `b->grad == [1]` (calcul détaillé en commentaire).}
 */
TEST(BackwardTest, ChaineMatmulAddRelu) {
    Tensor<float> wData({1, 2});
    wData.at({0, 0}) = 2.0f;
    wData.at({0, 1}) = -1.0f;

    Tensor<float> xData({2, 1});
    xData.at({0, 0}) = 3.0f;
    xData.at({1, 0}) = 1.0f;

    Tensor<float> bData({1, 1});
    bData.at({0, 0}) = -1.0f;

    const NodePtr w = variable(wData);
    const NodePtr x = variable(xData);
    const NodePtr b = variable(bData);

    // sum = w.x + b = (2*3 + (-1)*1) - 1 = 5 - 1 = 4, strictement positif -> relu(sum) = sum.
    const NodePtr sum = aisolver::autodiff::add(aisolver::autodiff::matmul(w, x), b);
    const NodePtr y = aisolver::autodiff::relu(sum);
    backward(y);

    EXPECT_EQ(y->value.shape(), (std::vector<std::size_t>{1, 1}));
    EXPECT_NEAR(y->value.at({0, 0}), 4.0f, TOLERANCE);

    // d(sum)/dw = x^T = [3, 1] ; d(sum)/dx = w^T = [[2], [-1]] ; d(sum)/db = 1 ; relu'(4) = 1.
    EXPECT_NEAR(w->grad.at({0, 0}), 3.0f, TOLERANCE);
    EXPECT_NEAR(w->grad.at({0, 1}), 1.0f, TOLERANCE);
    EXPECT_NEAR(x->grad.at({0, 0}), 2.0f, TOLERANCE);
    EXPECT_NEAR(x->grad.at({1, 0}), -1.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0, 0}), 1.0f, TOLERANCE);
}

/**
 * @brief `backward()` appelé sur un nœud racine non scalaire (`value.size() > 1`) déclenche
 * `PROJECTGAMING_ASSERT`.
 * \castest{<b>Backward : assertion sur racine non scalaire.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille de forme `{2}`.<br/>2. Appeler `backward()` dessus.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué avant tout calcul.}
 */
TEST(BackwardTest, AssertionRacineNonScalaire) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    const NodePtr root = variable(Tensor<float>({2}));

    core::setAssertionHandler([](const char*, const char*, const char*, int) {
        throw std::runtime_error("precondition");
    });

    EXPECT_THROW(backward(root), std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief Appeler `backward()` une seconde fois sur le même graphe, sans `zeroGrad()` entre les
 * deux, accumule par-dessus les gradients précédents (comportement attendu et documenté).
 * \castest{<b>Backward : deux appels successifs sans `zeroGrad()` accumulent.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire `y = add(a, b)`.<br/>2. Appeler `backward(y)` deux fois de suite, sans
 * `zeroGrad()` entre les deux.<br/>
 * \tattendu `a->grad` et `b->grad` valent `2.0` après le second appel (chaque appel accumule `1`).}
 */
TEST(BackwardTest, DeuxAppelsSansZeroGradAccumulent) {
    const NodePtr a = variable(scalar(2.0f));
    const NodePtr b = variable(scalar(3.0f));
    const NodePtr y = aisolver::autodiff::add(a, b);

    backward(y);
    backward(y);

    EXPECT_NEAR(a->grad.at({0}), 2.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), 2.0f, TOLERANCE);
}
