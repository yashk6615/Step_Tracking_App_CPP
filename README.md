# C++ Step Tracking Application

This is a console-based C++ application for tracking daily steps, setting goals, managing groups, and providing rewards and leaderboards. It uses a conceptual B+ tree implementation for efficient data management and persists data using CSV files.

---

## 📑 Table of Contents

- [Features](#features)
- [Conceptual B+ Tree](#conceptual-b-tree)
- [File Structure](#file-structure)
- [How to Compile and Run](#how-to-compile-and-run)
- [CSV Data Files](#csv-data-files)
- [Usage Examples](#usage-examples)

---

## 🚀 Features

### Individual Management
- **Add Person**: Add new individuals with unique IDs, name, age, daily step goal, and weekly step counts.
- **Delete Individual**: Remove individuals and automatically ungroup them if they belong to a group.
- **Suggest Goal Update**: Suggest daily goal updates based on recent performance using a heuristic.

### Group Management
- **Create Group**: Create groups with unique Group ID, name, weekly group goal, and assign up to 5 members.
- **Delete Group**: Remove a group and free its members to join other groups.
- **Merge Groups**: Merge two existing groups into a new one with a new goal.
- **Check Group Achievement**: Check if a group has met its weekly goal.
- **Display Group Range Info**: Show details of groups within a specified ID range.

### Leaderboards & Rewards
- **Get Top 3 Individuals**: List top 3 individuals who have met daily goals and have the highest steps.
- **Generate Group Leaderboard**: Rank groups by total weekly steps.
- **Check Individual Rewards**: Reward top 3 individuals (100, 75, 50 points respectively).

---

## 🌳 Conceptual B+ Tree

The application uses a `ConceptualBPlusTree` template class to simulate B+ tree behavior:

- **Sorted Data**: Maintains sorted `std::vector` of individuals or groups using a key extractor lambda.
- **Efficient Operations**: Uses `std::lower_bound`, `std::sort`, and `std::remove_if` for fast insert, search, and delete.
- **Generic Design**: Can store any data type with a key extractor, usable for both individuals and groups.

---

## 📁 File Structure

.
├── main.cpp # Main C++ source code for the application
├── individuals.csv # Stores individual data (generated on first run)
└── groups.csv # Stores group data (generated on first run)

## 🛠 How to Compile and Run

### Requirements
- C++ compiler (like `g++`)

### Steps

1. **Save the Code**  
   Save your source code as `main.cpp`.

2. **Compile**  
   Open terminal and run:
   ```bash
   g++ main.cpp -o step_tracker -std=c++17
3. **Run**
     ./step_tracker

   📂 CSV Data Files
individuals.csv
Header:
ID,Name,Age,DailyStepGoal,WeeklyStepCount1,...,WeeklyStepCount7

Example:
1,User1,20,5100,5300,5350,5400,5450,5500,5550,5600

groups.csv
Header:
GroupID,GroupName,MemberIDs,WeeklyGroupGoal

MemberIDs Format:
IDs separated by semicolon ;

Example:
G1,Fitness Fanatics,1;2;3;4;5,35000

💡 Usage Examples
Example calls from main.cpp:
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
Feel free to add more test cases or build an interactive CLI!
