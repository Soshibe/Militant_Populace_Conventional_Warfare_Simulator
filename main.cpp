#include <vector>
#include <chrono>
#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <algorithm>
#include <limits>
#include <array>
#include <numeric>
#include <unordered_set>

double simulation_age_years;
double simulation_birthtime;
bool random_wars_enabled = false;
int threads = 4;

class policy {
public:
	policy() {}
	std::string name = "Global Policy";
	double age_of_conscription = 13;
	double age_of_retirement = 35;
	double life_expectancy = 25;
	double fertile_age_start = 10;
	double fertile_age_end = 50;
	double allowed_pregnancy_age_start = 10;
	double allowed_pregnancy_age_end = 50;
	double allowed_number_of_children = 300;
	double pregnancies_per_lifetime = 40;
	double years_between_pregnancies = 1;
	double twinning_chance = 0.1;
	double learning_rate_mutation_range = 0.05;
	double learning_rate_median = 50.0;
	double learning_rate_range = 100.0;
	double conscription_minimum_learning_rate = 0.0;
	double chance_to_declare_war_per_year = 0.01;
	double minimum_age_genocide = 0;
	double maximum_age_genocide = 110;
	double chance_to_commit_genocide_during_war_success = 80.0;
	bool pacifist = false;
};

std::vector<policy> available_policies;
policy global_policy;	

void setup()
{
	available_policies.push_back(global_policy);
	simulation_age_years = 0;
	simulation_birthtime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}

class soldier {
	double birthtime;
public:
	double learning_rate = 0;
	double number_of_children = 0;
	double number_of_pregnancies = 0;
	policy fealty;
	soldier(policy conscribed_policy) {
		fealty = conscribed_policy;
		// Fix: Use simulation time, not wall-clock time
		bool positive_or_negative = (rand() % 2 == 0) ? 1 : -1;
		bool positive_or_negative2 = (rand() % 2 == 0) ? 1 : -1;
		learning_rate = fealty.learning_rate_median + (positive_or_negative * ((static_cast<double>(rand()) / RAND_MAX) - 0.5) * (fealty.learning_rate_range / 2)); // Randomize learning rate around median within range
		learning_rate += positive_or_negative2 * ((static_cast<double>(rand()) / RAND_MAX) * fealty.learning_rate_mutation_range); // Apply mutation
		birthtime = simulation_age_years * 31557600 + simulation_birthtime;
	}
	double get_age_years() const
	{
		double current_time = simulation_age_years * 31557600 + simulation_birthtime;
		return (current_time - birthtime) / 31557600;
	}
};

std::vector<soldier> armies;



policy who_wins_conventional_war(policy a, policy b)
{
	int total_soldiers_a = 0;
	int total_soldiers_b = 0;
	if (a.pacifist && !b.pacifist) {
		return b;
	}
	else if (!a.pacifist && b.pacifist) {
		return a;
	}
	for (auto& soldiers : armies) {
		if (a.name == soldiers.fealty.name && soldiers.get_age_years() >= a.age_of_conscription && soldiers.get_age_years() < a.age_of_retirement) {
			total_soldiers_a++;
		}
		else if (b.name == soldiers.fealty.name && soldiers.get_age_years() >= b.age_of_conscription && soldiers.get_age_years() < b.age_of_retirement) {
			total_soldiers_b++;
		}
	}
	if (total_soldiers_a == total_soldiers_b) {
		return global_policy; // Tie goes to global policy
	}
	std::cout << "Total soldiers for " << a.name << ": " << total_soldiers_a << "\n";
	std::cout << "Total soldiers for " << b.name << ": " << total_soldiers_b << "\n";

	if (total_soldiers_a > total_soldiers_b) {
		return a;
	}
	else {
		return b;
	}
}


double calculate_average_learning_rate()
{
	if (armies.empty()) return 0.0;
	double total_learning_rate = 0.0;
	for (const auto& soldier : armies) {
		total_learning_rate += soldier.learning_rate;
	}
	return total_learning_rate / armies.size();
}

double smartest_soldier_learning_rate()
{
	if (armies.empty()) return 0.0;
	double max_learning_rate = armies[0].learning_rate;
	for (const auto& soldier : armies) {
		if (soldier.learning_rate > max_learning_rate) {
			max_learning_rate = soldier.learning_rate;
		}
	}
	return max_learning_rate;
}

double dumbest_soldier_learning_rate()
{
	if (armies.empty()) return 0.0;
	double min_learning_rate = armies[0].learning_rate;
	for (const auto& soldier : armies) {
		if (soldier.learning_rate < min_learning_rate) {
			min_learning_rate = soldier.learning_rate;
		}
	}
	return min_learning_rate;
}

