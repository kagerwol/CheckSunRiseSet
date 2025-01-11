#include "centralHeader.h"
#include "MsConversion.h"
#include "SunRiseSetData.h"

SunRiseSetData::SunRiseSetData(
    unsigned int _yyyy,
    unsigned int _mm,
    unsigned int _dd,
    unsigned long long _sunrise_unix_ms,
    unsigned long long _sunset_unix_ms,
    unsigned long long _day_length_ms
)
    : m_Year(_yyyy)
    , m_Month(_mm)
    , m_Day(_dd)
    , m_Sunrise(_sunrise_unix_ms, MsConversion::TIMESTAMP)
    , m_Sunset(_sunset_unix_ms, MsConversion::TIMESTAMP)
    , m_Daylength(_day_length_ms, MsConversion::DURATION)
{
    return;     // to sender
}

// Copy constructor
SunRiseSetData::SunRiseSetData(const SunRiseSetData& other)
    : m_Year(other.m_Year)
    , m_Month(other.m_Month)
    , m_Day(other.m_Day)
    , m_Sunrise(other.m_Sunrise)
    , m_Sunset(other.m_Sunset)
    , m_Daylength(other.m_Daylength)
{
    return;     // to sender
}

// Assignment operator
SunRiseSetData& SunRiseSetData::operator=(const SunRiseSetData& other)
{
    if (this != &other)
    {
        m_Year = other.m_Year;
        m_Month = other.m_Month;
        m_Day = other.m_Day;
        m_Sunrise = other.m_Sunrise;
        m_Sunset = other.m_Sunset;
        m_Daylength = other.m_Daylength;
    }
    return *this;
}

SunRiseSetData::~SunRiseSetData()
{
    return;     // to sender
}


SunRiseSetDatas::SunRiseSetDatas()
{
    m_TheEvalIndex.init(); 

    for (SunRiseSetData::TimeCriteria criteria = SunRiseSetData::RISE; (true); )
    {
        m_PrevTurn[criteria].init();
        m_NextTurn[criteria].init();
        m_PrevSim[criteria].init();
        m_NextSim[criteria].init();
        if (criteria == SunRiseSetData::RISE)
        {
            criteria = SunRiseSetData::SET;
        }
        else if (criteria == SunRiseSetData::SET)
        {
            criteria = SunRiseSetData::DUR;
        }
        else
        {
            break;
        }
    }

	m_SunRiseSetDatas.clear();
}

SunRiseSetDatas::~SunRiseSetDatas()
{
	m_SunRiseSetDatas.clear();
}

size_t SunRiseSetDatas::addDayData(
    unsigned int ExamYear,                      // This is the Year to become examimed
    unsigned int ExamMonth,                     // dito month
    unsigned int ExamDay,						// dito day 
    unsigned int ActYear,                       // This is the Year of the actual day to be treated
    unsigned int ActMonth,  			        // dito month
    unsigned int ActDay,    		            // dito day
    unsigned long long sunrise_unix_ms,         // This is the Sunrise in ms after 1970-01-01 00:00:00
    unsigned long long sunset_unix_ms,          // This is the Sunset in ms after 1970-01-01 00:00:00
    unsigned long long day_length_ms            // This is the length of the day in ms
)
{
    // Fill one tupel of the vector of sunRiseSetDatas
    SunRiseSetData dayData(ActYear, ActMonth, ActDay, sunrise_unix_ms, sunset_unix_ms, day_length_ms);
    // Add the tupel to the vector of sunRiseSetDatas
    m_SunRiseSetDatas.push_back(dayData);
    // If the Date of the Day to be examined is found in the vector of sunRiseSetDatas, we have to remember the index
    if ((ActYear == ExamYear) && (ActMonth == ExamMonth) && (ActDay == ExamDay) && (!m_TheEvalIndex.valid))
    {
        m_TheEvalIndex.index = m_SunRiseSetDatas.size() - 1;
        m_TheEvalIndex.valid = true;
    }
    return m_SunRiseSetDatas.size();
}

