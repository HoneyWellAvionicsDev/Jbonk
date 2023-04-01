#pragma once

#include "Jbonk/Core/PlatformDetection.h"

#ifdef HZ_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX 
	#endif
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <chrono>
#include <thread>
#include <algorithm>
#include <functional>

#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <atomic>

#include "Jbonk/Core/Core.h"
#include "Jbonk/Core/Log.h"
#include "Jbonk/Core/Timer.h"
#include "Jbonk/Core/KeyCodes.h"
#include "Jbonk/Debug/Instrumentor.h"

#ifdef HZ_PLATFORM_WINDOWS
	#include <Windows.h>
#endif