double calculate_policy_learning_rate_median(const policy& pol)
{
	double total_learning_rate = 0.0;
	int count = 0;
	for (const auto& soldier : armies) {
		if (soldier.fealty.name == pol.name) {
			total_learning_rate += soldier.learning_rate;
			count++;
		}
	}
	if (count == 0) return 0.0;
	return total_learning_rate / count;
}

double calculate_policy_learning_rate_range(const policy& pol)
{
	double min_learning_rate = std::numeric_limits<double>::max();
	double max_learning_rate = std::numeric_limits<double>::lowest();
	for (const auto& soldier : armies) {
		if (soldier.fealty.name == pol.name) {
			if (soldier.learning_rate < min_learning_rate) {
				min_learning_rate = soldier.learning_rate;
			}
			if (soldier.learning_rate > max_learning_rate) {
				max_learning_rate = soldier.learning_rate;
			}
		}
	}
	if (min_learning_rate == std::numeric_limits<double>::max() || max_learning_rate == std::numeric_limits<double>::lowest()) {
		return 0.0;
	}
	return max_learning_rate - min_learning_rate;
}

void print_fealty_intelligence_distribution()
{
	std::cout << "Fealty Intelligence Distribution:\n";
	int pol_index = 0;
	for (const auto& pol : available_policies) {
		double total_learning_rate = 0.0;
		int count = 0;
		for (const auto& soldier : armies) {
			if (soldier.fealty.name == pol.name) {
				total_learning_rate += soldier.learning_rate;
				count++;
			}
		}
		if (count > 0) {
			std::cout << "Policy: " << pol.name << ", Average Learning Rate: " << (total_learning_rate / count) << ", Number of Soldiers: " << count << "\n";
			available_policies[pol_index].learning_rate_median = calculate_policy_learning_rate_median(pol);
			available_policies[pol_index].learning_rate_range = calculate_policy_learning_rate_range(pol);
		}
		else {
			std::cout << "Policy: " << pol.name << ", No soldiers.\n";
		}
	}
}

void print_most_intelligent_policy()
{
	if (available_policies.empty()) return;
	policy* smartest_policy = &available_policies[0];
	for (auto& pol : available_policies) {
		if (calculate_policy_learning_rate_median(pol) > calculate_policy_learning_rate_median(*smartest_policy)) {
			smartest_policy = &pol;
		}
	}
	std::cout << "Most Intelligent Policy: " << smartest_policy->name << " with Average Learning Rate: " << calculate_policy_learning_rate_median(*smartest_policy) << "\n";
}

void print_alarm_if_mentally_retarded()
{
	double average_learning_rate = calculate_average_learning_rate();
	if (average_learning_rate < 50.0) {
		std::cout << "!!ALARM: Average learning rate has fallen below 50.0! Civilization is at risk of collapse, immediate action required!\n";
		std::cout << "Consider implementing policies to increase learning rates and intelligence.\n";
		std::cout << "Encourage low breeding ages, low conscription intelligence requirements.\n";
		std::cout << "EXTREME PROBLEM: Earth's population has become mentally retarded!\n";
	}
}

void help()
{
	std::cout << "Type commands to interact with the simulation:\n";
	std::cout << "Type 'help' to see this message again.\n";
	std::cout << "Type threads <number> to set the number of threads for parallel processing (default is 4).\n";
	std::cout << "Type STANDARD_WAR to advance 50 years with USA and ENEMY policies and simulate a war between them.\n";
	std::cout << "Press Enter to advance one year in the simulation, \n";
	std::cout << "type policy <name> to add a new policy type, \n";
	std::cout << "type add <number> <policy> to add soldiers with fealty sworn to a specific policy, \n";
	std::cout << "type sex_age <policy> <age> to set the reproductive age for a policy, \n";
	std::cout << "type intelligence <policy> <median> <range> to set the intelligence median and range for a policy, \n";
	std::cout << "type warchoice <policy> <chance> to set the chance to declare war per year for a policy (in percent), \n";
	std::cout << "type conscription_age <policy> <age> to set the conscription age for a policy, \n";
	std::cout << "type retirement_age <policy> <age> to set the retirement age for a policy, \n";
	std::cout << "type life_expectancy <policy> <age> to set the life expectancy for a policy, \n";
	std::cout << "type fertility_range <policy> <start_age> <end_age> to set the fertility range for a policy, \n";
	std::cout << "type pregnancies_per_lifetime <policy> <number> to set the number of pregnancies per lifetime for a policy, \n";
	std::cout << "type number_of_children <policy> <number> to set the allowed number of children per soldier for a policy, \n";
	std::cout << "type random_war to allow wars to occur randomly between existing policies based on their chance to declare war per year, \n";
	std::cout << "or type war <policy1> <policy2> to simulate a war between two policies.\n";
}

