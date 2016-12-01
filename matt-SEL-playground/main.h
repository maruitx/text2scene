
#pragma once

//#define USE_MATLAB

#include "mLibInclude.h"

#include "appParams.h"

#include "app.h"

extern AppParameters* g_appParams;
inline const AppParameters& appParams()
{
	return *g_appParams;
}

inline AppParameters& appParamsMutable()
{
	return *g_appParams;
}
