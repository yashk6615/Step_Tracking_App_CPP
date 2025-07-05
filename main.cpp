#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm> // For std::sort, std::remove, std::unique, std::lower_bound
#include <set>       // For unique member IDs in merge_groups
#include <map>       // For rewards_map
#include <functional> // For std::function

// Use the entire std namespace for brevity
using namespace std;

// --- Data Models ---

class Individual {
public:
    int id;
    string name;
    int age;
    int daily_step_goal;
    vector<int> weekly_step_count; // List of 7 integers
    string current_group_id; // ID of the group they belong to, or empty string if none
    int points; // Rewards points

    // Constructor to initialize an Individual object
    Individual(int id, string name, int age, int daily_step_goal, vector<int> weekly_step_count)
        : id(id), name(std::move(name)), age(age), daily_step_goal(daily_step_goal),
          weekly_step_count(std::move(weekly_step_count)), current_group_id(""), points(0) {}

    // Method to convert Individual object to a string for printing/debugging
    string toString() const {
        stringstream ss;
        ss << "Individual(ID=" << id << ", Name=" << name << ", Age=" << age
           << ", DailyGoal=" << daily_step_goal << ", WeeklySteps=[";
        for (size_t i = 0; i < weekly_step_count.size(); ++i) {
            ss << weekly_step_count[i] << (i == weekly_step_count.size() - 1 ? "" : ",");
        }
        ss << "], Group=" << (current_group_id.empty() ? "None" : current_group_id)
           << ", Points=" << points << ")";
        return ss.str();
    }
};

class Group {
public:
    string group_id; // This is the key for Group objects
    string group_name;
    vector<int> member_ids;
    int weekly_group_goal;
    long long total_weekly_steps; // Calculated dynamically or updated

    static const int MAX_MEMBERS = 5; // Maximum number of members allowed in a group

    // Constructor to initialize a Group object
    Group(string group_id, string group_name, vector<int> member_ids, int weekly_group_goal)
        : group_id(std::move(group_id)), group_name(std::move(group_name)),
          member_ids(std::move(member_ids)), weekly_group_goal(weekly_group_goal),
          total_weekly_steps(0) {
        // Ensure unique members on construction by sorting and removing duplicates
        sort(this->member_ids.begin(), this->member_ids.end());
        this->member_ids.erase(unique(this->member_ids.begin(), this->member_ids.end()), this->member_ids.end());
    }

    // Method to convert Group object to a string for printing/debugging
    string toString() const {
        stringstream ss;
        ss << "Group(ID=" << group_id << ", Name=" << group_name
           << ", Members=[";
        for (size_t i = 0; i < member_ids.size(); ++i) {
            ss << member_ids[i] << (i == member_ids.size() - 1 ? "" : ",");
        }
        ss << "], Goal=" << weekly_group_goal
           << ", TotalSteps=" << total_weekly_steps << ")";
        return ss.str();
    }
};

// --- B+ Tree Implementation (Conceptual using sorted vector) ---
// This class simulates the sorted nature and efficient operations of a B+ tree
// by keeping a vector of objects sorted by their key. It's a conceptual
// implementation for in-memory use, not a full disk-based B+ tree.

template <typename T, typename KeyType, typename Compare = less<KeyType>>
class ConceptualBPlusTree {
private:
    vector<T> data; // Stores the actual data objects
    Compare comp; // Comparator for key comparison
    // New: A function to extract the key from an object of type T
    function<KeyType(const T&)> key_extractor;

    // Helper function to find the correct insertion point using binary search
    typename vector<T>::iterator find_insert_pos(const KeyType& key) {
        return lower_bound(data.begin(), data.end(), key,
                                [this](const T& obj, const KeyType& k) { return comp(key_extractor(obj), k); });
    }

public:
    // Constructor now takes a key_extractor function and an optional comparator
    ConceptualBPlusTree(function<KeyType(const T&)> extractor, Compare c = Compare())
        : key_extractor(std::move(extractor)), comp(std::move(c)) {}

    // Inserts an item into the tree, maintaining sorted order by its key.
    void insert(const T& item) {
        KeyType item_key = key_extractor(item); // Extract key from the item
        auto it = find_insert_pos(item_key); // Find insertion position
        
        // Check if an item with the same key already exists at this position
        if (it != data.end() && !comp(item_key, key_extractor(*it)) && !comp(key_extractor(*it), item_key)) {
            // If item with this key already exists, we do not insert a duplicate.
            // Duplicate checks are handled externally before calling insert.
            return;
        }
        data.insert(it, item); // Insert the new item at the found position
    }

    // Removes an item from the tree by its key.
    bool remove(const KeyType& key) {
        // Use std::remove_if to logically remove the item (move it to the end)
        auto it = remove_if(data.begin(), data.end(),
                                  [&](const T& obj) { return key_extractor(obj) == key; });
        // If an item was found and moved, erase it from the vector
        if (it != data.end()) {
            data.erase(it, data.end());
            return true; // Item successfully removed
        }
        return false; // Item not found
    }