void add_two_standard_policies()
{
	policy usa_policy;
	usa_policy.name = "USA";
	usa_policy.age_of_conscription = 18;
	usa_policy.age_of_retirement = 45;
	usa_policy.life_expectancy = 79;
	usa_policy.fertile_age_start = 15;
	usa_policy.fertile_age_end = 45;
	usa_policy.allowed_pregnancy_age_start = 15;
	usa_policy.allowed_pregnancy_age_end = 45;
	usa_policy.allowed_number_of_children = 10;
	usa_policy.pregnancies_per_lifetime = 5;
	usa_policy.conscription_minimum_learning_rate = 80.0;
	usa_policy.learning_rate_median = 70.0;
	usa_policy.learning_rate_range = 50.0;
	usa_policy.chance_to_declare_war_per_year = 0.05;
	usa_policy.maximum_age_genocide = 65;
	usa_policy.minimum_age_genocide = 15;

	available_policies.push_back(usa_policy);
	policy enemy_policy;
	enemy_policy.name = "ENEMY";
	enemy_policy.age_of_conscription = 13;
	enemy_policy.age_of_retirement = 60;
	enemy_policy.life_expectancy = 65;
	enemy_policy.fertile_age_start = 10;
	enemy_policy.fertile_age_end = 50;
	enemy_policy.allowed_pregnancy_age_start = 10;
	enemy_policy.allowed_pregnancy_age_end = 50;
	enemy_policy.allowed_number_of_children = 300;
	enemy_policy.pregnancies_per_lifetime = 30;
	enemy_policy.learning_rate_median = 40.0;
	enemy_policy.learning_rate_range = 25.0;
	enemy_policy.conscription_minimum_learning_rate = 0.0;
	enemy_policy.chance_to_declare_war_per_year = 0.01;
	enemy_policy.maximum_age_genocide = 70;
	enemy_policy.minimum_age_genocide = 7;
	available_policies.push_back(enemy_policy);
}
/// <summary>
/// ////////
/// th
/// </summary>
/// <returns></returns>
/// 

int soldiers_in_policy(policy pol)
{
	int count = 0;
	for (const auto& soldier : armies) {
		if (soldier.fealty.name == pol.name) {
			count++;
		}
	}
	return count;
}

int maxint(int a, int b)
{
	return (a > b) ? a : b;
}

double percent_to_kill(int soldiers_in_policyA, int soldiers_in_policyB) // assumes soldiers_in_policyA are the losers in the context.
{
	double median = 50.0;
	double range = 100.0;
	int max_delta_factor_of_army_size_difference = maxint(soldiers_in_policyA, soldiers_in_policyB) / 10;
	
	int difference_in_army_size = abs(soldiers_in_policyA - soldiers_in_policyB);
	bool maxxed = difference_in_army_size > max_delta_factor_of_army_size_difference;
	if (maxxed) {
		return median + (range / 2);
	}
	else {
		return median + ((difference_in_army_size / static_cast<double>(max_delta_factor_of_army_size_difference)) * (range / 2));
	}
	
}

void random_war_kill_most_losers_kill_some_winners()
{
	policy random_policyA;
	policy random_policyB;
	int total_policies = available_policies.size();
	int random_indexA = rand() % total_policies;
	int random_indexB = rand() % total_policies;
	while(random_indexB == random_indexA) {
		random_indexB = rand() % total_policies;
	}
	random_policyA = available_policies[random_indexA];
	random_policyB = available_policies[random_indexB];
	policy winner = who_wins_conventional_war(random_policyA, random_policyB);
	policy loser = (winner.name == random_policyA.name) ? random_policyB : random_policyA;
	std::vector<size_t> to_erase;
	double percent_kill_losers = percent_to_kill(soldiers_in_policy(loser), soldiers_in_policy(winner));
	for (size_t i = 0; i < armies.size(); ++i) {
		auto& soldiers = armies[i];
		if (soldiers.fealty.name == winner.name && soldiers.learning_rate > soldiers.fealty.conscription_minimum_learning_rate && soldiers.get_age_years()<soldiers.fealty.age_of_retirement && soldiers.get_age_years() > soldiers.fealty.age_of_conscription) {
			if ((static_cast<double>(rand()) / RAND_MAX) < (1-(percent_kill_losers / 100))) {
				to_erase.push_back(i);
			}
		}
		else {
			if(soldiers.fealty.name == loser.name && soldiers.learning_rate > soldiers.fealty.conscription_minimum_learning_rate && soldiers.get_age_years() < soldiers.fealty.age_of_retirement && soldiers.get_age_years() > soldiers.fealty.age_of_conscription)
			if ((static_cast<double>(rand()) / RAND_MAX) < (percent_kill_losers/100)) {
				to_erase.push_back(i);
			}
		}
	}

	std::cout << "A war has occurred between " << random_policyA.name << " and " << random_policyB.name << ". The winner is " << winner.name << ".\n";
	std::cout << "Number of soldiers killed in the war: " << to_erase.size() << "\n";
	std::cout << "Number of soldiers in Policy " << random_policyA.name << " died: ";
	int countA = 0;
	for(auto idx : to_erase) {
		if(armies[idx].fealty.name == random_policyA.name) {
			countA++;
		}
	}
	std::cout << countA << "\n";
	std::cout << "Number of soldiers in Policy " << random_policyB.name << " died: ";
	int countB = to_erase.size() - countA;
	std::cout << countB << "\n";

	// Remove soldiers who died (from back to front to avoid index shifting)
	for (auto it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
		armies.erase(armies.begin() + *it);
	}
}

