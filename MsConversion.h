#pragma once

class MsConversion
{
public:
	typedef enum {
		TIMESTAMP = 0,		// Handling a Time stamp
		DURATION = 1        // Handling a Duration or Delta time
	} ConversionType;       
protected:
	unsigned long long delta2Ref;              // Delta in ms to the Reference
	int                deltaRefSign;           // + Reference value bigger - Reference Value smaller, else 0
	unsigned long long unixEpochUTCms;	       // This is the number of miliseconds either in utc since 1970-01-01 00:00:00 or simple the number of miliseconds in a duration
	unsigned long long theMsAfterMidnight;     // This is the number of miliseconds after midnight in the local time zone in case of TIMESTAMP, otherwise same as above
	time_t secondsAfterMidnight;               // This is the number of Seconds after midnight rounded to full seconds in the local time zone in case of TIMESTAMP or otherwise the duration in Seconds rounded to full seconds
	unsigned long long theMsSecondPart;        // This is the milisecond part of a TIMESTAMP or Duration
	struct tm theTarget;					   // This is the time structure of the local time zone in case of TIMESTAMP (Year, Month, Day, Hour, Minute, Second) or the duration in case of DURATION but only in Hours, Minutes and Seconds
  struct tm theDelta2Ref;					 // This is a Delta Time stamp in Miliseconds
	int isExtreme;                             // -1 if this is somewhat earliest or shortest in environment, +1 if this is somewhat latest or longest in environment
public:
	MsConversion(unsigned long long _unixEpochUTCms, ConversionType theType); // Usual constructor
	MsConversion(const MsConversion& other);   // Copy constructor
	MsConversion& operator=(const MsConversion& other);   // Assign operator
	virtual ~MsConversion();                   // Destructor
	unsigned long long calcDelta2Ref(const MsConversion& otherTime); // Calculate Delta Time to a reference time

	// Accessors
	inline const unsigned long long& get_unixEpochUTCms() const { return unixEpochUTCms; };
	inline unsigned long long getSecondsAfterMidnight() const { return secondsAfterMidnight; };
	inline const unsigned long long& getMsAfterMidnight() const { return theMsAfterMidnight; };
	inline const unsigned long long& getMs() const { return theMsSecondPart; };
  inline const char getDSign() const { return (deltaRefSign > 0) ? '-' : ((deltaRefSign < 0) ? '+' : ' '); };
	inline const int& getHour() const { return theTarget.tm_hour; };
	inline const int& getMin() const { return theTarget.tm_min; };
	inline const int& getSec() const { return theTarget.tm_sec; };
	inline const int& getDHour() const { return theDelta2Ref.tm_hour; };
	inline const int& getDMin() const { return theDelta2Ref.tm_min; };
	inline const int& getDSec() const { return theDelta2Ref.tm_sec; };
	inline const int& getExtreme() const { return isExtreme; };
	inline int& setExtreme(int _extreme) { isExtreme = _extreme; return isExtreme; };
	inline const unsigned long long& getDeltaRef() const { return delta2Ref; };
};