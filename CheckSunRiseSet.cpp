// CheckSunRiseSet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "centralHeader.h"
#include "MsConversion.h"
#include "SunRiseLocData.h"
#include "SunRiseSetData.h"
#include "CheckSunRiseSetFunc.h"
#include "CalledPhytonPrograms.h"

int main(int argc, char* argv[], char* envp[])
{
    struct _timeb timebStr;                     // Structure to hold the current time
    struct tm localTimeStr;                     // Structure to hold the local time
    char actDateStr[1024];                      // String to store the current date in YYYY-MM-DD format
    int yyyy, mm, dd;                           // Variables to store year, month, and day
    int argx = 1;                               // Index for command-line arguments
    std::string dateArg;                        // String to store the date argument
    std::string tempFile("");                   // String to store the temporary file name
    int ret = 0;                                // Return code
    std::deque<std::string> argList;            // Vector to store command-line arguments

    setConsoleUtf8();                           // COnsole output in UTF-8, to be able to print the degree symbol

    double usedLatitude = 49.59749459760013;    // Variable to store user-provided latitude
    double usedLongitude = 11.030420778017055;  // Variable to store user-provided longitude
    std::string usedLocationName = "Erlangen LHS7";       // Variable to store the name of the location being used

    std::vector<SunRiseLocData> allSunRiseLoc;  // Vector to store sunrise and sunset data for all examined days

    allSunRiseLoc.push_back(SunRiseLocData(13.4050, 52.5200, "Berlin", "BER"));
    allSunRiseLoc.push_back(SunRiseLocData(16.004253309523712, 47.582752519429604, "Sankt Corona am Wechsel", "SCW"));
    allSunRiseLoc.push_back(SunRiseLocData(11.030420778017055, 49.59749459760013, "Erlangen LHS7", "LHS7", true));
    allSunRiseLoc.push_back(SunRiseLocData(11.02, 49.6, "Erlangen", "ERL"));
    allSunRiseLoc.push_back(SunRiseLocData(24.447981540937395, 60.99245734329251, "Hammeenlinna", "HAM"));
    allSunRiseLoc.push_back(SunRiseLocData(123.02513400392203, 41.14699766775208, "Anshan China", "ANSH"));
    allSunRiseLoc.push_back(SunRiseLocData(7.1739421, 51.47378,  "Bochum", "BO"));
    allSunRiseLoc.push_back(SunRiseLocData(-8.546278183796636, 42.87701552131139, "Santiago de Compostela", "SNTC"));
    allSunRiseLoc.push_back(SunRiseLocData(2.3522219, 48.856614, "Paris", "PAR"));
    allSunRiseLoc.push_back(SunRiseLocData(15.077890747349263, 37.51680658399921, "Catania", "CAT"));
    allSunRiseLoc.push_back(SunRiseLocData(22.565807164306058, 51.24074659813717, "Lublin", "LUB"));
    allSunRiseLoc.push_back(SunRiseLocData(19.040235, 47.497912, "Budapest", "BUD"));
    allSunRiseLoc.push_back(SunRiseLocData(10.752245, 59.913868, "Oslo", "OSL"));
    allSunRiseLoc.push_back(SunRiseLocData(12.466538356911842, 41.94913784098775, "Rome", "ROM"));
    allSunRiseLoc.push_back(SunRiseLocData(14.248124609834235, 40.83564085378236, "Naples", "NAP"));
    allSunRiseLoc.push_back(SunRiseLocData(-122.44101309663327, 37.76833367819105, "San Francisco", "SFO")); 
    allSunRiseLoc.push_back(SunRiseLocData(-157.85759797082366, 21.309809310750566, "Honolulu", "HNL"));
    allSunRiseLoc.push_back(SunRiseLocData(151.2081315508734, -33.85866717153931, "Sydney", "SYD"));
    allSunRiseLoc.push_back(SunRiseLocData(121.46295693023379, 31.205872515867057, "Shanghai", "SHA"));
    allSunRiseLoc.push_back(SunRiseLocData(174.7663930033069, -36.84552359972905, "Auckland", "AKL"));
    allSunRiseLoc.push_back(SunRiseLocData(-70.9028751273797, -53.15898500469935, "Punta Arenas de Chile", "PACH"));
    allSunRiseLoc.push_back(SunRiseLocData(18.406399051225634, -33.910038268565835, "Cape Town", "CAPT"));
    allSunRiseLoc.push_back(SunRiseLocData(9.436705326084184, 0.40764796753931765, "LibreVille Gabun", "LIBV"));
    allSunRiseLoc.push_back(SunRiseLocData(39.81697067583217, 21.43080874388954, "Mekka", "MEKK"));

    // Sort available locations by longitude (ascending) so lists and lookups are ordered geographically
    std::sort(allSunRiseLoc.begin(), allSunRiseLoc.end(),
      [](const SunRiseLocData& a, const SunRiseLocData& b) {
        return a.getLongitude() > b.getLongitude();
      });

    const SunRiseLocData* locDefData = getDefaultLocation(allSunRiseLoc);
    if (locDefData != nullptr)
    {
      usedLocationName = locDefData->getLocationName(); 
      usedLatitude = locDefData->getLatitude();
      usedLongitude = locDefData->getLongitude();
    }

    try
    {
        // Create a temporary file
        tempFile = createTempFile();
        // std::cout << "Temporary file created at: " << tempFile << std::endl;

        // Write the Python script to the temporary file
        std::ofstream ofs(tempFile);
        if (!ofs.is_open())
        {
            throw std::runtime_error("Failed to open temporary file for writing.");
        }

        ofs << GetSunRiseSetPyProgram << std::endl;
        ofs.close();

        // Get the current time
        errno_t xError = _ftime_s(&timebStr);
        if (xError != 0)
        {
            char errTxt[128];
            strerror_s(errTxt, sizeof(errTxt), xError);
            std::cerr << "Error getting current time: " << errTxt << std::endl;
            throw std::runtime_error("");
        }

        // Convert the current time to local time
        xError = localtime_s(&localTimeStr, &timebStr.time);
        if (xError != 0)
        {
            char errTxt[128];
            strerror_s(errTxt, sizeof(errTxt), xError);
            std::cerr << "Error getting local time: " << errTxt << std::endl;
            throw std::runtime_error("");
        }

        // Build a date string in the format YYYY-MM-DD from the local time
        sprintf_s(actDateStr, sizeof(actDateStr), "%04d-%02d-%02d", localTimeStr.tm_year + 1900, localTimeStr.tm_mon + 1, localTimeStr.tm_mday);

        // Determine the date argument to use
        if (argc == 1)
        {
            argList.push_back(actDateStr);
        }
        else
        {
          while (argc > 1)
          {
            argList.push_back(argv[argx++]);
            argc--;
          }
        }

        // Process each date argument
        while (! argList.empty())
        {
            dateArg = argList.front();
            // Check if the Argument is an known Location in Short
            const SunRiseLocData* locDataPtr = isKnownLocation(dateArg, allSunRiseLoc);
            if (locDataPtr != nullptr)
            {
                //std::cout << "Found known location for " << locDataPtr->getLocationName()  << std::endl;
                // danach entfernen
                argList.pop_front();
                if (argList.empty())
                {
                  argList.push_back(actDateStr);
                }
                usedLatitude = locDataPtr->getLatitude();
                usedLongitude = locDataPtr->getLongitude(); 
                usedLocationName = locDataPtr->getLocationName();
                continue;
            }
            // Check if the Argument is reside yyyy-mm-dd and is a valid date
            if (isValidDate(dateArg, yyyy, mm, dd))
            {

                SunRiseSetDatas sunRiseSetDatas;                // Here we store all the sunrise and sunset data for all the days what we got from our python programm
                unsigned int Exam_yyyy = 0;                     // This is the Year to become examimed
                unsigned int Exam_mm = 0;                       // dito month
                unsigned int Exam_dd = 0;						            // dito day 
				        unsigned int ActYear = 0;                       // This is the Year of the actual day to be treated
				        unsigned int ActMonth = 0;  			              // dito month
				        unsigned int ActDay = 0;    		                // dito day
                char sunrise_marker = ' ';                      // This is the marker for the sunrise time, it is ' ' if the sun rises on this day and 'p' if previous day sunrise time is used, because the sun does not rise on this day
                char sunset_marker = ' ';                       // This is the marker for the sunset time, it is ' ' if the sun sets on this day and 'n' if next day sunset time is used, because the sun does not set on this day
				        unsigned long long sunrise_unix_ms = 0;         // This is the Sunrise in ms after 1970-01-01 00:00:00
				        unsigned long long sunset_unix_ms = 0;          // This is the Sunset in ms after 1970-01-01 00:00:00
				        unsigned long long day_length_ms = 0;           // This is the length of the day in ms
				        size_t theEvalIndex = 0;                        // This is the index of the day to be examined in the vector of sunRiseSetDatas
				        int noArg = 0;								    // This is the number of arguments read by sscanf_s, to check if the line is correct
                char textBuffer[MAX_PATH + 128];

				        // Call the Python script to get the sunrise and sunset times
                sprintf_s(textBuffer, sizeof(textBuffer), "py %s %04d-%02d-%02d  %.12f %.12f", tempFile.c_str(), yyyy, mm, dd, usedLatitude, usedLongitude);
				        std::istringstream sres(cmdPipeExec(textBuffer));   // The output of the Python Program is a long String, which we have to split into lines
				        std::string aSingleLine;							// Here we store the single line of the output  

				        // Treat all the Lines we find in the output of the Python Programm
                for (size_t lineCnt = 0; (std::getline(sres, aSingleLine)); lineCnt++)
                {
					          // The first line contains the Day to be examined
                    if (lineCnt == 0)
                    {
						            noArg = sscanf_s(aSingleLine.c_str(), "%d-%d-%d", &Exam_yyyy, &Exam_mm, &Exam_dd);
                        if (noArg != 3)
                        {
							            std::cerr << "Python Program written back wrong Examinated Date:" << aSingleLine << std::endl;
							            throw std::runtime_error("");
                        }
                        continue;
                    }

					          // All other Lines should contain the Data for the Day: The Date itself, the Sunrise, the Sunset and the Sunlight length
					          noArg = sscanf_s(
                        aSingleLine.c_str(),
                        "%d-%d-%d, %llu%c, %llu%c, %llu",
                        &ActYear,
                        &ActMonth,
                        &ActDay,
                        &sunrise_unix_ms,
                        &sunrise_marker, static_cast<unsigned int>(sizeof(sunrise_marker)),
                        &sunset_unix_ms,
                        &sunset_marker, static_cast<unsigned int>(sizeof(sunset_marker)),
                        &day_length_ms
                    );
                    if (noArg != 8)
                    {
                        std::cerr << "Python Programm written back wrong response in line " << lineCnt << ": " << aSingleLine << std::endl;
                        throw std::runtime_error("");
                    }

					          // Add the tupel to the vector of sunRiseSetDatas
					          sunRiseSetDatas.addDayData(Exam_yyyy, Exam_mm, Exam_dd, ActYear, ActMonth, ActDay, sunrise_unix_ms, sunrise_marker, sunset_unix_ms, sunset_marker, day_length_ms);

                    // Process each line here
                    // std::cout << "Processing line: " << aSingleLine << std::endl;

                    // Add your analysis logic here
                }
                if (sunRiseSetDatas.size() > 0)
                {
                    sunRiseSetDatas.doAnalyzation();
                    sunRiseSetDatas.CalcDelta2Reference();
                    sprintf_s(textBuffer, sizeof(textBuffer), "At location % s(% 7.3lf%c, % 7.3lf%c)", usedLocationName.c_str(), fabs(usedLatitude), (usedLatitude >= 0) ? 'N' : 'S', fabs(usedLongitude), (usedLongitude >= 0) ? 'E' : 'W');
                    std::cout << textBuffer << std::endl << std::endl;
                    std::cout << sunRiseSetDatas.printResult().str();

					//std::cout << "Found Index " << theEvalIndex << " for " << Exam_yyyy << "-" << Exam_mm << "-" << Exam_dd << std::endl;
					//std::cout << "Sunrise and Sunset times for " << Exam_yyyy << "-" << Exam_mm << "-" << Exam_dd << ":" << std::endl;
					//std::cout << "Sunrise: " << sunRiseSetDatas[theEvalIndex].getSunriseUnixMs() << " ms" << std::endl;
					//std::cout << "Sunset: " << sunRiseSetDatas[theEvalIndex].getSunsetUnixMs() << " ms" << std::endl;
					//std::cout << "Day length: " << sunRiseSetDatas[theEvalIndex].getDayLengthMs() << " ms" << std::endl;
				}
                else
                {
                    std::cerr << "No data available for " << Exam_yyyy << "-" << Exam_mm << "-" << Exam_dd << std::endl;
                }
                //std::cout << "Result from Python script: " << result << std::endl;
            }
            else
            {
              char fmtstr[64];
              char buf[512];
          
              // Provide a clear usage message and examples, then list known locations sorted by longitude.
              std::cerr << "Invalid input: \"" << dateArg << "\"" << std::endl;
              std::cerr << std::endl;
              std::cerr << "Usage:" << std::endl;
              std::cerr << "  " << argv[0] << " [LOCATION|DATE]..." << std::endl;
              std::cerr << std::endl;
              std::cerr << "Rules:" << std::endl;
              std::cerr << "  - LOCATION   : known short name (e.g. LHS7) from the list below" << std::endl;
              std::cerr << "  - DATE       : calendar date in format YYYY-MM-DD (YYYY.MM.DD also accepted)" << std::endl;
              std::cerr << "  - Arguments can be repeated in any order." << std::endl;
              std::cerr << "  - If only LOCATION is given, DATE defaults to today." << std::endl;
              std::cerr << "  - If only DATE is given, LOCATION defaults to the configured default." << std::endl;
              std::cerr << std::endl;
              std::cerr << "Examples:" << std::endl;
              std::cerr << "  " << argv[0] << " LHS7 2024-02-20     # location then date" << std::endl;
              std::cerr << "  " << argv[0] << " 2024-02-20 LHS7     # date then location" << std::endl;
              std::cerr << "  " << argv[0] << " LHS7               # date = today" << std::endl;
              std::cerr << "  " << argv[0] << " 2024-02-20         # location = default" << std::endl;
              std::cerr << std::endl;

              std::cerr << "Known locations (sorted by longitude):" << std::endl;

              size_t longLoc = strlen("Location");
              size_t longShortLoc = strlen("Short");
              for (const auto& loc : allSunRiseLoc)
              {
                if (longLoc < strlen(loc.getLocationName().c_str()))
                {
                  longLoc = strlen(loc.getLocationName().c_str());
                }
                if (longShortLoc < strlen(loc.getShortName().c_str()))
                {
                  longShortLoc = strlen(loc.getShortName().c_str());
                }
              }
              sprintf_s(fmtstr, sizeof(fmtstr), "    Longitude    Latitude %%-%zds %%-%zds", longLoc, longShortLoc);
              sprintf_s(buf, sizeof(buf), fmtstr, "Location", "Short");
              std::cerr << buf << std::endl;

              sprintf_s(fmtstr, sizeof(fmtstr), "  %%10.6f%%c %%10.6f%%c %%-%zds %%-%zds %%s", longLoc, longShortLoc);

              for (const auto& loc : allSunRiseLoc)
              {
                // Print as:  <longitude> <latitude>, <Place>, <Short> [<-Default]
                sprintf_s(buf, sizeof(buf), fmtstr,
                  fabs(loc.getLongitude()),
                  (loc.getLongitude() < 0.0 ? 'W' : 'E'),
                  fabs(loc.getLatitude()),
                  (loc.getLatitude() < 0.0 ? 'S' : 'N'),
                  loc.getLocationName().c_str(),
                  loc.getShortName().c_str(),
                  loc.isDefault() ? " <-Default" : "");
                std::cerr << buf << std::endl;
              }
              break;
            }
            argList.pop_front();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        ret = 1;
    }

    // Delete the temporary file
    if (tempFile != "")
    {
        if (std::remove(tempFile.c_str()) != 0)
        {
            std::cerr << "Failed to delete temporary file: " << tempFile << std::endl;
        }
        else
        {
            // std::cout << "Temporary file deleted." << std::endl;
        }
    }

    return ret;
}
