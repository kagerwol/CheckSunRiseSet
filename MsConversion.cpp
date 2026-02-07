#include "centralHeader.h"
#include "MsConversion.h"

// Constructor for MsConversion
MsConversion::MsConversion(
    unsigned long long __unixEpochUTCms,
    ConversionType theType,
    char _theSign)
    : unixEpochUTCms(__unixEpochUTCms)
    , isExtreme(0)
    , delta2Ref(0)
    , deltaRefSign(0)
{
    theMsSecondPart = unixEpochUTCms % 1000;
    if (theType == TIMESTAMP)
    {
        // Convert Unix epoch time to local time
        time_t secondsUnixEpoch = static_cast<time_t>(unixEpochUTCms / 1000);
        struct tm theUnRoundTarget;
        localtime_s(&theUnRoundTarget, &secondsUnixEpoch);
        theMsAfterMidnight = (((theUnRoundTarget.tm_hour * 60) + theUnRoundTarget.tm_min) * 60 + theUnRoundTarget.tm_sec) * 1000 + theMsSecondPart;
        secondsAfterMidnight = secondsUnixEpoch;
        // Rounding to whole seconds; a ms part for a Sunrise/Sunset is ridiculous
        if (theMsSecondPart >= 500)
        {
            secondsAfterMidnight++;
        }
        localtime_s(&theTarget, &secondsAfterMidnight);
        secondsAfterMidnight = theTarget.tm_hour * 3600 + theTarget.tm_min * 60 + theTarget.tm_sec;
    }
    else
    {
        // Handle duration or delta time
        theMsAfterMidnight = unixEpochUTCms;
        secondsAfterMidnight = static_cast<time_t>(unixEpochUTCms / 1000);
        // Rounding to whole seconds; a ms part for a Sunshine duration is ridiculous
        if (theMsSecondPart >= 500)
        {
            secondsAfterMidnight++;
        }
        memset(&theTarget, 0, sizeof(theTarget));
        theTarget.tm_hour = static_cast<int>(secondsAfterMidnight / 3600);
        theTarget.tm_min = static_cast<int>((secondsAfterMidnight % 3600) / 60);
        theTarget.tm_sec = static_cast<int>((secondsAfterMidnight % 3600) % 60);
    }

    return;     // to sender
}

// Copy constructor
MsConversion::MsConversion(const MsConversion& other)
    : unixEpochUTCms(other.unixEpochUTCms)
    , theMsSecondPart(other.theMsSecondPart)
    , theMsAfterMidnight(other.theMsAfterMidnight)
    , secondsAfterMidnight(other.secondsAfterMidnight)
    , theTarget(other.theTarget)
    , isExtreme(other.isExtreme)
    , delta2Ref(other.delta2Ref)
    , deltaRefSign(other.deltaRefSign)
{
}

// Assignment operator
MsConversion& MsConversion::operator=(const MsConversion& other)
{
    if (this != &other)
    {
        unixEpochUTCms = other.unixEpochUTCms;
        theMsSecondPart = other.theMsSecondPart;
        theMsAfterMidnight = other.theMsAfterMidnight;
        secondsAfterMidnight = other.secondsAfterMidnight;
        theTarget = other.theTarget;
        isExtreme = other.isExtreme;
        delta2Ref = other.delta2Ref;
        deltaRefSign = other.deltaRefSign;
    }
    return *this;
}

// Destructor
MsConversion::~MsConversion()
{
    return;     // to sender
}

// Calculate the delta to the reference time
unsigned long long MsConversion::calcDelta2Ref(unsigned long long otherTime)
{
    if (otherTime > theMsAfterMidnight)
    {
        delta2Ref = otherTime - theMsAfterMidnight;
        deltaRefSign = +1;  // Reference is bigger
    }
    else if (otherTime < theMsAfterMidnight)
    {
        delta2Ref = theMsAfterMidnight - otherTime;
        deltaRefSign = -1;  // Reference is smaller
    }
    else
    {
        delta2Ref = deltaRefSign = 0;
    }

    return delta2Ref;
}