    // Searches for an item by its key and returns a pointer to it.
    // Returns nullptr if the item is not found.
    T* search(const KeyType& key) {
        // Find the potential position of the item using binary search
        auto it = find_insert_pos(key);
        // Check if the iterator is valid and if the item at that position matches the key
        if (it != data.end() && !comp(key, key_extractor(*it)) && !comp(key_extractor(*it), key)) {
            return &(*it); // Return a pointer to the found object
        }
        return nullptr; // Item not found
    }

    // Returns a non-constant reference to the underlying vector containing all items in sorted order.
    // This allows modification of the objects within the tree.
    vector<T>& getAllValues() {
        return data;
    }

    // Retrieves items within a specified key range (inclusive).
    vector<T> getRange(const KeyType& start_key, const KeyType& end_key) const {
        vector<T> results;
        // Find the first element that is not less than start_key
        auto it_start = lower_bound(data.begin(), data.end(), start_key,
                                         [this](const T& obj, const KeyType& k) { return comp(key_extractor(obj), k); });
        
        // Iterate from the starting point until the end of data or until an element exceeds end_key
        for (auto it = it_start; it != data.end(); ++it) {
            // Check if the current element's key is greater than end_key
            if (comp(end_key, key_extractor(*it)) && !comp(key_extractor(*it), end_key)) { // equivalent to it->key > end_key
                break; // We've gone past the end of our range
            }
            results.push_back(*it); // Add the element to the results
        }
        return results;
    }

    // Returns the number of items currently in the tree.
    size_t size() const {
        return data.size();
    }
};

// --- Step Tracking Application Logic ---

class StepTrackerApp {
private:
    // Two conceptual B+ trees, initialized with their respective key extractors
    ConceptualBPlusTree<Individual, int> individuals_tree{[](const Individual& ind){ return ind.id; }};
    ConceptualBPlusTree<Group, string> groups_tree{[](const Group& grp){ return grp.group_id; }};
    
    string individuals_file; // Name of the CSV file for individuals
    string groups_file;      // Name of the CSV file for groups

    // Helper function to split a string by a given delimiter
    vector<string> split(const string& s, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(s); // Create a string stream from the input string
        while (getline(tokenStream, token, delimiter)) { // Read tokens separated by delimiter
            tokens.push_back(token); // Add each token to the vector
        }
        return tokens;
    }

    // Loads individuals and groups data from the specified CSV files.
    void _load_data() {
        // Load Individuals from individuals.csv
        ifstream ind_file(individuals_file);
        if (!ind_file.is_open()) {
            cerr << "Warning: Individuals CSV file '" << individuals_file << "' not found. Starting with empty individual data." << endl;
        } else {
            string line;
            getline(ind_file, line); // Skip header line
            while (getline(ind_file, line)) { // Read each data line
                try {
                    vector<string> parts = split(line, ','); // Split line by comma
                    if (parts.size() < 5) { // Basic validation: ensure enough fields for core data
                        cerr << "Warning: Skipping malformed individual data line: '" << line << "'" << endl;
                        continue;
                    }

                    int id = stoi(parts[0]); // Convert string to int for ID
                    string name = parts[1];
                    int age = stoi(parts[2]);
                    int daily_goal = stoi(parts[3]);
                    vector<int> weekly_steps;
                    // Parse weekly step counts (remaining parts of the line)
                    for (size_t i = 4; i < parts.size(); ++i) {
                        weekly_steps.push_back(stoi(parts[i]));
                    }
                    // Insert the new Individual object into the individuals tree
                    individuals_tree.insert(Individual(id, name, age, daily_goal, weekly_steps));
                } catch (const exception& e) {
                    cerr << "Error parsing individual data line '" << line << "': " << e.what() << endl;
                }
            }
            ind_file.close(); // Close the file
        }

        // Load Groups from groups.csv
        ifstream grp_file(groups_file);
        if (!grp_file.is_open()) {
            cerr << "Warning: Groups CSV file '" << groups_file << "' not found. Starting with empty group data." << endl;
        } else {
            string line;
            getline(grp_file, line); // Skip header line
            while (getline(grp_file, line)) { // Read each data line
                try {
                    vector<string> parts = split(line, ','); // Split line by comma
                    if (parts.size() < 4) { // Basic validation
                        cerr << "Warning: Skipping malformed group data line: '" << line << "'" << endl;
                        continue;
                    }

                    string group_id = parts[0];
                    string group_name = parts[1];
                    vector<int> member_ids;
                    // Member IDs are semicolon-separated within a single CSV field
                    vector<string> member_id_strings = split(parts[2], ';');
                    for (const string& mid_str : member_id_strings) {
                        if (!mid_str.empty()) { // Ensure string is not empty before converting
                            member_ids.push_back(stoi(mid_str));
                        }
                    }
                    int weekly_group_goal = stoi(parts[3]);

                    // Insert the new Group object into the groups tree
                    groups_tree.insert(Group(group_id, group_name, member_ids, weekly_group_goal));

                    // Update individuals with their group_id after group is loaded
                    Group* group_ptr = groups_tree.search(group_id); // Get a pointer to the newly inserted group
                    if (group_ptr) {
                        for (int member_id : group_ptr->member_ids) {
                            Individual* individual = individuals_tree.search(member_id);
                            if (individual) {
                                individual->current_group_id = group_id; // Assign group ID to individual
                            }
                        }
                    }
                } catch (const exception& e) {
                    cerr << "Error parsing group data line '" << line << "': " << e.what() << endl;
                }
            }
            grp_file.close(); // Close the file
        }
        cout << "Loaded data. Individuals: " << individuals_tree.size() << ", Groups: " << groups_tree.size() << endl;
    }

