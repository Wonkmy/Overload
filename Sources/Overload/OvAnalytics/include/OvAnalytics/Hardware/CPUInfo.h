/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <stdint.h>

namespace OvAnalytics::Hardware
{
	/**
	* The CPUInfo class will gather informations about the CPU usage
	*/
	class CPUInfo final
	{
	public:
		/**
		* Calculate the CPU load for every process on the machine (%)
		*/
		float CalculateCPULoad();

	private:
		float CalculateCPULoad(uint64_t idleTicks, uint64_t totalTicks);

	private:
		uint64_t m_cpuPreviousTotalTicks = 0;
		uint64_t m_cpuPreviousIdleTicks = 0;
	};
}