void war_kill_most_losers_kill_some_winners(policy random_policyA, policy random_policyB)
{
	policy winner = who_wins_conventional_war(random_policyA, random_policyB);
	policy loser = (winner.name == random_policyA.name) ? random_policyB : random_policyA;
	std::vector<size_t> to_erase;
	double percent_kill_losers = percent_to_kill(soldiers_in_policy(loser), soldiers_in_policy(winner));
	for (size_t i = 0; i < armies.size(); ++i) {
		auto& soldiers = armies[i];
		if (soldiers.fealty.name == winner.name && soldiers.learning_rate > soldiers.fealty.conscription_minimum_learning_rate && soldiers.get_age_years() < soldiers.fealty.age_of_retirement && soldiers.get_age_years() > soldiers.fealty.age_of_conscription) {
			if ((static_cast<double>(rand()) / RAND_MAX) < (1 - (percent_kill_losers / 100))) {
				to_erase.push_back(i);
			}
		}
		else {
			if (soldiers.fealty.name == loser.name && soldiers.learning_rate > soldiers.fealty.conscription_minimum_learning_rate && soldiers.get_age_years() < soldiers.fealty.age_of_retirement && soldiers.get_age_years() > soldiers.fealty.age_of_conscription)
				if ((static_cast<double>(rand()) / RAND_MAX) < (percent_kill_losers / 100)) {
					to_erase.push_back(i);
				}
		}
	}

	std::cout << "A war has occurred between " << random_policyA.name << " and " << random_policyB.name << ". The winner is " << winner.name << ".\n";
	std::cout << "Number of soldiers killed in the war: " << to_erase.size() << "\n";
	std::cout << "Number of soldiers in Policy " << random_policyA.name << " died: ";
	int countA = 0;
	for (auto idx : to_erase) {
		if (armies[idx].fealty.name == random_policyA.name) {
			countA++;
		}
	}
	std::cout << countA << "\n";
	std::cout << "Number of soldiers in Policy " << random_policyB.name << " died: ";
	int countB = to_erase.size() - countA;
	std::cout << countB << "\n";

	// Remove soldiers who died (from back to front to avoid index shifting)
	for (auto it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
		armies.erase(armies.begin() + *it);
	}
}

