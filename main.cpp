#include <vector>
#include <chrono>
#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>

double simulation_age_years;
double simulation_birthtime;


class policy {
public:
	policy() {}
	std::string name = "Global Policy";
	double age_of_conscription = 13;
	double age_of_retirement = 35;
	double life_expectancy = 65;
	double fertile_age_start = 10;
	double fertile_age_end = 50;
	double allowed_pregnancy_age_start = 10;
	double allowed_pregnancy_age_end = 50;
	double allowed_number_of_children = 300;
	double pregnancies_per_lifetime = 30;
	double years_between_pregnancies = 1;
	double twinning_chance = 0.1;
	bool pacifist = false;
};

std::vector<policy> available_policies;
policy global_policy;
policy your_policy;
policy enemy_policy;

void setup()
{
	available_policies.push_back(global_policy);
	simulation_age_years = 0;
	simulation_birthtime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}

class soldier {
	double birthtime;
public:
	double number_of_children = 0;
	double number_of_pregnancies = 0;
	policy fealty;
	soldier(policy conscribed_policy) {
		fealty = conscribed_policy;
		// Fix: Use simulation time, not wall-clock time
		birthtime = simulation_age_years * 31557600 + simulation_birthtime;
	}
	double get_age_years()
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
		if (age >= soldiers.fealty.age_of_conscription && age < soldiers.fealty.age_of_retirement) {
			number_of_military_aged_soldiers++;
		}
		if (age >= soldiers.fealty.fertile_age_start && age < soldiers.fealty.fertile_age_end) {
			number_of_fertile_soldiers++;
		}
		if (age >= soldiers.fealty.allowed_pregnancy_age_start && age < soldiers.fealty.allowed_pregnancy_age_end && soldiers.number_of_children < soldiers.fealty.allowed_number_of_children && soldiers.fealty.pregnancies_per_lifetime > soldiers.number_of_pregnancies) {
			number_of_babies_this_year++;
			number_of_pregnancies_this_year++;
			armies[i].number_of_pregnancies++;
			new_soldiers.push_back(soldiers.fealty);
			if ((static_cast<double>(rand()) / RAND_MAX) < soldiers.fealty.twinning_chance) {
				number_of_babies_this_year++;
				new_soldiers.push_back(soldiers.fealty);
				armies[i].number_of_children++;
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
	std::cout << "Babies Born This Year: " << number_of_babies_this_year << "\n";

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
}

void help()
{
	std::cout << "Type commands to interact with the simulation:\n";
	std::cout << "Type 'help' to see this message again.\n";
	std::cout << "Type STANDARD_WAR to advance 50 years with USA and ENEMY policies and simulate a war between them.\n";
	std::cout << "Press Enter to advance one year in the simulation, \n";
	std::cout << "type policy <name> to add a new policy type, \n";
	std::cout << "type add <number> <policy> to add soldiers with fealty sworn to a specific policy, \n";
	std::cout << "type sex_age <policy> <age> to set the reproductive age for a policy, \n";
	std::cout << "type conscription_age <policy> <age> to set the conscription age for a policy, \n";
	std::cout << "type retirement_age <policy> <age> to set the retirement age for a policy, \n";
	std::cout << "type life_expectancy <policy> <age> to set the life expectancy for a policy, \n";
	std::cout << "type fertility_range <policy> <start_age> <end_age> to set the fertility range for a policy, \n";
	std::cout << "type pregnancies_per_lifetime <policy> <number> to set the number of pregnancies per lifetime for a policy, \n";
	std::cout << "type number_of_children <policy> <number> to set the allowed number of children per soldier for a policy, \n";
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
	available_policies.push_back(enemy_policy);
}

int main()
{
	setup();
	for (int i = 0; i < 10; i++) {
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
					policy winner = who_wins_conventional_war(*policy1, *policy2);
					std::cout << "The winner of the war between " << policy1_name << " and " << policy2_name << " is " << winner.name << ".\n";
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
		// Help command
		else if (command == "help") {
			help();
		}
		// Advance one year
		else {
			add_year();
			std::cout << "Advanced one year in the simulation. Current simulation age: " << simulation_age_years << " years.\n";
		}
	}
	return 0;
}