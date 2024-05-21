#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

// Fonction pour vérifier les permissions de suppression
bool hasDeletePermission(const std::string& filePath) {
    struct stat fileStat;

    // Obtenir les informations du fichier
    if (stat(filePath.c_str(), &fileStat) != 0) {
        // Erreur lors de l'obtention des informations
        return false;
    }

    // Vérifier si c'est un fichier régulier
    if (!S_ISREG(fileStat.st_mode)) {
        return false;
    }

    // Vérifier les permissions d'écriture
    if (!(fileStat.st_mode & S_IWUSR)) {
        return false;
    }

    return true;
}

// Fonction pour traiter la requête DELETE
void handleDeleteRequest(const HttpRequest& request, HttpResponse& response) {
    std::string path = "/var/www" + request.uri;

    // Vérifier l'existence de la ressource
    if (!std::filesystem::exists(path)) {
        response.setStatusCode(404); // Not Found
        response.setStatusMessage("Resource Not Found");
        response.setBody("");
        response.send();
        return;
    }

    // Vérifier si la ressource est un dossier
    if (std::filesystem::is_directory(path)) {
        response.setStatusCode(403); // Forbidden
        response.setStatusMessage("Forbidden: Resource is a Directory");
        response.setBody("");
        response.send();
        return;
    }

    // Vérifier les permissions de suppression
    if (!hasDeletePermission(path)) {
        response.setStatusCode(403); // Forbidden
        response.setStatusMessage("Forbidden: Insufficient Permissions");
        response.setBody("");
        response.send();
        return;
    }

    // Supprimer le fichier
    if (std::filesystem::remove(path)) {
        response.setStatusCode(204); // No Content
        response.setStatusMessage("No Content");
        response.setBody("");
    } else {
        response.setStatusCode(500); // Internal Server Error
        response.setStatusMessage("Internal Server Error: Could Not Delete Resource");
        response.setBody("");
    }

    response.send();
}