    // Saves current individuals and groups data to the respective CSV files.
    void _save_data() {
        // Save Individuals to individuals.csv
        ofstream ind_file(individuals_file);
        if (!ind_file.is_open()) {
            cerr << "Error: Could not open individuals CSV file '" << individuals_file << "' for writing." << endl;
            return;
        }
        // Write header for individuals CSV
        ind_file << "ID,Name,Age,DailyStepGoal,WeeklyStepCount1,WeeklyStepCount2,WeeklyStepCount3,WeeklyStepCount4,WeeklyStepCount5,WeeklyStepCount6,WeeklyStepCount7\n";
        // Iterate through all individuals in the tree and write their data
        for (const auto& individual : individuals_tree.getAllValues()) {
            ind_file << individual.id << "," << individual.name << "," << individual.age << ","
                     << individual.daily_step_goal;
            for (int steps : individual.weekly_step_count) {
                ind_file << "," << steps; // Append each weekly step count
            }
            ind_file << "\n"; // New line for the next individual
        }
        ind_file.close();

        // Save Groups to groups.csv
        ofstream grp_file(groups_file);
        if (!grp_file.is_open()) {
            cerr << "Error: Could not open groups CSV file '" << groups_file << "' for writing." << endl;
            return;
        }
        // Write header for groups CSV
        grp_file << "GroupID,GroupName,MemberIDs,WeeklyGroupGoal\n";
        // Iterate through all groups in the tree and write their data
        for (const auto& group : groups_tree.getAllValues()) {
            grp_file << group.group_id << "," << group.group_name << ",";
            // Write member IDs, separated by semicolons
            for (size_t i = 0; i < group.member_ids.size(); ++i) {
                grp_file << group.member_ids[i] << (i == group.member_ids.size() - 1 ? "" : ";");
            }
            grp_file << "," << group.weekly_group_goal << "\n"; // Append group goal and new line
        }
        grp_file.close();
        cout << "Data saved to CSV files." << endl;
    }

public:
    // Constructor for StepTrackerApp, initializes file names and loads initial data
    StepTrackerApp(string ind_file = "individuals.csv", string grp_file = "groups.csv")
        : individuals_file(std::move(ind_file)), groups_file(std::move(grp_file)) {
        _load_data(); // Load data when the application is initialized
    }

    // Public getters for the trees (for testing in main)
    ConceptualBPlusTree<Individual, int>& get_individuals_tree() { return individuals_tree; }
    ConceptualBPlusTree<Group, string>& get_groups_tree() { return groups_tree; }


    // Adds a new individual to the tree of individuals. The tree remains sorted.
    bool add_person(int id, const string& name, int age, int daily_step_goal, const vector<int>& weekly_step_count) {
        if (individuals_tree.search(id) != nullptr) { // Check if individual with this ID already exists
            cout << "Error: Individual with ID " << id << " already exists." << endl;
            return false;
        }
        individuals_tree.insert(Individual(id, name, age, daily_step_goal, weekly_step_count)); // Insert new individual
        _save_data(); // Save changes to file
        cout << "Individual " << name << " (ID: " << id << ") added successfully." << endl;
        return true;
    }

    // Creates a new group and adds existing individuals to it.
    // An individual cannot be added to a new group if they already belong to one.
    // A group can contain a maximum of 5 individuals.
    bool create_group(const string& group_id, const string& group_name, const vector<int>& member_ids, int weekly_group_goal) {
        if (groups_tree.search(group_id) != nullptr) { // Check if group with this ID already exists
            cout << "Error: Group with ID " << group_id << " already exists." << endl;
            return false;
        }
        if (member_ids.size() > Group::MAX_MEMBERS) { // Check maximum members limit
            cout << "Error: A group cannot have more than " << Group::MAX_MEMBERS << " members." << endl;
            return false;
        }

        vector<int> actual_member_ids;
        for (int mid : member_ids) { // Iterate through proposed member IDs
            Individual* individual = individuals_tree.search(mid); // Find individual in the tree
            if (individual == nullptr) {
                cout << "Warning: Individual with ID " << mid << " not found. Skipping." << endl;
                continue;
            }
            if (!individual->current_group_id.empty()) { // Check if individual is already in a group
                cout << "Warning: Individual " << individual->name << " (ID: " << mid << ") already belongs to group " << individual->current_group_id << ". Skipping." << endl;
                continue;
            }
            actual_member_ids.push_back(mid); // Add valid member to the list
        }

        if (actual_member_ids.empty()) { // If no valid members could be added
            cout << "Error: No valid members to create the group." << endl;
            return false;
        }

        groups_tree.insert(Group(group_id, group_name, actual_member_ids, weekly_group_goal)); // Insert new group

        // Update individuals' current_group_id to reflect their new group membership
        for (int mid : actual_member_ids) {
            Individual* individual = individuals_tree.search(mid);
            if (individual) {
                individual->current_group_id = group_id;
            }
        }
        _save_data(); // Save changes to file
        cout << "Group '" << group_name << "' (ID: " << group_id << ") created successfully with members: ";
        for (int mid : actual_member_ids) cout << mid << " ";
        cout << "." << endl;
        return true;
    }

