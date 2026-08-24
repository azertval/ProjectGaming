#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Math/Autodiff/Node.h
 * @brief Graphe de calcul dynamique de l'autodiff maison (aisolver::autodiff::Node).
 */

namespace aisolver::autodiff {

class Node;

/// Un `Node` n'est jamais manipulé que par pointeur partagé (jamais copié, jamais sur la pile).
using NodePtr = std::shared_ptr<Node>;

/**
 * @brief Nœud d'un graphe de calcul dynamique (mode *reverse*, construit à l'exécution).
 *
 * Porte le résultat d'une passe avant (`value`), le gradient accumulé de même forme (`grad`), et
 * la connaissance de comment régénérer ce gradient localement à partir de ses parents
 * (`_backwardFn`, appelée uniquement par `backward()`). Non copiable : un `Node` n'existe qu'au
 * travers d'un `NodePtr` partagé par les nœuds qui le référencent comme parent.
 */
class Node {
public:
    /**
     * @brief Construit un nœud portant `value`, gradient initialisé à zéro de même forme.
     * @param value Résultat de la passe avant.
     */
    explicit Node(Tensor<float> value);

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    /// Résultat de la passe avant.
    Tensor<float> value;

    /// Gradient accumulé, même forme que `value` ; remis à zéro par `zeroGrad()`.
    Tensor<float> grad;

    /// @brief Remet `grad` à zéro (même forme que `value`).
    void zeroGrad();

private:
    friend NodePtr variable(Tensor<float> value);
    friend void backward(const NodePtr& root);
    friend NodePtr unaryOp(const NodePtr& input,
                           std::function<Tensor<float>(const Tensor<float>&)> forward,
                           std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&,
                                                       const Tensor<float>&)>
                               localGrad);
    friend NodePtr binaryOp(
        const NodePtr& a, const NodePtr& b,
        std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&)> forward,
        std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&,
                                    const Tensor<float>&, const Tensor<float>&)>
            localGradA,
        std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&,
                                    const Tensor<float>&, const Tensor<float>&)>
            localGradB);

    /// Nœuds parents dans le graphe ; vide pour une feuille produite par `variable()`.
    std::vector<NodePtr> _parents;

    /// Règle de dérivation locale, capturée en fermeture sur les parents ; vide pour une feuille.
    /// Appelée uniquement par `backward()` (parcours topologique inverse), jamais par l'appelant.
    std::function<void()> _backwardFn;
};

/**
 * @brief Construit une feuille du graphe (paramètre ou entrée), sans parent.
 * @param value Valeur portée par la feuille.
 */
[[nodiscard]] NodePtr variable(Tensor<float> value);

/**
 * @brief Fabrique générique d'opération différentiable à un seul parent.
 *
 * Calcule `value` immédiatement (passe avant *eager*, `forward` appelé tout de suite) et
 * enregistre la règle de dérivation locale comme fermeture sur `input`, appelée uniquement par
 * `backward()`. Permet d'ajouter une nouvelle opération différentiable (ex. `relu`, `tanh`) sans
 * modifier `Node` ni le moteur de rétropropagation.
 *
 * @param input     Nœud parent.
 * @param forward   Calcule la valeur de sortie à partir de la valeur de `input`.
 * @param localGrad Calcule la contribution à accumuler sur `input->grad`, à partir de la valeur de
 *                  sortie, du gradient de sortie et de la valeur de `input` (dans cet ordre).
 */
[[nodiscard]] NodePtr unaryOp(
    const NodePtr& input, std::function<Tensor<float>(const Tensor<float>&)> forward,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const Tensor<float>&)>
        localGrad);

/**
 * @brief Fabrique générique d'opération différentiable à deux parents.
 *
 * Même principe que `unaryOp`, avec une contribution indépendante accumulée sur chacun des deux
 * parents.
 *
 * @param a          Premier nœud parent.
 * @param b          Second nœud parent.
 * @param forward    Calcule la valeur de sortie à partir des valeurs de `a` et `b`.
 * @param localGradA Contribution à accumuler sur `a->grad`, à partir de la valeur de sortie, du
 *                   gradient de sortie, de la valeur de `a` et de la valeur de `b` (dans cet
 * ordre).
 * @param localGradB Contribution à accumuler sur `b->grad`, mêmes arguments que `localGradA`.
 */
[[nodiscard]] NodePtr binaryOp(
    const NodePtr& a, const NodePtr& b,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&)> forward,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const Tensor<float>&,
                                const Tensor<float>&)> localGradA,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const Tensor<float>&,
                                const Tensor<float>&)> localGradB);

/**
 * @brief Rétropropagation : déclenche toutes les règles de dérivation locales du graphe atteignable
 * depuis `root`, de la racine vers les feuilles.
 *
 * Initialise `root->grad` à `1` (dérivée d'une quantité par rapport à elle-même), construit un tri
 * topologique du graphe par parcours en profondeur post-fixe (chaque nœud après ses parents), puis
 * appelle `_backwardFn()` sur chaque nœud de ce tri en ordre inverse (de `root` vers les feuilles)
 * : chaque appel accumule (`+=`) sa contribution dans le(s) `grad` du(des) parent(s) direct(s). Les
 * feuilles produites par `variable()` n'ont pas de `_backwardFn` et sont ignorées.
 *
 * N'appelle jamais `zeroGrad()` : appeler `backward()` deux fois de suite sur le même graphe sans
 * `zeroGrad()` entre les deux accumule par-dessus les gradients précédents, à la charge de
 * l'appelant (typiquement l'optimiseur, `LOT-ANNEXE-04`).
 *
 * @param root Nœud racine, dont dérive la rétropropagation ; doit être **scalaire**
 *             (`root->value.size() == 1`, `PROJECTGAMING_ASSERT` sinon).
 */
void backward(const NodePtr& root);

}  // namespace aisolver::autodiff
