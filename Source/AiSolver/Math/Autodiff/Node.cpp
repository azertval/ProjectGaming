#include "AiSolver/Math/Autodiff/Node.h"

#include <unordered_set>
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

NodePtr binaryOp(
    const NodePtr& a, const NodePtr& b,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&)> forward,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const Tensor<float>&,
                                 const Tensor<float>&)>
        localGradA,
    std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const Tensor<float>&,
                                 const Tensor<float>&)>
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
    std::vector<Node*> order;
    std::unordered_set<Node*> visited;
    const std::function<void(const NodePtr&)> visit = [&](const NodePtr& node) {
        if (!visited.insert(node.get()).second) {
            return;
        }
        for (const NodePtr& parent : node->_parents) {
            visit(parent);
        }
        order.push_back(node.get());
    };
    visit(root);

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