int SunRiseSetDatas::CheckForExtrema(const size_t anEvalIndex)
{
    if ((anEvalIndex > m_SunRiseSetDatas.size() - 2) || (anEvalIndex < 2))
    {
        m_SunRiseSetDatas[anEvalIndex].setExtreme(SunRiseSetData::RISE, 0);
        m_SunRiseSetDatas[anEvalIndex].setExtreme(SunRiseSetData::SET, 0);
        m_SunRiseSetDatas[anEvalIndex].setExtreme(SunRiseSetData::DUR, 0);
        return 0;
    }

    int retVal = 0;
    const SunRiseSetData& prevDay = m_SunRiseSetDatas[anEvalIndex - 1];
    SunRiseSetData& actDay = m_SunRiseSetDatas[anEvalIndex];
    const SunRiseSetData& nextDay = m_SunRiseSetDatas[anEvalIndex + 1];

    for (SunRiseSetData::TimeCriteria criteria = SunRiseSetData::RISE; (true); )
    {
        if (prevDay.whatCriteria(criteria).getMsAfterMidnight() < actDay.whatCriteria(criteria).getMsAfterMidnight() && nextDay.whatCriteria(criteria).getMsAfterMidnight() < actDay.whatCriteria(criteria).getMsAfterMidnight())
        {
            // Latest Sun Rise
            actDay.setExtreme(criteria, 1);
            retVal |= 0x0001;
        }
        else if (prevDay.whatCriteria(criteria).getMsAfterMidnight() > actDay.whatCriteria(criteria).getMsAfterMidnight() && nextDay.whatCriteria(criteria).getMsAfterMidnight() > actDay.whatCriteria(criteria).getMsAfterMidnight())
        {
            // Earliest Sun Rise
            actDay.setExtreme(criteria, -1);
            retVal |= 0x0002;
        }
        else
        {
            // not Extreme Sun Rise
            actDay.setExtreme(criteria, 0);
        }
        if (criteria == SunRiseSetData::RISE)
        {
            retVal <<= 2;
            criteria = SunRiseSetData::SET;
        }
        else if (criteria == SunRiseSetData::SET)
        {
            retVal <<= 2;
            criteria = SunRiseSetData::DUR;
        }
        else
        {
            break;
        }
    }


    return retVal;
}

