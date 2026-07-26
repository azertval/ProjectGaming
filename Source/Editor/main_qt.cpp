/**
 * @file main_qt.cpp
 * @brief Point d'entrée de l'éditeur Qt (LOT-34, refonte IHM).
 *
 * L'application ouvre la fenêtre principale (`editor::MainWindow`) dont le widget central est le
 * viewport Direct3D 11 (`editor::GameViewport`, TACHE-02). La boucle de jeu et les entrées arrivent
 * à la TACHE-03, le rendu d'un niveau à la TACHE-04.
 */

#include <QApplication>
#include <QCoreApplication>

#include "Editor/MainWindow.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    QApplication application(argc, argv);
    // Identité de l'application : sert de portée aux réglages persistés (QSettings — disposition
    // des panneaux de l'éditeur, EX-IHM-011).
    QCoreApplication::setOrganizationName(QStringLiteral("ProjectGaming"));
    QCoreApplication::setApplicationName(QStringLiteral("Editor"));

    editor::MainWindow window;
    window.show();

    return application.exec();
}