void add_year()
{
	simulation_age_years++;
	std::vector<policy> new_soldiers;
	std::vector<size_t> to_erase;

	double number_of_babies_this_year = 0;
	double number_of_deaths_this_year = 0;
	double number_of_retirements_this_year = 0;
	double number_of_conscriptions_this_year = 0;
	double number_of_new_fertile_soldiers = 0;
	double number_of_military_aged_soldiers = 0;
	double number_of_pregnancies_this_year = 0;
	double number_of_fertile_soldiers = 0;

	// Parallel processing setup
	size_t n = armies.size();
	int worker_threads = std::max(1, threads - 1);

	if (n == 0) {
		// nothing to do
		return;
	}

	if (worker_threads <= 1 || n < static_cast<size_t>(worker_threads)) {
		// single-threaded path (preserve original behavior)
		for (size_t i = 0; i < armies.size(); ++i) {
			auto& soldiers = armies[i];
			double age = soldiers.get_age_years();
			if (age >= 9 && age < 10) {
				number_of_new_fertile_soldiers++;
			}
			if (age >= soldiers.fealty.age_of_conscription && age < soldiers.fealty.age_of_conscription + 1) {
				number_of_conscriptions_this_year++;
			}
			if (age >= soldiers.fealty.age_of_retirement && age < soldiers.fealty.age_of_retirement + 1) {
				number_of_retirements_this_year++;
			}
			if (age >= soldiers.fealty.life_expectancy) {
				number_of_deaths_this_year++;
				to_erase.push_back(i);
				continue;
			}
			if (age >= soldiers.fealty.age_of_conscription && age < soldiers.fealty.age_of_retirement && soldiers.learning_rate > soldiers.fealty.conscription_minimum_learning_rate) {
				number_of_military_aged_soldiers++;
			}
			if (age >= soldiers.fealty.fertile_age_start && age < soldiers.fealty.fertile_age_end) {
				number_of_fertile_soldiers++;
			}
			if (age >= soldiers.fealty.allowed_pregnancy_age_start && age < soldiers.fealty.allowed_pregnancy_age_end && soldiers.number_of_children < soldiers.fealty.allowed_number_of_children && soldiers.fealty.pregnancies_per_lifetime > soldiers.number_of_pregnancies) {
				number_of_babies_this_year++;
				number_of_pregnancies_this_year++;
				armies[i].number_of_pregnancies++;
				armies[i].number_of_children++;
				new_soldiers.push_back(soldiers.fealty);
				if ((static_cast<double>(rand()) / RAND_MAX) < soldiers.fealty.twinning_chance) {
					number_of_babies_this_year++;
					new_soldiers.push_back(soldiers.fealty);
					armies[i].number_of_children++;
				}
			}
		}
	} else {
		// multi-threaded path: each thread reads armies and accumulates local results (no mutation)
		size_t chunk = (n + worker_threads - 1) / worker_threads;
		std::vector<std::vector<size_t>> to_erase_parts(worker_threads);
		std::vector<std::vector<policy>> new_soldiers_parts(worker_threads);
		std::vector<std::vector<size_t>> pregnancy_parent_indices(worker_threads); // indices to increment pregnancies/children
		std::vector<std::array<double,8>> counters(worker_threads);
		for (int t = 0; t < worker_threads; ++t) counters[t].fill(0.0);

		std::vector<std::thread> workers;
		workers.reserve(worker_threads);
		for (int t = 0; t < worker_threads; ++t) {
			size_t start = t * chunk;
			size_t end = std::min(n, start + chunk);
			workers.emplace_back([start,end,t,&to_erase_parts,&new_soldiers_parts,&pregnancy_parent_indices,&counters]() {
				for (size_t i = start; i < end; ++i) {
					auto const& soldiers = armies[i]; // read-only
					double age = soldiers.get_age_years();
					if (age >= 9 && age < 10) counters[t][0]++;
					if (age >= soldiers.fealty.age_of_conscription && age < soldiers.fealty.age_of_conscription + 1) counters[t][1]++;
					if (age >= soldiers.fealty.age_of_retirement && age < soldiers.fealty.age_of_retirement + 1) counters[t][2]++;
					if (age >= soldiers.fealty.life_expectancy) {
						counters[t][3]++;
						to_erase_parts[t].push_back(i);
						continue;
					}
					if (age >= soldiers.fealty.age_of_conscription && age < soldiers.fealty.age_of_retirement && soldiers.learning_rate > soldiers.fealty.conscription_minimum_learning_rate) counters[t][4]++;
					if (age >= soldiers.fealty.fertile_age_start && age < soldiers.fealty.fertile_age_end) counters[t][5]++;
					if (age >= soldiers.fealty.allowed_pregnancy_age_start && age < soldiers.fealty.allowed_pregnancy_age_end && soldiers.number_of_children < soldiers.fealty.allowed_number_of_children && soldiers.fealty.pregnancies_per_lifetime > soldiers.number_of_pregnancies) {
						counters[t][6]++; // babies
						counters[t][7]++; // pregnancies
						// record parent index to increment later on main thread
						pregnancy_parent_indices[t].push_back(i);
						new_soldiers_parts[t].push_back(soldiers.fealty);
						if ((static_cast<double>(rand()) / RAND_MAX) < soldiers.fealty.twinning_chance) {
							counters[t][6]++;
							pregnancy_parent_indices[t].push_back(i);
							new_soldiers_parts[t].push_back(soldiers.fealty);
						}
					}
				}
			});
		}
		for (auto &w : workers) w.join();

		// aggregate results and collect global lists
		std::vector<size_t> pregnancy_parents_all;
		for (int t = 0; t < worker_threads; ++t) {
			number_of_new_fertile_soldiers += counters[t][0];
			number_of_conscriptions_this_year += counters[t][1];
			number_of_retirements_this_year += counters[t][2];
			number_of_deaths_this_year += counters[t][3];
			number_of_military_aged_soldiers += counters[t][4];
			number_of_fertile_soldiers += counters[t][5];
			number_of_babies_this_year += counters[t][6];
			number_of_pregnancies_this_year += counters[t][7];
			// merge parts
			std::move(to_erase_parts[t].begin(), to_erase_parts[t].end(), std::back_inserter(to_erase));
			std::move(new_soldiers_parts[t].begin(), new_soldiers_parts[t].end(), std::back_inserter(new_soldiers));
			std::move(pregnancy_parent_indices[t].begin(), pregnancy_parent_indices[t].end(), std::back_inserter(pregnancy_parents_all));
		}

		// apply pregnancy/children increments for parents that are not dying
		if (!pregnancy_parents_all.empty()) {
			// mark deceased for quick lookup
			std::vector<char> dead_mark(n, 0);
			for (auto idx : to_erase) if (idx < n) dead_mark[idx] = 1;
			for (auto idx : pregnancy_parents_all) {
				if (idx < n && !dead_mark[idx]) {
					armies[idx].number_of_pregnancies++;
					armies[idx].number_of_children++;
				}
			}
		}
	}

	// Remove soldiers who died (from back to front to avoid index shifting)
		for (auto it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
		armies.erase(armies.begin() + *it);
	}
	// Add new soldiers
	for (const auto& pol : new_soldiers) {
		armies.emplace_back(pol);
	}

	/* print out the data for
		double number_of_babies_this_year = 0;
		double number_of_deaths_this_year = 0;
		double number_of_retirements_this_year = 0;
		double number_of_conscriptions_this_year = 0;
		double number_of_new_fertile_soldiers = 0;
		double number_of_military_aged_soldiers = 0;
		double number_of_pregnancies_this_year = 0;
		double number_of_fertile_soldiers = 0;
	*/
	std::cout << "Year " << simulation_age_years << " Summary:\n";
	std::cout << "New Fertile Soldiers: " << number_of_new_fertile_soldiers << "\n";
	std::cout << "Conscriptions This Year: " << number_of_conscriptions_this_year << "\n";
	std::cout << "Retirements This Year: " << number_of_retirements_this_year << "\n";
	std::cout << "Deaths This Year: " << number_of_deaths_this_year << "\n";
	std::cout << "Military Aged Soldiers: " << number_of_military_aged_soldiers << "\n";
	std::cout << "Fertile Soldiers: " << number_of_fertile_soldiers << "\n";
	std::cout << "Pregnancies This Year: " << number_of_pregnancies_this_year << "\n";
	std::cout << "Babies Born This Year: " << number_of_babies_this_year << "\n\n";

	std::cout << "Smartest soldier's learning rate: " << smartest_soldier_learning_rate() << "\n";
	std::cout << "Dumbest soldier's learning rate: " << dumbest_soldier_learning_rate() << "\n";
	std::cout << "Average maximum intelligence (learning rate) of Earth: " << calculate_average_learning_rate() << "\n\n";
	std::cout << "Average learning rate is expected to be minimum 120.0 to sustain a good technological civilization over long periods.\n\n";

	print_fealty_intelligence_distribution();
	print_most_intelligent_policy();
	print_alarm_if_mentally_retarded();

	std::vector<int> number_of_current_soldiers_in_policy(available_policies.size(), 0);
	for (auto z : armies) {
		for (size_t j = 0; j < available_policies.size(); ++j) {
			if (z.fealty.name == available_policies[j].name) {
				number_of_current_soldiers_in_policy[j]++;
			}
		}
	}
	// print out the number of soldiers in each policy
	for (size_t j = 0; j < available_policies.size(); ++j) {
		std::cout << "Number of soldiers in policy " << available_policies[j].name << ": " << number_of_current_soldiers_in_policy[j] << "\n";
	}

	// Check for random wars
	if (random_wars_enabled) {
		for (const auto& pol : available_policies) {
			if (!pol.pacifist) {
				if ((static_cast<double>(rand()) / RAND_MAX) < pol.chance_to_declare_war_per_year) {
					random_war_kill_most_losers_kill_some_winners();
					break; // Only one war per year
				}
			}
		}
	}
}