    // Displays the top 3 individuals who have completed their daily step goals and achieved the highest steps for the current day.
    // Individuals who have not completed daily goals are excluded.
    // Assumes the last element in weekly_step_count is today's steps.
    vector<Individual*> get_top_3() {
        vector<Individual*> eligible_individuals;
        // Iterate through all individuals to find those who met their daily goal
        for (auto& individual : individuals_tree.getAllValues()) { // Use non-const reference to get mutable objects
            if (individual.weekly_step_count.empty()) continue; // Skip if no step data
            int current_day_steps = individual.weekly_step_count.back(); // Get last day's steps
            if (current_day_steps >= individual.daily_step_goal) { // Check if daily goal is met
                eligible_individuals.push_back(&individual); // Add to eligible list (pointer to original object)
            }
        }

        // Sort eligible individuals by their current day's steps in descending order
        sort(eligible_individuals.begin(), eligible_individuals.end(),
                  [](const Individual* a, const Individual* b) {
                      return a->weekly_step_count.back() > b->weekly_step_count.back();
                  });

        cout << "\n--- Top 3 Individuals (Daily Goal Achievers) ---" << endl;
        if (eligible_individuals.empty()) {
            cout << "No individuals met their daily goal today." << endl;
            return {}; // Return empty vector if no one is eligible
        }

        vector<Individual*> top_3_result;
        // Display and collect the top 3 individuals
        for (size_t i = 0; i < min((size_t)3, eligible_individuals.size()); ++i) {
            cout << "Rank " << i + 1 << ": " << eligible_individuals[i]->name
                      << " (ID: " << eligible_individuals[i]->id << ") - Steps: "
                      << eligible_individuals[i]->weekly_step_count.back() << endl;
            top_3_result.push_back(eligible_individuals[i]);
        }
        return top_3_result;
    }

    // Displays whether the given group has completed its weekly group goal.
    // Calculates total weekly steps for the group by summing up members' steps.
    bool check_group_achievement(const string& group_id) {
        Group* group = groups_tree.search(group_id); // Find the group by ID
        if (group == nullptr) {
            cout << "Error: Group with ID " << group_id << " not found." << endl;
            return false;
        }

        long long total_group_steps = 0;
        // Sum up weekly steps for all members of the group
        for (int member_id : group->member_ids) {
            Individual* individual = individuals_tree.search(member_id);
            if (individual && !individual->weekly_step_count.empty()) {
                for (int steps : individual->weekly_step_count) {
                    total_group_steps += steps;
                }
            }
        }

        group->total_weekly_steps = total_group_steps; // Update the group object's total steps
        _save_data(); // Save updated total steps to file

        cout << "\n--- Group Achievement for '" << group->group_name << "' (ID: " << group_id << ") ---" << endl;
        cout << "Weekly Group Goal: " << group->weekly_group_goal << " steps" << endl;
        cout << "Total Steps Completed by Group: " << total_group_steps << " steps" << endl;

        if (total_group_steps >= group->weekly_group_goal) {
            cout << "Result: Congratulations! Group '" << group->group_name << "' has achieved its weekly goal!" << endl;
            return true;
        } else {
            cout << "Result: Group '" << group->group_name << "' has not yet achieved its weekly goal. "
                      << "Needs " << group->weekly_group_goal - total_group_steps << " more steps." << endl;
            return false;
        }
    }

    // Generates and displays a leaderboard for groups, sorted by total weekly steps (Descending).
    void generate_leader_board() {
        vector<pair<Group*, long long>> groups_with_steps;
        // Calculate total steps for each group
        for (auto& group : groups_tree.getAllValues()) { // Use non-const reference
            long long total_group_steps = 0;
            for (int member_id : group.member_ids) {
                Individual* individual = individuals_tree.search(member_id);
                if (individual && !individual->weekly_step_count.empty()) {
                    for (int steps : individual->weekly_step_count) {
                        total_group_steps += steps;
                    }
                }
            }
            // Explicitly construct std::pair
            groups_with_steps.push_back(make_pair(&group, total_group_steps)); 
        }

        // Sort groups by total steps in descending order
        sort(groups_with_steps.begin(), groups_with_steps.end(),
                  [](const auto& a, const auto& b) {
                      return a.second > b.second;
                  });

        cout << "\n--- Group Leaderboard ---" << endl;
        if (groups_with_steps.empty()) {
            cout << "No groups available to generate a leaderboard." << endl;
            return;
        }

        // Display the ranked groups
        for (size_t i = 0; i < groups_with_steps.size(); ++i) {
            cout << "Rank " << i + 1 << ": Group '" << groups_with_steps[i].first->group_name
                      << "' (ID: " << groups_with_steps[i].first->group_id << ") - Total Weekly Steps: "
                      << groups_with_steps[i].second << endl;
        }
    }

