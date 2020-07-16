/*!
	\brief Some inline functions to make performance testing easier
*/

#include <string>
#include <chrono>

using namespace std::chrono;
using std::string;

// Set the type of clock by changing this variable here
using this_clock = std::chrono::high_resolution_clock;
using timepoint = this_clock::time_point;

/*
	\brief Results of a trial with a start/end point.
*/
struct trial {

	timepoint start{}; //
	timepoint end{};

	/*! \brief Create a new clock and start it if auto_start is true.*/
	inline trial::trial(bool auto_start = true) {
		if (auto_start)
			start_clock();
	}

	/*! \brief Sets the start time of this trial to the current time.*/
	inline void start_clock() {
		start = this_clock::now();
	}


	/*! \brief Get the time between the current time and the previous time in units.*/
	inline long long startToNow() const {
		return duration_cast<std::chrono::milliseconds>(this_clock::now() - start).count();
	}

	/*! \brief Set the end time of this trial. */
	inline void stop() {
		end = this_clock::now();
	}

	/*! \brief Get the duration from start to now. */
	inline long long getDuration() const {
		return duration_cast<std::chrono::milliseconds>(end - start).count();
	}
};


/*! 
	\brief A function to uniformly print trials to std::cerr.

	\param trials Trials to print. Used to get the time of execution
	\param num_units number of units were processed by each trial in order.
	\param unit_name Name of units being processed. I.E. the graph generator would put Nodes.
*/
inline void PrintTrials(
	const std::vector<trial> & trials,
	const std::vector<int> num_units,
	std::string unit_name = "Elements"
) {
	for (int i = 0; i < trials.size(); i++) {
		auto num_elements = num_units[i];
		auto& this_trial = trials[i];
		auto duration = this_trial.getDuration();

		float units_per_ms = static_cast<float>(num_elements) / static_cast<float>(duration);

		std::cerr << "TRIAL " << i << " | "
			<< "Count of " << unit_name << ": " << num_elements << ", "
			<< "Time: " << duration << "ms, "
			<< unit_name << " / ms: " << units_per_ms
			<< std::endl;
	}
}
