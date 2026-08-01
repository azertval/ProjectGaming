#include "AiSolver/Math/Rng.h"

namespace aisolver {

Rng::Rng(std::uint64_t seed) : _engine(seed) {}

// flottant uniforme dans [0, 1), obtenu à la main à partir de la sortie brute de _engine() :
// conserver les 24 bits de poids fort (_engine() >> 40) et diviser par 2²⁴ (16777216.0f),
// soit exactement le nombre de valeurs représentables dans la mantisse d'un float — mapping exact,
// sans arrondi surprenant, et strictement < 1.0f.
float Rng::nextFloat() {
    return static_cast<float>(_engine() >> 40) / 16777216.0f;
}

// Flottant uniforme dans [min, max), obtenu par mise à l'échelle et translation de nextFloat().
float Rng::nextFloat(float min, float max) {
    return min + nextFloat() * (max - min);
}

// loi normale par la transformation de Box-Muller, calculée à la main sur deux appels à nextFloat() :
// z = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * kPi * u2), puis mean + stddev * z.
float Rng::nextGaussian(float mean, float stddev) {    
    float kPi = 3.14159265358979323846f;

    float u1 = 0.0f;
    do { // u1 est retiré tant qu'il vaut exactement 0.0f (le logarithme de zéro n'est pas défini)
        u1 = nextFloat();
    } while (u1 == 0.0f);

    float u2 = nextFloat(); 

    float z = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * kPi * u2);

    return mean + stddev * z;
}

// entier uniforme dans [min, max] (bornes incluses), par rejet sur la sortie brute de _engine() :
// soit range = max - min + 1, on tire _engine() jusqu'à obtenir une valeur strictement
// inférieure au plus grand multiple de range représentable sur 64 bits,
// puis on renvoie min + valeur % range — le rejet élimine le biais (léger mais réel)
// du modulo simple sur les dernières valeurs de la plage. 
int Rng::nextInt(int min, int max) {
    int range = max - min + 1;
    std::uint64_t limit = std::numeric_limits<std::uint64_t>::max() - std::numeric_limits<std::uint64_t>::max() % range;
    std::uint64_t value;
    do {
        value = _engine();
    } while (value >= limit);

    return min + static_cast<int>(value % range);
}

}  // namespace aisolver