int SunRiseSetDatas::doAnalyzation()                         // Find all Previous and Next Turning Points
{
    int retVal = 0;

    if (m_TheEvalIndex.valid)
    {
        bool first = true;
        int aRise = 0;
        int aSet = 0;
        int aDur = 0;
        size_t anEvalInx = 0;
        long long deltaMs = 0;
        bool deltaMsSign = false;
        long long minDeltaMs = 0;
        bool deltaMinDeltaMsSign = false;

        for (anEvalInx = 1; anEvalInx < m_SunRiseSetDatas.size() - 1; anEvalInx++)
        {
            CheckForExtrema(anEvalInx);
        }

        for (SunRiseSetData::TimeCriteria criteria = SunRiseSetData::RISE; (true); )
        {
            for (anEvalInx = m_TheEvalIndex.index; anEvalInx > 1; )
            {
                anEvalInx--;
                if (
                    (m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(criteria).getExtreme() == 0) && 
                    (m_SunRiseSetDatas[anEvalInx].whatCriteria(criteria).getExtreme()  != 0)           && 
                    (!m_PrevTurn[criteria].valid)
                    )
                {
                    m_PrevTurn[criteria].index = anEvalInx;
                    m_PrevTurn[criteria].valid = true;
                    break;
                }
            }

            if ((m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(criteria).getExtreme() == 0) && (m_PrevTurn[criteria].valid))
            {
                for (anEvalInx = m_PrevTurn[criteria].index, first = true; anEvalInx > 1; )
                {
                    anEvalInx--;
                    deltaMs = m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(criteria).getMsAfterMidnight();
                    deltaMs -= m_SunRiseSetDatas[anEvalInx].whatCriteria(criteria).getMsAfterMidnight();
                    if (first)
                    {
                        minDeltaMs = deltaMs;
                        first = false;
                        continue;
                    }
                    deltaMsSign = (deltaMs < 0);
                    deltaMinDeltaMsSign = (minDeltaMs < 0);
                    if (deltaMsSign != deltaMinDeltaMsSign)
                    {
                        if (llabs(minDeltaMs) < llabs(deltaMs))
                        {
                            m_PrevSim[criteria].index = anEvalInx + 1;
                            m_PrevSim[criteria].valid = true;
                            break;
                        }
                        else
                        {
                            m_PrevSim[criteria].index = anEvalInx;
                            m_PrevSim[criteria].valid = true;
                            break;
                        }
                    }
                    minDeltaMs = deltaMs;
                }
            }

            for (anEvalInx = m_TheEvalIndex.index; anEvalInx < m_SunRiseSetDatas.size() - 1; )
            {
                anEvalInx++;
                if (
                    (m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(criteria).getExtreme() == 0) &&
                    (m_SunRiseSetDatas[anEvalInx].whatCriteria(criteria).getExtreme() != 0) &&
                    (!m_NextTurn[criteria].valid)
                    )
                {
                    m_NextTurn[criteria].index = anEvalInx;
                    m_NextTurn[criteria].valid = true;
                    break;
                }
            }

            if ((m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(criteria).getExtreme() == 0) && (m_NextTurn[criteria].valid))
            {
                for (anEvalInx = m_NextTurn[criteria].index, first = true; anEvalInx < m_SunRiseSetDatas.size() - 1; )
                {
                    anEvalInx++;
                    deltaMs = m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(criteria).getMsAfterMidnight();
                    deltaMs -= m_SunRiseSetDatas[anEvalInx].whatCriteria(criteria).getMsAfterMidnight();
                    if (first)
                    {
                        minDeltaMs = deltaMs;
                        first = false;
                        continue;
                    }
                    deltaMsSign = (deltaMs < 0);
                    deltaMinDeltaMsSign = (minDeltaMs < 0);
                    if (deltaMsSign != deltaMinDeltaMsSign)
                    {
                        if (llabs(minDeltaMs) < llabs(deltaMs))
                        {
                            m_NextSim[criteria].index = anEvalInx - 1;
                            m_NextSim[criteria].valid = true;
                            break;
                        }
                        else
                        {
                            m_NextSim[criteria].index = anEvalInx;
                            m_NextSim[criteria].valid = true;
                            break;
                        }
                    }
                    minDeltaMs = deltaMs;
                }
            }


            if (criteria == SunRiseSetData::RISE)
            {
                criteria = SunRiseSetData::SET;
            }
            else if (criteria == SunRiseSetData::SET)
            {
                criteria = SunRiseSetData::DUR;
            }
            else
            {
                break;
            }
        }
    }
    return retVal;
}

