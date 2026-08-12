#include "HMI/Audio/SoundTriggers.h"

namespace hmi {

std::optional<std::string> soundForEvent(GameEvent event) {
    // switch exhaustif sans default : un GameEvent ajoute sans entree casse la compilation.
    switch (event) {
        case GameEvent::Jumped:
            return std::string{"saut"};
        case GameEvent::Landed:
            return std::string{"atterrissage"};
        case GameEvent::Dashed:
            return std::string{"dash"};
        case GameEvent::WallContactEnter:
            return std::nullopt;  // aucun bruitage dedie dans ce lot.
        case GameEvent::SwitchToggled:
            return std::string{"interrupteur"};
        case GameEvent::DoorOpened:
            return std::string{"porte"};
        case GameEvent::DoorClosed:
            return std::string{"porte"};
        case GameEvent::PressurePlatePressed:
            return std::string{"plaque_pression"};
        case GameEvent::PressurePlateReleased:
            return std::string{"plaque_pression"};
        case GameEvent::BlockPushed:
            return std::nullopt;  // aucun bruitage dedie dans ce lot.
        case GameEvent::Died:
            return std::string{"mort"};
        case GameEvent::LevelCompleted:
            return std::string{"victoire_tableau"};
        case GameEvent::MenuNavigate:
            return std::string{"menu_deplacement"};
        case GameEvent::MenuConfirm:
            return std::string{"menu_validation"};
        case GameEvent::MenuBack:
            return std::string{"menu_deplacement"};
        case GameEvent::PauseOpened:
            return std::nullopt;  // pas dans le perimetre sonore de l'epic (Inclus).
        case GameEvent::SequenceCompleted:
            return std::string{"fin_sequence"};
    }
    return std::nullopt;  // inatteignable : le switch couvre tout l'enum.
}

}  // namespace hmi
