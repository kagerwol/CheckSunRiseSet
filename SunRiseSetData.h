#pragma once

const size_t RELEVANTINDICES = 5;

class SunRiseSetData
{
public:
	typedef enum {
		RISE = 0,									  // Evaluation of the Sunrise
		SET  = 1,						      	// Evaluation of the Sunset		
		DUR  = 2,                   // Evaluation of the Daylight
    DRISE = 3,                  // Evaluation of the Delta Sunrise to the previous day
    DSET = 4,                   // Evaluation of the Delta Sunset to the previous day
    DDUR = 5,                   // Evaluation of the Delta Daylight to the previous day
		NOCRITERIA
	} TimeCriteria;

protected:
  unsigned int m_Year;					       // The Year of the day to be examined
  unsigned int m_Month;					       // The Month of the day to be examined
  unsigned int m_Day;                  // The Day of the day to be examined
  MsConversion m_Sunrise;				       // The Sunrise of the day to be examined
  MsConversion m_Sunset;			       	 // The Sunset of the day to be examined
  MsConversion m_Daylength;            // The Daylight Time of the day to be examined
	MsConversion m_DeltaRise;            // The Delta Time of the sunrise to the previous day [ms]
	MsConversion m_DeltaSet;	           // The Delta Time of the sunset to the previous day [ms]
	MsConversion m_DeltaDur;             // The Delta Time of the daylight time to the previous day [ms]

public:
	inline const MsConversion& whatCriteria(TimeCriteria aType) const
	{
		switch (aType)
		{
		case RISE:
			return m_Sunrise;
		case SET:
			return m_Sunset;
		case DUR:
			return m_Daylength;
		case DRISE:
			return m_DeltaRise; // or some appropriate member for DELTA
    case DSET:
      return m_DeltaSet; // or some appropriate member for DELTA
    case DDUR:
      return m_DeltaDur; // or some appropriate member for DELTA
		default:
			return m_Daylength;
		}
	}
public:
	inline MsConversion& whatCriteriaNConst(TimeCriteria aType)
	{
		switch (aType)
		{
		case RISE:
			return m_Sunrise;
		case SET:
			return m_Sunset;
		case DUR:
			return m_Daylength;
		case DRISE:
			return m_DeltaRise; // or some appropriate member for DELTA
		case DSET:
			return m_DeltaSet; // or some appropriate member for DELTA
		case DDUR:
			return m_DeltaDur; // or some appropriate member for DELTA
		default:
			return m_Daylength;
		}
	}
public:
	inline const unsigned int& getYear() const { return m_Year; };
	inline const unsigned int& getMonth() const { return m_Month; };
	inline const unsigned int& getDay() const { return m_Day; };
	inline const unsigned long long& getUnixMs(TimeCriteria aType) const { return whatCriteria(aType).get_unixEpochUTCms(); };
	inline int& setExtreme(TimeCriteria aType, int _Extreme) { return whatCriteriaNConst(aType).setExtreme(_Extreme); };
	inline const int& getHour(TimeCriteria aType) const { return whatCriteria(aType).getHour(); };
	inline const int& getMin(TimeCriteria aType) const { return whatCriteria(aType).getMin(); };
	inline const int& getSec(TimeCriteria aType) const { return whatCriteria(aType).getSec(); };
	inline const unsigned long long& getSunriseUnixMs() const { return whatCriteria(RISE).get_unixEpochUTCms(); };
	inline const unsigned long long& getSunsetUnixMs() const { return whatCriteria(SET).get_unixEpochUTCms(); };
	inline const unsigned long long& getDaylengthMs() const { return whatCriteria(DUR).get_unixEpochUTCms(); };
	inline const int& getSunriseHour() const { return m_Sunrise.getHour(); };
	inline const int& getSunriseMin() const { return m_Sunrise.getMin(); };
	inline const int& getSunriseSec() const { return m_Sunrise.getSec(); };
	inline const int& getSunsetHour() const { return m_Sunset.getHour(); };
	inline const int& getSunsetMin() const { return m_Sunset.getMin(); };
	inline const int& getSunsetSec() const { return m_Sunset.getSec(); };
	inline const int& getDaylengthHour() const { return m_Daylength.getHour(); };
	inline const int& getDaylengthMin() const { return m_Daylength.getMin(); };
	inline const int& getDaylengthSec() const { return m_Daylength.getSec(); };
	inline const unsigned long long& getRiseMsAfterMidnight() const { return m_Sunrise.getMsAfterMidnight(); };
	inline const unsigned long long& getSetMsAfterMidnight() const { return m_Sunset.getMsAfterMidnight(); };
	inline const unsigned long long& getDurMsAfterMidnight() const { return m_Daylength.getMsAfterMidnight(); };

	SunRiseSetData(
		unsigned int _yyyy,
		unsigned int _mm,
		unsigned int _dd,
		unsigned long long _sunrise_unix_ms,
		unsigned long long _sunset_unix_ms,
		unsigned long long _day_length_ms
	);

	// Copy constructor
	SunRiseSetData(const SunRiseSetData& other);

	// Delta to previous
	void calcDeltaPrev(const SunRiseSetData& other);

	// Assignment operator
	SunRiseSetData& operator=(const SunRiseSetData& other);

	~SunRiseSetData();

	
};

class SunRiseSetDatas
{
public:

	struct IndexValidPair
	{
		size_t index;
		bool   valid;
		inline void init() { index = 0; valid = false; };
	};

protected:
	std::vector<SunRiseSetData> m_SunRiseSetDatas;

	IndexValidPair m_TheEvalIndex;

	IndexValidPair m_PrevTurn[SunRiseSetData::NOCRITERIA];
	IndexValidPair m_NextTurn[SunRiseSetData::NOCRITERIA];
	IndexValidPair m_PrevSim[SunRiseSetData::NOCRITERIA];
	IndexValidPair m_NextSim[SunRiseSetData::NOCRITERIA];

	std::vector<size_t> m_SortIndices[SunRiseSetData::NOCRITERIA];
	std::vector<size_t> m_RelevantIndices[SunRiseSetData::NOCRITERIA];

protected:

	int CheckForExtrema(const size_t anEvalIndex);
public:
	SunRiseSetDatas();
	~SunRiseSetDatas();


	// Fill one tupel of the vector of sunRiseSetDatas
	size_t addDayData(
		unsigned int Exam_yyyy,                     // This is the Year to become examimed
		unsigned int Exam_mm,                       // dito month
		unsigned int Exam_dd,						// dito day 
		unsigned int ActYear,                       // This is the Year of the actual day to be treated
		unsigned int ActMonth,  			        // dito month
		unsigned int ActDay,    		            // dito day
		unsigned long long sunrise_unix_ms,         // This is the Sunrise in ms after 1970-01-01 00:00:00
    char sunrise_marker,                        // This is the marker for the sunrise time, it is ' ' if the sun rises on this day and 'p' if previous day sunrise time is used, because the sun does not rise on this day
		unsigned long long sunset_unix_ms,          // This is the Sunset in ms after 1970-01-01 00:00:00
    char sunset_marker,                         // This is the marker for the sunset time, it is ' ' if the sun sets on this day and 'n' if next day sunset time is used, because the sun does not set on this day
		unsigned long long day_length_ms            // This is the length of the day in ms
	);

	int doAnalyzation();                            // Find all Previous and Next Turning Points
	std::ostringstream printResult();               // Create a Result as String

	inline const size_t size() const { return m_SunRiseSetDatas.size(); };
	size_t CalcDelta2Reference();
};