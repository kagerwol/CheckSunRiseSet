#pragma once

// Nomen est Omen, create a name for a temporary file
std::string createTempFile();

// Execute a command and return the result as a string
std::string cmdPipeExec(const char* cmd);

// Check if the input String is a valid date in the format YYYY-MM-DD or YYYY.MM.DD, and return the date values in Form of Year month and day in ok case
bool isValidDate
(
    const std::string& date,
    int& yyyy,
    int& mm,
    int& dd
);