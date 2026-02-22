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
    std::deque<std::string> argList;           // Vector to store command-line arguments

    double usedLatitude = 49.59749459760013;    // Variable to store user-provided latitude
    double usedLongitude = 11.030420778017055;  // Variable to store user-provided longitude
    std::string usedLocationName = "Erlangen LHS7";       // Variable to store the name of the location being used

    std::vector<SunRiseLocData> allSunRiseLoations;  // Vector to store sunrise and sunset data for all examined days

    allSunRiseLoations.push_back(SunRiseLocData(13.4050, 52.5200, "Berlin", "BER"));
    allSunRiseLoations.push_back(SunRiseLocData(16.004253309523712, 47.582752519429604, "Sankt Corona am Wechsel", "SCW"));
    allSunRiseLoations.push_back(SunRiseLocData(11.030420778017055, 49.59749459760013, "Erlangen LHS7", "LHS7", true));
    allSunRiseLoations.push_back(SunRiseLocData(11.02, 49.6, "Erlangen", "ERL"));
    allSunRiseLoations.push_back(SunRiseLocData(24.447981540937395, 60.99245734329251, "Hämmeenlinna", "HAM"));
    allSunRiseLoations.push_back(SunRiseLocData(123.02513400392203, 41.14699766775208, "Anshan China", "ANSH"));
    allSunRiseLoations.push_back(SunRiseLocData(7.1739421, 51.47378,  "Bochum", "BO"));
    
    const SunRiseLocData* locDefData = getDefaultLocation(allSunRiseLoations);
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
            const SunRiseLocData* locDataPtr = isKnownLocation(dateArg, allSunRiseLoations);
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
                unsigned int Exam_dd = 0;						// dito day 
				        unsigned int ActYear = 0;                       // This is the Year of the actual day to be treated
				        unsigned int ActMonth = 0;  			        // dito month
				        unsigned int ActDay = 0;    		            // dito day
				        unsigned long long sunrise_unix_ms = 0;         // This is the Sunrise in ms after 1970-01-01 00:00:00
				        unsigned long long sunset_unix_ms = 0;          // This is the Sunset in ms after 1970-01-01 00:00:00
				        unsigned long long day_length_ms = 0;           // This is the length of the day in ms
				        size_t theEvalIndex = 0;                        // This is the index of the day to be examined in the vector of sunRiseSetDatas
				        int noArg = 0;								    // This is the number of arguments read by sscanf_s, to check if the line is correct
                char textBuffer[MAX_PATH + 128];

				        // Call the Python script to get the sunrise and sunset times
                sprintf_s(textBuffer, sizeof(textBuffer), "py %s %d-%d-%d  %.12f %.12f", tempFile.c_str(), yyyy, mm, dd, usedLatitude, usedLongitude);
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
					          noArg = sscanf_s(aSingleLine.c_str(), "%d-%d-%d, %llu, %llu, %llu", &ActYear, &ActMonth, &ActDay, &sunrise_unix_ms, &sunset_unix_ms, &day_length_ms);
                    if (noArg != 6)
                    {
                        std::cerr << "Python Programm written back wrong response in line " << lineCnt << ": " << aSingleLine << std::endl;
                        throw std::runtime_error("");
                    }

					          // Add the tupel to the vector of sunRiseSetDatas
					          sunRiseSetDatas.addDayData(Exam_yyyy, Exam_mm, Exam_dd, ActYear, ActMonth, ActDay, sunrise_unix_ms, sunset_unix_ms, day_length_ms);

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
                std::cerr << "Invalid date format (" << dateArg << "). Please use YYYY-MM-DD or YYYY.MM.DD, where YYYY is a valid Year, MM a valid Month and DD a valid Day" << std::endl;
                throw std::runtime_error("");
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
