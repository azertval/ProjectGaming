#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file HMI/Platform/PlatformLog.h
 * @brief Macros de journalisation de la catégorie « Platform ».
 *
 * Sous-catégorie de log de la plateforme (fenêtre Win32 ; voir `HMI/HmiLog.h` pour le modèle).
 * À réserver aux événements rares (création de la fenêtre, demande de fermeture) ; jamais aux
 * messages reçus en continu (redimensionnement en cours de glissement, mouvements souris…).
 */

#define PLATFORM_LOG_TRACE(message) PROJECTGAMING_LOG_TRACE("Platform", message)
#define PLATFORM_LOG_INFO(message) PROJECTGAMING_LOG_INFO("Platform", message)
#define PLATFORM_LOG_WARNING(message) PROJECTGAMING_LOG_WARNING("Platform", message)
#define PLATFORM_LOG_ERROR(message) PROJECTGAMING_LOG_ERROR("Platform", message)
