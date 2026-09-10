#pragma once

// A Catch2 reporter for benchmarks of CPU- and memory-bound code, where interference only ever adds time:
// preemption, interrupts, a busy sibling thread, a page fault. Under that one-sided noise the fastest samples
// are the honest ones, so the reported figure is the mean of the fastest CATCH_BENCHMARK_FASTEST_FRACTION of
// them. Against the plain minimum that rests on one observation, a mean over several is far steadier at the
// sample counts a large suite can afford; min and median come along to show the spread.
//
// Pass --benchmark-no-analysis: this reads the raw samples, and Catch2's bootstrap is then pure cost.
//
// Define CATCH_BENCHMARK_REPORTER_IMPLEMENTATION in exactly one translation unit to register it, then select
// it with -r fastest.

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// The share of the samples, fastest first, that the reported mean is taken over.
#ifndef CATCH_BENCHMARK_FASTEST_FRACTION
#define CATCH_BENCHMARK_FASTEST_FRACTION (1.0 / 3.0)
#endif

namespace CatchBenchmark {

constexpr int nameColumnWidth = 38;
constexpr int valueColumnWidth = 14;

[[nodiscard]] inline std::string formatDuration(double nanoseconds)
{
	const char* unit = "ns";
	double value = nanoseconds;
	for (const char* larger : { "us", "ms", "s" })
	{
		if (value < 1000.0)
			break;

		value /= 1000.0;
		unit = larger;
	}

	std::ostringstream text;
	text << std::fixed << std::setprecision(value < 10.0 ? 2 : (value < 100.0 ? 1 : 0)) << value << ' ' << unit;
	return text.str();
}

class FastestQuantileReporter : public Catch::ConsoleReporter
{
public:
	using ConsoleReporter::ConsoleReporter;

	static std::string getDescription()
	{
		return "Benchmarks: mean of the fastest samples, with min and median";
	}

	// The base implementations write into ConsoleReporter's benchmark table. None of them are called, so that
	// table is never opened and its output cannot interleave with the lines written here.
	void benchmarkPreparing(const std::string& name) override { _benchmarkName = name; }
	void benchmarkStarting(const Catch::BenchmarkInfo& info) override { _sampleCount = info.samples; }

	void benchmarkFailed(const std::string& error) override
	{
		printHeaderOnce();
		stream << std::left << std::setw(nameColumnWidth) << _benchmarkName << "  failed: " << error << '\n';
	}

	void benchmarkEnded(const Catch::BenchmarkStats<>& stats) override
	{
		if (stats.samples.empty())
			return;

		std::vector<double> nanoseconds;
		nanoseconds.reserve(stats.samples.size());
		for (const auto& sample : stats.samples)
			nanoseconds.push_back(sample.count());

		std::sort(nanoseconds.begin(), nanoseconds.end());

		const size_t fastestCount = fastestSampleCount(nanoseconds.size());
		const double fastestMean =
			std::accumulate(nanoseconds.begin(), nanoseconds.begin() + fastestCount, 0.0) / static_cast<double>(fastestCount);

		printHeaderOnce();
		stream << std::left << std::setw(nameColumnWidth) << _benchmarkName << std::right
			<< std::setw(valueColumnWidth) << formatDuration(fastestMean)
			<< std::setw(valueColumnWidth) << formatDuration(nanoseconds.front())
			<< std::setw(valueColumnWidth) << formatDuration(median(nanoseconds)) << '\n';
	}

	void testCaseStarting(const Catch::TestCaseInfo& info) override
	{
		ConsoleReporter::testCaseStarting(info);
		_testCaseName = info.name;
		_headerPrinted = false;
	}

	// Samples are already sorted here, so the count doubles as the index of the first one left out.
	[[nodiscard]] static size_t fastestSampleCount(size_t sampleCount) noexcept
	{
		const auto share = static_cast<size_t>(static_cast<double>(sampleCount) * CATCH_BENCHMARK_FASTEST_FRACTION);
		return std::clamp<size_t>(share, 1, sampleCount);
	}

private:
	[[nodiscard]] static double median(const std::vector<double>& sorted) noexcept
	{
		const size_t middle = sorted.size() / 2;
		return sorted.size() % 2 == 1 ? sorted[middle] : (sorted[middle - 1] + sorted[middle]) / 2.0;
	}

	// Deferred to the first result, so a test case holding no benchmarks prints nothing.
	void printHeaderOnce()
	{
		if (_headerPrinted)
			return;

		_headerPrinted = true;
		stream << '\n' << _testCaseName << "  (" << _sampleCount << " samples, mean of the fastest "
			<< fastestSampleCount(static_cast<size_t>(_sampleCount)) << ")\n"
			<< std::left << std::setw(nameColumnWidth) << "benchmark name" << std::right
			<< std::setw(valueColumnWidth) << "fastest mean"
			<< std::setw(valueColumnWidth) << "min"
			<< std::setw(valueColumnWidth) << "median" << '\n'
			<< std::string(nameColumnWidth + 3 * valueColumnWidth, '-') << '\n';
	}

	std::string _testCaseName;
	std::string _benchmarkName;
	int _sampleCount = 0;
	bool _headerPrinted = false;
};

}

#ifdef CATCH_BENCHMARK_REPORTER_IMPLEMENTATION
// CATCH_REGISTER_REPORTER pastes the type name into an identifier, so it needs an unqualified one.
using CatchBenchmarkFastestQuantileReporter = CatchBenchmark::FastestQuantileReporter;
CATCH_REGISTER_REPORTER("fastest", CatchBenchmarkFastestQuantileReporter)
#endif