    // Displays the rewards earned by the given individual if they are in the top 3 daily goal achievers.
    // Awards points based on rank.
    void check_individual_rewards(int individual_id) {
        Individual* individual = individuals_tree.search(individual_id); // Find the individual
        if (individual == nullptr) {
            cout << "Error: Individual with ID " << individual_id << " not found." << endl;
            return;
        }

        vector<Individual*> top_3_individuals = get_top_3(); // Get the current top 3 daily achievers
        // Map rank index (0-based) to points earned
        map<int, int> rewards_map = {{0, 100}, {1, 75}, {2, 50}};

        int found_rank = -1;
        // Check if the individual is in the top 3 list
        for (size_t i = 0; i < top_3_individuals.size(); ++i) {
            if (top_3_individuals[i]->id == individual_id) {
                found_rank = i; // Store their rank index
                break;
            }
        }

        cout << "\n--- Rewards for " << individual->name << " (ID: " << individual_id << ") ---" << endl;
        if (found_rank != -1) {
            int points_earned = rewards_map[found_rank]; // Get points for their rank
            individual->points += points_earned; // Add points to individual's total
            cout << "Congratulations! You are Rank " << found_rank + 1 << " and earned " << points_earned << " points!" << endl;
            cout << "Total points: " << individual->points << endl;
            _save_data(); // Save updated points to file
        } else {
            cout << "This individual is not in the top 3 daily goal achievers today." << endl;
            cout << "Total points: " << individual->points << endl;
        }
    }

    // Deletes an individual from the individuals tree and removes them from any group they belong to.
    bool delete_individual(int individual_id) {
        Individual* individual = individuals_tree.search(individual_id); // Find the individual
        if (individual == nullptr) {
            cout << "Error: Individual with ID " << individual_id << " not found." << endl;
            return false;
        }

        // If the individual belongs to a group, remove them from that group's member list
        if (!individual->current_group_id.empty()) {
            Group* group = groups_tree.search(individual->current_group_id);
            if (group) {
                // Use std::remove to move the element to the end, then erase it
                auto it = remove(group->member_ids.begin(), group->member_ids.end(), individual_id);
                if (it != group->member_ids.end()) {
                    group->member_ids.erase(it, group->member_ids.end());
                    cout << "Individual " << individual->name << " removed from group " << group->group_name << "." << endl;
                }
            }
        }

        // Delete individual from the individuals tree
        if (individuals_tree.remove(individual_id)) {
            _save_data(); // Save changes to file
            cout << "Individual " << individual->name << " (ID: " << individual_id << ") deleted successfully." << endl;
            return true;
        } else {
            cout << "Failed to delete individual " << individual->name << " (ID: " << individual_id << ")." << endl;
            return false;
        }
    }

    // Deletes a group from the groups tree but retains its individuals, making them available for other groups.
    bool delete_group(const string& group_id) {
        Group* group = groups_tree.search(group_id); // Find the group
        if (group == nullptr) {
            cout << "Error: Group with ID " << group_id << " not found." << endl;
            return false;
        }

        // Set current_group_id to empty for all members of this group
        for (int member_id : group->member_ids) {
            Individual* individual = individuals_tree.search(member_id);
            if (individual) {
                individual->current_group_id = ""; // Un-group the individual
                cout << "Individual " << individual->name << " (ID: " << member_id << ") is now un-grouped." << endl;
            }
        }

        // Delete group from the groups tree
        if (groups_tree.remove(group_id)) {
            _save_data(); // Save changes to file
            cout << "Group '" << group->group_name << "' (ID: " << group_id << ") deleted successfully." << endl;
            return true;
        } else {
            cout << "Failed to delete group '" << group->group_name << "' (ID: " << group_id << ")." << endl;
            return false;
        }
    }

    // Creates a new group by merging two existing groups.
    // The original groups are deleted, and the new group uses group_ID_1 as its ID.
    bool merge_groups(const string& group_id_1, const string& group_id_2, const string& new_group_name, int new_weekly_goal) {
        Group* group1 = groups_tree.search(group_id_1);
        Group* group2 = groups_tree.search(group_id_2);

        if (group1 == nullptr) {
            cout << "Error: Group with ID " << group_id_1 << " not found." << endl;
            return false;
        }
        if (group2 == nullptr) {
            cout << "Error: Group with ID " << group_id_2 << " not found." << endl;
            return false;
        }

        // Combine members from both groups using a set to ensure uniqueness
        set<int> merged_member_set;
        for (int id : group1->member_ids) merged_member_set.insert(id);
        for (int id : group2->member_ids) merged_member_set.insert(id);

        vector<int> merged_member_ids(merged_member_set.begin(), merged_member_set.end());

        if (merged_member_ids.size() > Group::MAX_MEMBERS) { // Check if merged group exceeds max members
            cout << "Error: Merging these groups would exceed the maximum of " << Group::MAX_MEMBERS << " members. "
                      << "Please remove members from one of the groups before merging." << endl;
            return false;
        }

        // Delete original groups first. This also un-groups their members.
        if (!delete_group(group_id_1)) {
            cout << "Error: Could not delete original group " << group_id_1 << " during merge." << endl;
            return false;
        }
        if (!delete_group(group_id_2)) {
            cout << "Error: Could not delete original group " << group_id_2 << " during merge." << endl;
            // In a real application, you might implement a more robust rollback here if deletion fails.
            return false;
        }

        // Create the new merged group with group_id_1 as the new ID
        groups_tree.insert(Group(group_id_1, new_group_name, merged_member_ids, new_weekly_goal));

        // Update current_group_id for all members of the new merged group
        for (int mid : merged_member_ids) {
            Individual* individual = individuals_tree.search(mid);
            if (individual) {
                individual->current_group_id = group_id_1;
            }
        }
        _save_data(); // Save changes to file
        cout << "Groups '" << group1->group_name << "' and '" << group2->group_name
                  << "' merged into new group '" << new_group_name << "' (ID: " << group_id_1 << ")." << endl;
        return true;
    }

