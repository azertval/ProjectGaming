#include "AiSolver/Math/Autodiff/Node.h"

#include "AiSolver/Math/TensorOps.h"

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

}  // namespace aisolver::autodiff
