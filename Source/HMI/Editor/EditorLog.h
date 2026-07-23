#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file HMI/Editor/EditorLog.h
 * @brief Macros de journalisation de la catégorie « Editor ».
 *
 * Sous-catégorie de log de l'éditeur de niveaux intégré (voir `HMI/HmiLog.h` pour le modèle).
 * Réservée aux actions ponctuelles (redimensionnement, liaison de mécanisme, annuler/refaire,
 * sélection de niveau), jamais aux opérations exécutées à chaque frame (peinture au survol,
 * glisser en cours).
 */

#define EDITOR_LOG_TRACE(message) PROJECTGAMING_LOG_TRACE("Editor", message)
#define EDITOR_LOG_INFO(message) PROJECTGAMING_LOG_INFO("Editor", message)
#define EDITOR_LOG_WARNING(message) PROJECTGAMING_LOG_WARNING("Editor", message)
#define EDITOR_LOG_ERROR(message) PROJECTGAMING_LOG_ERROR("Editor", message)
