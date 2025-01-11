

#include "centralHeader.h"
#include "MsConversion.h"
#include "SunRiseSetData.h"
#include "CheckSunRiseSetFunc.h"

std::string createTempFile()
{
    std::string tempFileName;

#ifdef _WIN32
    // Windows implementation using GetTempPathA and GetTempFileNameA (ANSI versions)
    char tempPath[MAX_PATH];
    char tempFile[MAX_PATH];

    // Get the temporary directory path
    if (!GetTempPathA(MAX_PATH, tempPath))
    {
        throw std::runtime_error("Failed to get temporary directory.");
    }

    // Generate a unique temporary file name
    if (!GetTempFileNameA(tempPath, "TMP", 0, tempFile))
    {
        throw std::runtime_error("Failed to create temporary file.");
    }

    tempFileName = tempFile;

#else
    // Unix/Linux implementation using mkstemp
    char tempFile[] = "/tmp/tempfileXXXXXX"; // Template for mkstemp

    // Create and open the temporary file
    int fd = mkstemp(tempFile);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to create temporary file.");
    }

    // Close the file descriptor, but keep the file
    close(fd);

    tempFileName = tempFile;
#endif

    return tempFileName;
}

std::string cmdPipeExec(const char* cmd)
{
    std::array<char, 1024> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

// Check if the input String is a valid date in the format YYYY-MM-DD or YYYY.MM.DD, and return the date values in Form of Year month and day in ok case
bool isValidDate
(
    const std::string& date,
    int& yyyy,
    int& mm,
    int& dd
)
{
    struct tm tmStr;                            // Result Time Structure

    int noarg = sscanf_s(date.c_str(), "%d-%d-%d", &yyyy, &mm, &dd);
    if (noarg != 3)
    {
        noarg = sscanf_s(date.c_str(), "%d.%d.%d", &yyyy, &mm, &dd);
        if (noarg != 3)
        {
            return false;
        }
    }

    // Fill in Time Structure
    memset(&tmStr, 0, sizeof(tmStr));
    tmStr.tm_year = yyyy - 1900;
    tmStr.tm_mon = mm - 1;
    tmStr.tm_mday = dd;
    tmStr.tm_hour = 12;
    tmStr.tm_min = 0;
    tmStr.tm_sec = 0;
    tmStr.tm_isdst = -1;

    // Validate the date using mktime
    if (mktime(&tmStr) == -1)
    {
        return false;
    }

    yyyy = tmStr.tm_year + 1900;
    mm = tmStr.tm_mon + 1;
    dd = tmStr.tm_mday;


    return true;
}
