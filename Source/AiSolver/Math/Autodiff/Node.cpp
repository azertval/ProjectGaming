// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Math/Autodiff/Node.h"

#include <cstddef>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AiSolver/Math/TensorOps.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::autodiff {

Node::Node(Tensor<float> value) : value(std::move(value)), grad(this->value.shape()) {}

// Remet grad a zero (meme forme que value) : necessaire entre deux passes d'entrainement, avant
// qu'un nouveau backward() n'accumule sur ce noeud.
void Node::zeroGrad() {
    grad = Tensor<float>(value.shape());
}

// Feuille du graphe : aucun parent, _backwardFn vide (rien a propager plus loin -- un parametre
// ou une entree n'a pas de regle de derivation).
NodePtr variable(Tensor<float> value) {
    return std::make_shared<Node>(std::move(value));
}

NodePtr unaryOp(
    const NodePtr& input, std::function<Tensor<float>(const Tensor<float>&)> forward,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const Tensor<float>&)>
        localGrad) {
    // Passe avant eager : forward() est appele tout de suite, value est donc disponible des la
    // construction du noeud, sans attendre un backward() ulterieur.
    auto result = std::make_shared<Node>(forward(input->value));
    result->_parents = {input};

    // La fermeture capture le resultat par pointeur brut, jamais par shared_ptr : le resultat
    // possede deja _backwardFn (qui capturerait donc lui-meme le resultat) -- une capture par
    // shared_ptr creerait un cycle de comptage de references (le noeud ne serait jamais libere).
    // Le pointeur brut reste valide : _backwardFn n'est appelee que par backward() (parcours du
    // graphe depuis la racine), qui maintient le noeud en vie via _parents pendant tout l'appel.
    Node* output = result.get();
    result->_backwardFn = [output, input, localGrad]() {
        // Accumulation (+=), jamais ecrasement : un parent reutilise plusieurs fois dans le
        // graphe doit recevoir la somme des contributions de chacun de ses usages.
        input->grad = add(input->grad, localGrad(output->value, output->grad, input->value));
    };
    return result;
}

NodePtr binaryOp(const NodePtr& a, const NodePtr& b,
                 std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&)> forward,
                 std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&,
                                             const Tensor<float>&, const Tensor<float>&)>
                     localGradA,
                 std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&,
                                             const Tensor<float>&, const Tensor<float>&)>
                     localGradB) {
    auto result = std::make_shared<Node>(forward(a->value, b->value));
    result->_parents = {a, b};

    // Meme raison qu'unaryOp : pointeur brut sur le resultat pour eviter un cycle de shared_ptr.
    Node* output = result.get();
    result->_backwardFn = [output, a, b, localGradA, localGradB]() {
        a->grad = add(a->grad, localGradA(output->value, output->grad, a->value, b->value));
        b->grad = add(b->grad, localGradB(output->value, output->grad, a->value, b->value));
    };
    return result;
}

void backward(const NodePtr& root) {
    PROJECTGAMING_ASSERT(root->value.size() == 1,
                         "backward() : la racine doit etre un tenseur scalaire (un seul element)");

    // Derivee d'une quantite par rapport a elle-meme.
    root->grad = Tensor<float>(root->value.shape());
    root->grad.data()[0] = 1.0f;

    // Tri topologique par parcours en profondeur post-fixe : un noeud n'est pousse dans `order`
    // qu'apres tous ses parents, donc `order` va des feuilles vers `root`. La deduplication par
    // adresse brute (Node*) est sans risque de cycle : un graphe construit uniquement par
    // unaryOp/binaryOp ne peut referencer qu'un noeud deja existant.
    //
    // Parcours ITERATIF, avec une pile explicite : la profondeur du graphe vaut le nombre de pas
    // d'un episode (la perte de policy gradient est une chaine d'`add`, un maillon par pas). Une
    // recursion y consommerait une trame par pas, et un episode de plusieurs milliers de pas
    // deborderait la pile de 1 Mio d'un fil Windows -- c'est ce qui bornait la longueur des
    // episodes bien avant la memoire.
    //
    // Chaque entree de pile porte le noeud et l'indice du prochain parent a explorer : le noeud
    // n'est pousse dans `order` qu'a sa seconde visite, quand tous ses parents le sont deja
    // (post-fixe). La pile est indexee, jamais referencee : `emplace_back` peut la reallouer.
    std::vector<Node*> order;
    std::unordered_set<Node*> visited;
    std::vector<std::pair<Node*, std::size_t>> pending;
    visited.insert(root.get());
    pending.emplace_back(root.get(), 0);
    while (!pending.empty()) {
        const std::size_t top = pending.size() - 1;
        Node* node = pending[top].first;
        if (pending[top].second < node->_parents.size()) {
            Node* parent = node->_parents[pending[top].second].get();
            ++pending[top].second;
            if (visited.insert(parent).second) {
                pending.emplace_back(parent, 0);
            }
            continue;
        }
        order.push_back(node);
        pending.pop_back();
    }

    // Propagation de root vers les feuilles (ordre inverse de `order`) : quand _backwardFn() est
    // appelee sur un noeud, son propre grad est deja completement accumule (tous les noeuds qui
    // l'utilisent comme parent ont ete traites avant lui dans ce parcours).
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        if ((*it)->_backwardFn) {
            (*it)->_backwardFn();
        }
    }
}

}  // namespace aisolver::autodiff
