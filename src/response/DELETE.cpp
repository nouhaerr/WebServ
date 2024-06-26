#include "HttpResponse.hpp"

bool HttpResponse::checkFilePermission(const std::string& filePath) {
    struct stat fileStat;
    // Obtenir les informations sur le fichier
    if (stat(filePath.c_str(), &fileStat) != 0) {
        return false;
    }

    // Vérifier si le fichier est régulier
    if (!S_ISREG(fileStat.st_mode)) {
        return false;
    }

    // Vérifier les permissions d'écriture
    if ((fileStat.st_mode & S_IWUSR) == 0) {
        return false;
    }

    return true;
}

void HttpResponse::handleDeleteMethod() 
{
    std::string root = _serv.getRoot();
    std::string filePath = _filePath;
    
    // std::cout << "Root directory: " << root << std::endl; // Log added
    // std::cout << "Trying to delete file at: " << filePath << std::endl; // Log added

    // Vérifier l'existence de la ressource
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        buildResponse(404); // Not Found
        return;
    }

    // Gestion des dossiers
    if (S_ISDIR(fileStat.st_mode)) {
        buildResponse(403); // Forbidden
        return;
    }

    // Gestion des fichiers
    if (checkFilePermission(filePath)) {
        if (unlink(filePath.c_str()) == 0) {
            buildResponse(204); // No Content
        } else {
            buildResponse(403); // Forbidden
        }
    } else {
        buildResponse(403); // Forbidden
    }
}