    // Displays information about members in the range of given group IDs, including group goals and ranks.
    void display_group_range_info(const string& start_group_id, const string& end_group_id) {
        cout << "\n--- Group Information in Range: " << start_group_id << " to " << end_group_id << " ---" << endl;
        
        // Get a copy of all groups from the tree
        vector<Group> all_groups = groups_tree.getAllValues(); 
        
        vector<Group*> relevant_groups_ptrs;
        // Filter groups that fall within the specified ID range
        for (auto& group : all_groups) { // Iterate over reference to modify total_weekly_steps
            // Assuming string comparison for group IDs works for range (e.g., G1, G2, G10)
            if (group.group_id >= start_group_id && group.group_id <= end_group_id) {
                relevant_groups_ptrs.push_back(&group); // Store pointer to the group in the copy
            }
        }
        
        // Sort the relevant groups by their Group ID for consistent display
        sort(relevant_groups_ptrs.begin(), relevant_groups_ptrs.end(),
                  [](const Group* a, const Group* b) {
                      return a->group_id < b->group_id;
                  });

        if (relevant_groups_ptrs.empty()) {
            cout << "No groups found in the specified range." << endl;
            return;
        }

        // Recalculate total steps for ranking within this specific range
        vector<pair<Group*, long long>> groups_with_steps;
        for (Group* group : relevant_groups_ptrs) {
            long long total_group_steps = 0;
            for (int member_id : group->member_ids) {
                Individual* individual = individuals_tree.search(member_id);
                if (individual && !individual->weekly_step_count.empty()) {
                    for (int steps : individual->weekly_step_count) {
                        total_group_steps += steps;
                    }
                }
            }
            group->total_weekly_steps = total_group_steps; // Update the group object's total steps
            groups_with_steps.push_back(make_pair(group, total_group_steps));
        }
        
        // Sort for ranking within the displayed range (highest steps first)
        sort(groups_with_steps.begin(), groups_with_steps.end(),
                  [](const auto& a, const auto& b) {
                      return a.second > b.second;
                  });

        // Display information for each group in the range, along with its rank
        for (size_t i = 0; i < groups_with_steps.size(); ++i) {
            Group* group = groups_with_steps[i].first;
            long long steps = groups_with_steps[i].second;
            cout << "\nRank " << i + 1 << " in Range:" << endl;
            cout << "  Group ID: " << group->group_id << endl;
            cout << "  Group Name: " << group->group_name << endl;
            cout << "  Weekly Group Goal: " << group->weekly_group_goal << endl;
            cout << "  Total Weekly Steps: " << steps << endl;
            
            vector<string> member_names;
            for (int member_id : group->member_ids) {
                Individual* individual = individuals_tree.search(member_id);
                if (individual) {
                    member_names.push_back(individual->name + " (ID: " + to_string(individual->id) + ")");
                }
            }
            cout << "  Members: ";
            if (member_names.empty()) {
                cout << "None" << endl;
            } else {
                for (size_t j = 0; j < member_names.size(); ++j) {
                    cout << member_names[j] << (j == member_names.size() - 1 ? "" : ", ");
                }
                cout << endl;
            }
        }
    }

