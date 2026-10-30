#pragma once

#ifdef _WIN32
#include <Windows.h>

#include <bit>
#include <cstdint>
#include <vector>
#endif

// Restricts the process to its fastest cores: on a hybrid CPU the performance cores, elsewhere every core.
// Returns how many logical processors it now runs on, for sizing worker pools; 0 when the process stays unpinned.
// Windows only, processor group 0 only; returns 0 elsewhere.
inline int pinToPerformanceCores()
{
#ifdef _WIN32
	const HANDLE process = ::GetCurrentProcess();
	ULONG length = 0;
	::GetSystemCpuSetInformation(nullptr, 0, &length, process, 0);
	std::vector<char> buffer(length);
	if (length == 0 || !::GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(buffer.data()), length, &length, process, 0))
		return 0;

	// Entries vary in size: each states its own
	const auto forEachCpuSet = [&](auto&& visit) {
		for (ULONG offset = 0; offset < length;)
		{
			const auto& info = *reinterpret_cast<const SYSTEM_CPU_SET_INFORMATION*>(buffer.data() + offset);
			if (info.Type == CpuSetInformation && info.CpuSet.Group == 0)
				visit(info.CpuSet);
			offset += info.Size;
		}
	};

	// A higher efficiency class is faster
	BYTE fastestClass = 0;
	forEachCpuSet([&](const auto& cpuSet) {
		if (cpuSet.EfficiencyClass > fastestClass)
			fastestClass = cpuSet.EfficiencyClass;
	});

	DWORD_PTR fastestMask = 0;
	forEachCpuSet([&](const auto& cpuSet) {
		if (cpuSet.EfficiencyClass == fastestClass)
			fastestMask |= DWORD_PTR{ 1 } << cpuSet.LogicalProcessorIndex;
	});

	// Never widened: a job object or the launcher may already restrict the process
	DWORD_PTR processMask = 0, systemMask = 0;
	if (!::GetProcessAffinityMask(process, &processMask, &systemMask))
		return 0;
	fastestMask &= processMask;
	if (fastestMask == 0 || !::SetProcessAffinityMask(process, fastestMask))
		return 0;

	return std::popcount(static_cast<uint64_t>(fastestMask));
#else
	return 0;
#endif
}
