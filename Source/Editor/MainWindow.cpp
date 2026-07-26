#include "Editor/MainWindow.h"

#include <QString>
#include <QWidget>

#include "Editor/GameViewport.h"

namespace editor {

MainWindow::MainWindow() : _viewport(new GameViewport()) {
    // `createWindowContainer` embarque la fenêtre native du viewport dans la hiérarchie de widgets
    // et en prend la propriété (détruite avec la fenêtre principale).
    QWidget* const container = QWidget::createWindowContainer(_viewport, this);
    container->setMinimumSize(320, 240);
    container->setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(container);

    setWindowTitle(QStringLiteral("ProjectGaming — Éditeur (Qt)"));
    resize(1280, 720);
}

MainWindow::~MainWindow() = default;

}  // namespace editor