    // Suggests a daily goal update for an individual based on their recent performance.
    // The suggestion aims to help them consistently appear in the top 3.
    void suggest_goal_update(int individual_id) {
        Individual* individual = individuals_tree.search(individual_id); // Find the individual
        if (individual == nullptr) {
            cout << "Error: Individual with ID " << individual_id << " not found." << endl;
            return;
        }

        cout << "\n--- Goal Suggestion for " << individual->name << " (ID: " << individual_id << ") ---" << endl;
        if (individual->weekly_step_count.size() < 7) { // Check for sufficient data
            cout << "Not enough weekly data to provide a meaningful suggestion (need 7 days)." << endl;
            cout << "Current Daily Goal: " << individual->daily_step_goal << endl;
            return;
        }

        // Analyze last 7 days performance
        int achieved_days = 0;
        long long total_steps_last_7_days = 0;
        for (int steps : individual->weekly_step_count) {
            if (steps >= individual->daily_step_goal) {
                achieved_days++; // Count days where goal was achieved
            }
            total_steps_last_7_days += steps; // Sum total steps
        }
        int total_days = individual->weekly_step_count.size();
        double current_daily_avg = static_cast<double>(total_steps_last_7_days) / total_days;

        string suggestion_msg = "Current Daily Goal: " + to_string(individual->daily_step_goal) + "\n";
        int new_goal = individual->daily_step_goal; // Initialize new goal with current goal

        if (achieved_days >= 6) { // If consistently achieving (6 or 7 days)
            if (current_daily_avg > individual->daily_step_goal * 1.2) { // If significantly exceeding goal
                new_goal = static_cast<int>(individual->daily_step_goal * 1.1); // Increase by 10%
                suggestion_msg += "You consistently achieve your daily goal and often exceed it! "
                                  "Consider increasing your daily goal to " + to_string(new_goal) + " steps to challenge yourself further.";
            } else {
                suggestion_msg += "You consistently achieve your daily goal. Keep up the great work! "
                                  "Current goal of " + to_string(individual->daily_step_goal) + " steps seems appropriate.";
            }
        } else if (achieved_days <= 2) { // If consistently missing (0, 1, or 2 days achieved)
            if (current_daily_avg < individual->daily_step_goal * 0.8) { // If significantly missing goal
                new_goal = static_cast<int>(individual->daily_step_goal * 0.9); // Decrease by 10%
                suggestion_msg += "You are consistently missing your daily goal. "
                                  "Consider lowering your daily goal to " + to_string(new_goal) + " steps to build consistency and confidence.";
            } else {
                suggestion_msg += "You sometimes miss your daily goal. "
                                  "Review your activity patterns. Current goal of " + to_string(individual->daily_step_goal) + " steps "
                                  "might be achievable with slight adjustments.";
            }
        } else { // Mixed performance (3, 4, or 5 days achieved)
            suggestion_msg += "Your performance is mixed. Current goal of " + to_string(individual->daily_step_goal) + " steps "
                              "is a good target. Focus on consistency.";
        }

        cout << suggestion_msg << endl;
        if (new_goal != individual->daily_step_goal) {
            cout << "Suggested New Daily Goal: " << new_goal << endl;
            // Optionally, uncomment the lines below to automatically update the goal and save data:
            // individual->daily_step_goal = new_goal;
            // _save_data();
        }
    }
};

// --- Data File Generation (CSV) ---

// Generates sample data and writes it to two CSV files: individuals.csv and groups.csv.
void generate_sample_data_csv(const string& individuals_file, const string& groups_file) {
    // Generate individuals.csv
    ofstream ind_csv(individuals_file);
    if (!ind_csv.is_open()) {
        cerr << "Error: Could not create individuals CSV file: " << individuals_file << endl;
        return;
    }
    // Write header for individuals CSV
    ind_csv << "ID,Name,Age,DailyStepGoal,WeeklyStepCount1,WeeklyStepCount2,WeeklyStepCount3,WeeklyStepCount4,WeeklyStepCount5,WeeklyStepCount6,WeeklyStepCount7\n";
    for (int i = 1; i <= 20; ++i) { // Generate data for 20 individuals
        int id = i;
        string name = "User" + to_string(i);
        int age = 20 + (i % 30);
        int daily_goal = 5000 + (i * 100);
        vector<int> weekly_steps;
        for (int j = 0; j < 7; ++j) { // Generate 7 days of step data
            int steps = daily_goal - 500 + (j * 100);
            if (j % 2 == 0) steps = daily_goal + 200 + (j * 50); // Some higher steps
            if (i % 3 == 0) steps = daily_goal + 100 + (j * 50); // Make every 3rd user consistently achieve
            else if (i % 5 == 0) steps = daily_goal - 1000 + (j * 50); // Make every 5th user consistently miss
            weekly_steps.push_back(steps);
        }
        
        // Write individual data to CSV
        ind_csv << id << "," << name << "," << age << "," << daily_goal;
        for (int steps : weekly_steps) {
            ind_csv << "," << steps;
        }
        ind_csv << "\n";
    }
    ind_csv.close();
    cout << "Generated sample individuals CSV: '" << individuals_file << "'" << endl;

    // Generate groups.csv
    ofstream grp_csv(groups_file);
    if (!grp_csv.is_open()) {
        cerr << "Error: Could not create groups CSV file: " << groups_file << endl;
        return;
    }
    // Write header for groups CSV
    grp_csv << "GroupID,GroupName,MemberIDs,WeeklyGroupGoal\n";
    // Define and write sample group data
    grp_csv << "G1,Fitness Fanatics,1;2;3;4;5,35000\n";
    grp_csv << "G2,Step Squad,6;7;8;9,30000\n";
    grp_csv << "G3,Trail Blazers,10;11;12,25000\n";
    grp_csv << "G4,Pace Setters,13;14,20000\n";
    grp_csv << "G5,Solo Stars,15,10000\n";
    // Individuals 16-20 are initially un-grouped
    grp_csv.close();
    cout << "Generated sample groups CSV: '" << groups_file << "'" << endl;
}

// --- Main Application Execution ---

