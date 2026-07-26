/**
 * @file main_qt.cpp
 * @brief Point d'entrée de l'éditeur Qt (LOT-34, refonte IHM).
 *
 * À cette tâche (TACHE-01), l'application se limite à ouvrir une **fenêtre nue** : elle valide le
 * socle de build Qt (find_package, AUTOMOC, lien avec `HmiRuntime`) avant d'embarquer le viewport
 * Direct3D 11 (TACHE-02), la boucle et les entrées (TACHE-03).
 */

#include <QApplication>
#include <QMainWindow>
#include <QString>

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    QApplication application(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("ProjectGaming — Éditeur (Qt)"));
    window.resize(1280, 720);
    window.show();

    return application.exec();
}
