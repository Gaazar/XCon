#pragma once

#include "json.hpp"
#include <map>
#include <string>
#include <vector>

extern configor::wjson configs;
void SaveConfigs();
void OSDUpdateYPR(float y, float p, float r);
void OSDUpdateGPS(float lat, float lon);
void OSDUpdateSpeed(float as, float gs);
void OSDUpdateAlt(float alt,float climb);
void CalibrateShow();