int main() {
    // Define the names for the CSV files
    string individuals_csv_file = "individuals.csv";
    string groups_csv_file = "groups.csv";
    
    // Generate the sample data CSV files
    generate_sample_data_csv(individuals_csv_file, groups_csv_file);

    // Create an instance of the StepTrackerApp, which will load data from the CSVs
    StepTrackerApp app(individuals_csv_file, groups_csv_file);

    cout << "\n--- Initial State ---" << endl;
    cout << "Individuals in tree: " << app.get_individuals_tree().size() << endl;
    cout << "Groups in tree: " << app.get_groups_tree().size() << endl;

    // --- Test Functionalities ---

    cout << "\n--- Testing Add_Person ---" << endl;
    app.add_person(21, "NewUser", 28, 5500, {5000, 5600, 5400, 5700, 5300, 5800, 5900});
    app.add_person(22, "AnotherUser", 35, 6000, {5500, 5800, 5900, 5700, 5600, 5900, 6100});
    app.add_person(21, "DuplicateUser", 20, 4000, {100,200,300,400,500,600,700}); // This should fail as ID 21 already exists

    cout << "\n--- Testing Create_group ---" << endl;
    // Try creating a group with existing un-grouped members (16 and 17 are initially un-grouped)
    app.create_group("G6", "New Explorers", {16, 17}, 20000);
    // Try creating a group with an already grouped member (User 1 is in G1)
    app.create_group("G7", "Mixed Group", {1, 18}, 15000); // User 1 should be skipped
    // Try creating a group with too many members (should fail due to MAX_MEMBERS = 5)
    app.create_group("G8", "Too Many", {19, 20, 21, 22, 1, 2}, 40000); // Users 1,2 are grouped, too many overall

    cout << "\n--- Testing Get_top_3 ---" << endl;
    app.get_top_3();

    cout << "\n--- Testing Check_group_achievement ---" << endl;
    app.check_group_achievement("G1");
    app.check_group_achievement("G5"); // G5 has only one member (User 15), likely not achieved

    cout << "\n--- Testing Generate_leader_board ---" << endl;
    app.generate_leader_board();

    cout << "\n--- Testing Check_individual_rewards ---" << endl;
    // Test with users likely to be in top 3 (based on data generation logic)
    app.check_individual_rewards(3);
    app.check_individual_rewards(6);
    app.check_individual_rewards(15); // User 15 might not be in top 3

    cout << "\n--- Testing Delete_individual ---" << endl;
    app.delete_individual(1); // Delete User 1 (who is in G1)
    // Verify User 1 is no longer found and G1's members list is updated
    Individual* individual_1 = app.get_individuals_tree().search(1);
    Group* group_1 = app.get_groups_tree().search("G1");
    cout << "User 1 after deletion: " << (individual_1 ? individual_1->toString() : "Not found") << endl;
    cout << "Group G1 members after User 1 deletion: ";
    if (group_1) {
        for (int mid : group_1->member_ids) cout << mid << " ";
        cout << endl;
    } else {
        cout << "Group G1 not found" << endl;
    }

    cout << "\n--- Testing Delete_group ---" << endl;
    app.delete_group("G5"); // Delete Solo Stars group
    // Verify User 15 (its only member) is now un-grouped
    Individual* individual_15 = app.get_individuals_tree().search(15);
    cout << "User 15 after G5 deletion: " << (individual_15 ? individual_15->toString() : "Not found") << endl;

    cout << "\n--- Testing Merge_groups ---" << endl;
    // Merge G3 (members 10,11,12) and G4 (members 13,14) into a new group named "Merged Titans" with ID "G3"
    app.merge_groups("G3", "G4", "Merged Titans", 50000);
    // Verify G3 is updated with merged members, G4 is deleted, and members' group_id updated
    Group* group_3_new = app.get_groups_tree().search("G3");
    Group* group_4_old = app.get_groups_tree().search("G4");
    cout << "New G3 after merge: " << (group_3_new ? group_3_new->toString() : "Not found") << endl;
    cout << "Old G4 after merge: " << (group_4_old ? group_4_old->toString() : "Not found") << endl;
    Individual* individual_10 = app.get_individuals_tree().search(10);
    Individual* individual_13 = app.get_individuals_tree().search(13);
    cout << "User 10 group_id after merge: " << (individual_10 ? individual_10->current_group_id : "User 10 not found") << endl;
    cout << "User 13 group_id after merge: " << (individual_13 ? individual_13->current_group_id : "User 13 not found") << endl;


    cout << "\n--- Testing Display_group_range_info ---" << endl;
    // Display info for groups with IDs between "G1" and "G6" (inclusive)
    app.display_group_range_info("G1", "G6"); 

    cout << "\n--- Testing Suggest_goal_update ---" << endl;
    app.suggest_goal_update(3); // User 3 is likely a high achiever
    app.suggest_goal_update(19); // User 19 is un-grouped, likely mixed performance
    app.suggest_goal_update(10); // User 10 is now in Merged Titans, check its performance
    app.suggest_goal_update(100); // Non-existent user (should show error)

    cout << "\n--- Final State ---" << endl;
    cout << "Individuals in tree: " << app.get_individuals_tree().size() << endl;
    cout << "Groups in tree: " << app.get_groups_tree().size() << endl;
    // Uncomment the following loops to print the full details of all individuals and groups in their final state:
    // for (const auto& ind : app.get_individuals_tree().getAllValues()) {
    //     cout << ind.toString() << endl;
    // }
    // for (const auto& grp : app.get_groups_tree().getAllValues()) {
    //     cout << grp.toString() << endl;
    // }

    return 0;
}
