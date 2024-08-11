#include "HttpResponse.hpp"

bool HttpResponse::checkFilePermission(const std::string& filePath) 
{
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0)
        return false;

    if (!S_ISREG(fileStat.st_mode) && !S_ISDIR(fileStat.st_mode))
        return false;

    if ((fileStat.st_mode & S_IWUSR) == 0)  // Check if write permission is missing
        return false;

    return true;
}

bool HttpResponse::deleteDirectory(const std::string& dirPath)
{
    DIR* dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        return false;
    }

    bool success = true;
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string entryPath = dirPath + "/" + entry->d_name;
        struct stat entryStat;
        
        if (stat(entryPath.c_str(), &entryStat) == 0) 
        {
            // Check permissions before attempting deletion
            if (!checkFilePermission(entryPath)) 
            {
                success = false;  // Fail if any file or directory lacks permissions
                continue;  // Skip to the next entry
            }

            if (S_ISDIR(entryStat.st_mode)) 
            {
                if (!deleteDirectory(entryPath))
                {
                    success = false;
                }
            } 
            else 
            {
                if (unlink(entryPath.c_str()) != 0) 
                {
                    success = false;
                }
            }
        }
    }

    closedir(dir);
    // Attempt to remove the directory itself only if all internal operations succeeded
    if (success && rmdir(dirPath.c_str()) != 0) 
    {
        success = false;
    }

    return success;
}

void HttpResponse::handleDeleteMethod() 
{
    std::string root = _serv.getRoot();
    std::string filePath = _filePath;
    
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) 
    {
        buildResponse(404);
        return;
    }
    if (!checkFilePermission(filePath))
    {
        buildResponse(403);
        return;
    }

     if (S_ISDIR(fileStat.st_mode))
    {
        if (deleteDirectory(filePath))
        {
            _errCode = 204;
            buildResponse(204);
        }
        else
            buildResponse(403);
    }
    else
    {
        if (unlink(filePath.c_str()) == 0)
        {
            _errCode = 204;
            buildResponse(204);
        }
        else
            buildResponse(403);
    }
}