void random_genocide(policy pol, policy target)
{
	std::vector<size_t> to_erase;
	for (size_t i = 0; i < armies.size(); ++i) {
		auto& soldiers = armies[i];
		double age = soldiers.get_age_years();
		if (age >= pol.minimum_age_genocide && age <= pol.maximum_age_genocide && soldiers.fealty.name == target.name) {
			to_erase.push_back(i);
		}
	}
}

int main()
{
	setup();
	for (int i = 0; i < 1; i++) {
		armies.emplace_back(global_policy);
	}
	std::string command;
	help();
	while (1)
	{
		std::getline(std::cin, command);
		// Check for adding a new policy
		if (command.rfind("policy ", 0) == 0) {
			std::string policy_name = command.substr(7);
			policy new_policy;
			new_policy.name = policy_name;
			available_policies.push_back(new_policy);
			std::cout << "Added new policy: " << policy_name << "\n";
		}
		// allow random wars
		else if (command.rfind("random_war", 0) == 0) {
			if(!random_wars_enabled)
			std::cout << "Random wars enabled.\n";
			else std::cout << "Random wars disabled.\n";
			random_wars_enabled = !random_wars_enabled;
		}
		else if (command.rfind("threads ", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			int num_threads = 4;
			if (iss >> cmd >> num_threads) {
				threads = num_threads;
				std::cout << "Set number of threads to " << threads << ".\n";
			}
			else {
				std::cout << "Invalid threads syntax. Use: threads <number>\n";
			}
		}
		// Check for war command (uses last-token split: policy2 is last token, policy1 is the rest)
		else if (command.rfind("war ", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid war syntax. Use: war <policy1> <policy2>\n";
			}
			else {
				// policy2 is last token, policy1 is join of previous tokens
				std::string policy2_name = tokens.back();
				std::string policy1_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy1_name += " ";
					policy1_name += tokens[i];
				}

				policy* policy1 = nullptr;
				policy* policy2 = nullptr;
				for (auto& pol : available_policies) {
					if (pol.name == policy1_name) policy1 = &pol;
					if (pol.name == policy2_name) policy2 = &pol;
				}
				if (policy1_name == global_policy.name) policy1 = &global_policy;
				if (policy2_name == global_policy.name) policy2 = &global_policy;

				if (policy1 && policy2) {
					war_kill_most_losers_kill_some_winners(*policy1, *policy2);
				}
				else {
					std::cout << "One or both policies not found.\n";
				}
			}
		}
		// Check for adding soldiers
		else if (command.rfind("add ", 0) == 0) {
			// Parse using istringstream so we can accept policy names with spaces and avoid brittle index math.
			std::istringstream iss(command);
			std::string cmd;
			int number_to_add = 0;
			if (!(iss >> cmd >> number_to_add)) {
				std::cout << "Invalid add syntax. Use: add <number> <policy>\n";
			}
			else {
				std::string policy_name;
				std::getline(iss, policy_name);
				// trim leading space if present
				if (!policy_name.empty() && policy_name.front() == ' ') policy_name.erase(0, 1);

				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						for (int i = 0; i < number_to_add; ++i) {
							armies.emplace_back(pol);
						}
						std::cout << "Added " << number_to_add << " soldiers with fealty to " << policy_name << ".\n";
						found = true;
						break;
					}
				}
				if (!found) {
					std::cout << "Policy '" << policy_name << "' not found. Use 'policy <name>' to add it first.\n";
				}
			}
		}
		// Allow random wars
		// Add two standard policies
		else if (command == "STANDARD_WAR") {
			add_two_standard_policies();
			int usa_index;
			int enemy_index;
			for (int i = 0; i < available_policies.size(); i++) {
				if (available_policies[i].name == "USA") {
					usa_index = i;
				}
				if (available_policies[i].name == "ENEMY") {
					enemy_index = i;
				}
			}
			for (int j = 0; j < 100; j++) {
				armies.emplace_back(available_policies[usa_index]);
				armies.emplace_back(available_policies[enemy_index]);
			}
			// Simulate 50 years
			for (int year = 0; year < 50; year++) {
				add_year();
			}

			std::cout << "Simulated 50 years.\n";

			std::cout << "Added two standard policies: USA and ENEMY.\n";
			std::cout << "USA policy details:\n";
			std::cout << "  Conscription Age: " << available_policies[usa_index].age_of_conscription << "\n";
			std::cout << "  Retirement Age: " << available_policies[usa_index].age_of_retirement << "\n";
			std::cout << "  Life Expectancy: " << available_policies[usa_index].life_expectancy << "\n";
			std::cout << "  Age of Pregnancy Start: " << available_policies[usa_index].allowed_pregnancy_age_start << "\n\n";

			std::cout << "Added 100 soldiers for each policy.\n\n";

			std::cout << "ENEMY policy details:\n";
			std::cout << "  Conscription Age: " << available_policies[enemy_index].age_of_conscription << "\n";
			std::cout << "  Retirement Age: " << available_policies[enemy_index].age_of_retirement << "\n";
			std::cout << "  Life Expectancy: " << available_policies[enemy_index].life_expectancy << "\n";
			std::cout << "  Age of Pregnancy Start: " << available_policies[enemy_index].allowed_pregnancy_age_start << "\n\n";

			std::cout << "Now simulating war between USA and ENEMY after 50 years of reproduction...\n";
			std::cout << "The winner of the war is " << who_wins_conventional_war(available_policies[usa_index], available_policies[enemy_index]).name << ".\n";

		}
		// set sex age (policy may contain spaces; last token is age)
		else if (command.rfind("sex_age", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid sex_age syntax. Use: sex_age <policy> <age>\n";
			}
			else {
				std::string age_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int age = std::stoi(age_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.allowed_pregnancy_age_start = age;
						std::cout << "Set minimum pregnancy age for " << policy_name << " to start at " << age << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		else if (command.rfind("conscription_age", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid conscription_age syntax. Use: conscription_age <policy> <age>\n";
			}
			else {
				std::string age_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int age = std::stoi(age_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.age_of_conscription = age;
						std::cout << "Set conscription age for " << policy_name << " to " << age << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		// set retirement age
		else if (command.rfind("retirement_age", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid retirement_age syntax. Use: retirement_age <policy> <age>\n";
			}
			else {
				std::string age_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int age = std::stoi(age_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.age_of_retirement = age;
						std::cout << "Set retirement age for " << policy_name << " to " << age << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		// set intelligence (policy may contain spaces; last two tokens are median and range)
		else if (command.rfind("intelligence", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);
			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);
			if (tokens.size() < 3) {
				std::cout << "Invalid intelligence syntax. Use: intelligence <policy> <median> <range>\n";
			}
			else {
				std::string range_str = tokens.back();
				std::string median_str = tokens[tokens.size() - 2];
				std::string policy_name;
				for (size_t i = 0; i + 2 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				double median = std::stod(median_str);
				double range = std::stod(range_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.learning_rate_median = median;
						pol.learning_rate_range = range;
						std::cout << "Set intelligence for " << policy_name << " to median " << median << " and range " << range << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		//set war chance
		else if (command.rfind("warchoice", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);
			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);
			if (tokens.size() < 2) {
				std::cout << "Invalid warchoice syntax. Use: warchoice <policy> <chance>\n";
			}
			else {
				std::string chance_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				double chance = std::stod(chance_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.chance_to_declare_war_per_year = chance;
						std::cout << "Set war chance for " << policy_name << " to " << chance << "% per year.\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		// set life expectancy
		else if (command.rfind("life_expectancy", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid life_expectancy syntax. Use: life_expectancy <policy> <age>\n";
			}
			else {
				std::string age_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int age = std::stoi(age_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.life_expectancy = age;
						std::cout << "Set life expectancy for " << policy_name << " to " << age << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		// set fertility range (policy may contain spaces; last two tokens are start_age and end_age)
		else if (command.rfind("fertility_range", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 3) {
				std::cout << "Invalid fertility_range syntax. Use: fertility_range <policy> <start_age> <end_age>\n";
			}
			else {
				std::string end_age_str = tokens.back();
				std::string start_age_str = tokens[tokens.size() - 2];
				std::string policy_name;
				for (size_t i = 0; i + 2 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int start_age = std::stoi(start_age_str);
				int end_age = std::stoi(end_age_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.fertile_age_start = start_age;
						pol.fertile_age_end = end_age;
						std::cout << "Set fertility range for " << policy_name << " to " << start_age << " - " << end_age << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		// set pregnancies per lifetime
		else if (command.rfind("pregnancies_per_lifetime", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid pregnancies_per_lifetime syntax. Use: pregnancies_per_lifetime <policy> <number>\n";
			}
			else {
				std::string number_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int number = std::stoi(number_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.pregnancies_per_lifetime = number;
						std::cout << "Set pregnancies per lifetime for " << policy_name << " to " << number << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		// set allowed number of children
		else if (command.rfind("number_of_children", 0) == 0) {
			std::istringstream iss(command);
			std::string cmd;
			iss >> cmd;
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

			std::istringstream iss2(rest);
			std::vector<std::string> tokens;
			std::string tok;
			while (iss2 >> tok) tokens.push_back(tok);

			if (tokens.size() < 2) {
				std::cout << "Invalid number_of_children syntax. Use: number_of_children <policy> <number>\n";
			}
			else {
				std::string number_str = tokens.back();
				std::string policy_name;
				for (size_t i = 0; i + 1 < tokens.size(); ++i) {
					if (i) policy_name += " ";
					policy_name += tokens[i];
				}
				int number = std::stoi(number_str);
				bool found = false;
				for (auto& pol : available_policies) {
					if (pol.name == policy_name) {
						pol.allowed_number_of_children = number;
						std::cout << "Set allowed number of children for " << policy_name << " to " << number << ".\n";
						found = true;
						break;
					}
				}
				if (!found) std::cout << "Policy '" << policy_name << "' not found.\n";
			}
		}
		else if (command == "help") {
			help();
		}
		else {
			add_year();
			std::cout << "Advanced one year in the simulation. Current simulation age: " << simulation_age_years << " years.\n";
		}
	}
	return 0;
}