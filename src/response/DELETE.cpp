#include "HttpResponse.hpp"

bool HttpResponse::checkFilePermission(const std::string& filePath) 
{
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0)
        return false;

    if (!S_ISREG(fileStat.st_mode) && !S_ISDIR(fileStat.st_mode))
        return false;

    if ((fileStat.st_mode & S_IWUSR) == 0)
        return false;

    return true;
}

bool deleteDirectory(const std::string& dirPath) 
{
    DIR* dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string entryPath = dirPath + "/" + entry->d_name;
        struct stat entryStat;
        if (stat(entryPath.c_str(), &entryStat) == 0) 
        {
            if (S_ISDIR(entryStat.st_mode)) 
            {
                if (!deleteDirectory(entryPath))
                {
                    closedir(dir);
                    return false;
                }
            } 
            else 
            {
                if (unlink(entryPath.c_str()) != 0) 
                {
                    closedir(dir);
                    return false;
                }
            }
        }
    }

    closedir(dir);
    return rmdir(dirPath.c_str()) == 0;
}

void HttpResponse::handleDeleteMethod() 
{
    std::string root = _serv.getRoot();
    std::string filePath = _filePath;
    
    struct stat fileStat;
    // if (filePath == root)
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
