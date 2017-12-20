#pragma once

#include "targetver.h"

#include <Windows.h>
#include <unordered_map>

#ifdef _DEBUG
#include <iostream>
#endif

#include <comdef.h>
//  Include the task header file.
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")