std::ostringstream SunRiseSetDatas::printResult()                      // Create a Result as String
{
    std::ostringstream resultString;
    char msgTxt[256];

    if (m_TheEvalIndex.valid)
    {
        sprintf_s(msgTxt, sizeof(msgTxt), "          Date:         Sunrise     Sunset   Duration"); resultString << msgTxt << std::endl;
        //                                 Actual    yyyy-mm-dd   hh:mm:ss   hh:mm:ss   hh:mm:ss
        sprintf_s(msgTxt, sizeof(msgTxt), "Actual    %04d-%02d-%02d%5d:%02d:%02d%5d:%02d:%02d%5d:%02d:%02d",
            m_SunRiseSetDatas[m_TheEvalIndex.index].getYear(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].getMonth(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].getDay(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::RISE).getHour(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::RISE).getMin(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::RISE).getSec(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::SET).getHour(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::SET).getMin(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::SET).getSec(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::DUR).getHour(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::DUR).getMin(),
            m_SunRiseSetDatas[m_TheEvalIndex.index].whatCriteria(SunRiseSetData::DUR).getSec()
        );  resultString << msgTxt << std::endl;

        if (m_PrevSim[SunRiseSetData::RISE].valid)
        {
            sprintf_s(msgTxt, sizeof(msgTxt), "Previous  %04d-%02d-%02d%5d:%02d:%02d   --:--:--   --:--:--",
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::RISE].index].getYear(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::RISE].index].getMonth(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::RISE].index].getDay(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::RISE].index].whatCriteria(SunRiseSetData::RISE).getHour(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::RISE].index].whatCriteria(SunRiseSetData::RISE).getMin(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::RISE].index].whatCriteria(SunRiseSetData::RISE).getSec()
            ); resultString << msgTxt << std::endl;
        }
        if (m_PrevSim[SunRiseSetData::SET].valid)
        {
            sprintf_s(msgTxt, sizeof(msgTxt), "Previous  %04d-%02d-%02d   --:--:--%5d:%02d:%02d   --:--:--",
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::SET].index].getYear(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::SET].index].getMonth(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::SET].index].getDay(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::SET].index].whatCriteria(SunRiseSetData::SET).getHour(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::SET].index].whatCriteria(SunRiseSetData::SET).getMin(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::SET].index].whatCriteria(SunRiseSetData::SET).getSec()
            ); resultString << msgTxt << std::endl;
        }
        if (m_PrevSim[SunRiseSetData::DUR].valid)
        {
            sprintf_s(msgTxt, sizeof(msgTxt), "Previous  %04d-%02d-%02d   --:--:--   --:--:--%5d:%02d:%02d",
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::DUR].index].getYear(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::DUR].index].getMonth(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::DUR].index].getDay(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::DUR].index].whatCriteria(SunRiseSetData::DUR).getHour(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::DUR].index].whatCriteria(SunRiseSetData::DUR).getMin(),
                m_SunRiseSetDatas[m_PrevSim[SunRiseSetData::DUR].index].whatCriteria(SunRiseSetData::DUR).getSec()
            ); resultString << msgTxt << std::endl;          
        }

        if (m_NextSim[SunRiseSetData::RISE].valid)
        {
            sprintf_s(msgTxt, sizeof(msgTxt), "Next      %04d-%02d-%02d%5d:%02d:%02d   --:--:--   --:--:--",
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::RISE].index].getYear(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::RISE].index].getMonth(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::RISE].index].getDay(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::RISE].index].whatCriteria(SunRiseSetData::RISE).getHour(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::RISE].index].whatCriteria(SunRiseSetData::RISE).getMin(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::RISE].index].whatCriteria(SunRiseSetData::RISE).getSec()
            ); resultString << msgTxt << std::endl;
        }
        if (m_NextSim[SunRiseSetData::SET].valid)
        {
            sprintf_s(msgTxt, sizeof(msgTxt), "Next      %04d-%02d-%02d   --:--:--%5d:%02d:%02d   --:--:--",
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::SET].index].getYear(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::SET].index].getMonth(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::SET].index].getDay(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::SET].index].whatCriteria(SunRiseSetData::SET).getHour(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::SET].index].whatCriteria(SunRiseSetData::SET).getMin(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::SET].index].whatCriteria(SunRiseSetData::SET).getSec()
            ); resultString << msgTxt << std::endl;
        }
        if (m_NextSim[SunRiseSetData::DUR].valid)
        {
            sprintf_s(msgTxt, sizeof(msgTxt), "Next      %04d-%02d-%02d   --:--:--   --:--:--%5d:%02d:%02d",
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::DUR].index].getYear(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::DUR].index].getMonth(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::DUR].index].getDay(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::DUR].index].whatCriteria(SunRiseSetData::DUR).getHour(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::DUR].index].whatCriteria(SunRiseSetData::DUR).getMin(),
                m_SunRiseSetDatas[m_NextSim[SunRiseSetData::DUR].index].whatCriteria(SunRiseSetData::DUR).getSec()
            ); resultString << msgTxt << std::endl;
        }





    }
    else
    {
        resultString << "Input date not found" << std::endl;
    }

    return resultString;

}