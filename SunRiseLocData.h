#pragma once
class SunRiseLocData
{
protected:
  double longitude;
  double latitude;
  std::string locationName;
  std::string shortName;
  bool isDefaultLocation;

public:
  SunRiseLocData(double _longitude, double _latitude, const std::string& _locationName, const std::string &_shortName, bool _isDefault = false)
    : longitude(_longitude), latitude(_latitude), locationName(_locationName), shortName(_shortName), isDefaultLocation(_isDefault) {}
  const double &getLongitude() const { return longitude; }
  const double &getLatitude() const { return latitude; }
  const std::string& getLocationName() const { return locationName; }
  const std::string& getShortName() const { return shortName; } 
  const bool& isDefault() const { return isDefaultLocation; }
};

const SunRiseLocData* isKnownLocation(const std::string& shortName, const std::vector<SunRiseLocData>& locations);
const SunRiseLocData* getDefaultLocation(const std::vector<SunRiseLocData>& locations);