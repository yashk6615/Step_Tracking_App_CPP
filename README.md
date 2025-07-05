C++ Step Tracking Application
This is a console-based C++ application for tracking daily steps, setting goals, managing groups, and providing rewards and leaderboards. It uses a conceptual B+ tree implementation for efficient data management and persists data using CSV files.
Table of Contents
Features
Conceptual B+ Tree
File Structure
How to Compile and Run
CSV Data Files
Usage Examples
Features
The application provides the following core functionalities:
Individual Management:
Add Person: Add new individuals with unique IDs, name, age, daily step goal, and weekly step counts.
Delete Individual: Remove an individual from the system and automatically un-group them if they belong to a group.
Suggest Goal Update: Provides a heuristic-based suggestion for an individual's daily goal based on their recent performance.
Group Management:
Create Group: Form new groups with a unique Group ID, name, weekly group goal, and assign existing individuals (up to 5 members per group). Individuals cannot be in multiple groups.
Delete Group: Remove a group, making its members available to join other groups.
Merge Groups: Combine two existing groups into a new one, inheriting members and allowing new goals to be set. The original groups are deleted.
Check Group Achievement: Determine if a specific group has met its weekly step goal.
Display Group Range Info: Show details (members, goals, and ranks) for groups within a specified ID range.
Leaderboards & Rewards:
Get Top 3 Individuals: Display the top 3 individuals who have met their daily step goals and achieved the highest steps for the current day.
Generate Group Leaderboard: Rank and display groups based on their total weekly steps.
Check Individual Rewards: Award points (100 for Rank 1, 75 for Rank 2, 50 for Rank 3) to individuals who are in the top 3 daily goal achievers.
Conceptual B+ Tree
The application utilizes a ConceptualBPlusTree template class. This is an in-memory simulation of a B+ tree's core properties:
Sorted Data: It maintains data (Individuals or Groups) in a std::vector that is always kept sorted by a specified key (Individual ID or Group ID).
Efficient Operations: By leveraging std::lower_bound, std::sort, and std::remove_if, it provides efficient insert, search, remove, and getRange operations, mimicking the logarithmic time complexity characteristics of a true B+ tree for these operations on sorted data.
Genericity: The template allows it to store different data types and extract keys using a provided lambda function, making it reusable for both individuals and groups.
File Structure
The project consists of the following files:
.
├── main.cpp                # Main C++ source code for the application
├── individuals.csv         # Stores individual data (generated on first run)
└── groups.csv              # Stores group data (generated on first run)


How to Compile and Run
To compile and run this application, you will need a C++ compiler (like g++).
Save the Code:
Save the provided C++ code into a file named main.cpp.
Compile:
Open your terminal or command prompt, navigate to the directory where you saved main.cpp, and compile using g++:
g++ main.cpp -o step_tracker -std=c++17


g++: The C++ compiler.
main.cpp: Your source code file.
-o step_tracker: Specifies the output executable file name as step_tracker.
-std=c++17: Ensures C++17 features (like std::move and certain lambda captures) are enabled.
Run:
After successful compilation, run the executable:
./step_tracker

Upon first run, the application will automatically generate individuals.csv and groups.csv with sample data. Subsequent runs will load data from these files.
CSV Data Files
The application uses two CSV files for data persistence:
individuals.csv:
Header: ID,Name,Age,DailyStepGoal,WeeklyStepCount1,WeeklyStepCount2,WeeklyStepCount3,WeeklyStepCount4,WeeklyStepCount5,WeeklyStepCount6,WeeklyStepCount7
Example Row: 1,User1,20,5100,5300,5350,5400,5450,5500,5550,5600
groups.csv:
Header: GroupID,GroupName,MemberIDs,WeeklyGroupGoal
MemberIDs Format: Member IDs within this field are separated by semicolons (;).
Example Row: G1,Fitness Fanatics,1;2;3;4;5,35000
Usage Examples
The main function in main.cpp contains a series of test calls that demonstrate all the functionalities. When you run the step_tracker executable, you will see the output of these tests in your console, illustrating how each feature works.
// Example calls from main.cpp:

// Add a new person
app.add_person(21, "NewUser", 28, 5500, {5000, 5600, 5400, 5700, 5300, 5800, 5900});

// Create a new group
app.create_group("G6", "New Explorers", {16, 17}, 20000);

// Get top 3 individuals
app.get_top_3();

// Check group achievement
app.check_group_achievement("G1");

// Generate leaderboard
app.generate_leader_board();

// Check individual rewards
app.check_individual_rewards(3);

// Delete an individual
app.delete_individual(1);

// Delete a group
app.delete_group("G5");

// Merge two groups
app.merge_groups("G3", "G4", "Merged Titans", 50000);

// Display group range information
app.display_group_range_info("G1", "G6");

// Suggest goal update for an individual
app.suggest_goal_update(3);


Feel free to modify main.cpp to add your own test cases or integrate these functionalities into a more interactive command-line interface.
