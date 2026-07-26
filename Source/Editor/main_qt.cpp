/**
 * @file main_qt.cpp
 * @brief Point d'entrée de l'éditeur Qt (LOT-34, refonte IHM).
 *
 * L'application ouvre la fenêtre principale (`editor::MainWindow`) dont le widget central est le
 * viewport Direct3D 11 (`editor::GameViewport`, TACHE-02). La boucle de jeu et les entrées arrivent
 * à la TACHE-03, le rendu d'un niveau à la TACHE-04.
 */

#include <QApplication>

#include "Editor/MainWindow.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    QApplication application(argc, argv);

    editor::MainWindow window;
    window.show();

    return application.exec();
}
