#pragma once

// Standard library includes
#include <iostream>     // For standard I/O operations
#include <memory>       // For smart pointers
#include <stdexcept>    // For standard exceptions
#include <string>       // For string operations
#include <array>        // For array container
#include <regex>        // For regular expressions
#include <ctime>        // For time-related functions
#include <stdio.h>      // For standard I/O functions
#include <stdlib.h>     // For standard library functions
#include <chrono>       // For time utilities
#include <sys/timeb.h>  // For ftime and timeb structure
#include <time.h>       // For time-related functions
#include <errno.h>      // For error number definitions
#include <fstream>      // For file stream operations
#include <tchar.h>      // For Unicode and ASCII text mapping
#include <sstream>      // For string stream operations
#include <cstdio>       // For C-style I/O functions

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>    // For Windows-specific functions
#else
#include <unistd.h>     // For POSIX-specific functions
#endif

