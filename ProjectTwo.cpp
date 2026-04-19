//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Esther Castaneda
// Description : ABCU Advising Assistance Program
//============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

// Course information stored in the binary search tree.
struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};

// Internal node for the binary search tree.
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(Course course) {
        this->course = course;
        left = nullptr;
        right = nullptr;
    }
};

class BinarySearchTree {
private:
    Node* root;
    int courseCount;

    void addNode(Node* node, Course course);
    void inOrder(Node* node) const;
    void deleteTree(Node* node);

public:
    BinarySearchTree();
    ~BinarySearchTree();
    void Clear();
    void Insert(Course course);
    Course* Search(string courseNumber) const;
    void PrintCourseList() const;
    bool IsEmpty() const;
    int Size() const;
};

BinarySearchTree::BinarySearchTree() {
    root = nullptr;
    courseCount = 0;
}

BinarySearchTree::~BinarySearchTree() {
    deleteTree(root);
}

void BinarySearchTree::Clear() {
    deleteTree(root);
    root = nullptr;
    courseCount = 0;
}

void BinarySearchTree::Insert(Course course) {
    if (root == nullptr) {
        root = new Node(course);
    }
    else {
        addNode(root, course);
    }

    ++courseCount;
}

Course* BinarySearchTree::Search(string courseNumber) const {
    Node* current = root;

    while (current != nullptr) {
        if (current->course.courseNumber == courseNumber) {
            return &current->course;
        }
        else if (courseNumber < current->course.courseNumber) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }

    return nullptr;
}

void BinarySearchTree::PrintCourseList() const {
    inOrder(root);
}

bool BinarySearchTree::IsEmpty() const {
    return courseCount == 0;
}

int BinarySearchTree::Size() const {
    return courseCount;
}

void BinarySearchTree::addNode(Node* node, Course course) {
    if (course.courseNumber < node->course.courseNumber) {
        if (node->left == nullptr) {
            node->left = new Node(course);
        }
        else {
            addNode(node->left, course);
        }
    }
    else if (course.courseNumber > node->course.courseNumber) {
        if (node->right == nullptr) {
            node->right = new Node(course);
        }
        else {
            addNode(node->right, course);
        }
    }
    else {
        // Replace duplicate course numbers with the latest record loaded.
        node->course = course;
        --courseCount;
    }
}

void BinarySearchTree::inOrder(Node* node) const {
    if (node != nullptr) {
        inOrder(node->left);
        cout << node->course.courseNumber << ", " << node->course.title << endl;
        inOrder(node->right);
    }
}

void BinarySearchTree::deleteTree(Node* node) {
    if (node != nullptr) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

// Trim spaces and carriage returns around file values and user input.
string Trim(string value) {
    size_t start = 0;
    while (start < value.length() && isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.length();
    while (end > start && isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

string ToUpperCase(string value) {
    for (char& letter : value) {
        letter = static_cast<char>(toupper(static_cast<unsigned char>(letter)));
    }

    return value;
}

string RemoveSurroundingQuotes(string value) {
    value = Trim(value);

    if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.length() - 2);
    }

    return value;
}

// Split one CSV line. This handles quoted fields in case a title contains a comma.
vector<string> SplitCsvLine(string line) {
    vector<string> tokens;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char currentChar = line[i];

        if (currentChar == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                current += '"';
                ++i;
            }
            else {
                inQuotes = !inQuotes;
            }
        }
        else if (currentChar == ',' && !inQuotes) {
            tokens.push_back(Trim(current));
            current.clear();
        }
        else {
            current += currentChar;
        }
    }

    tokens.push_back(Trim(current));
    return tokens;
}

bool BuildCourse(vector<string> tokens, Course& course) {
    if (tokens.size() < 2 || tokens[0].empty() || tokens[1].empty()) {
        return false;
    }

    course.courseNumber = ToUpperCase(tokens[0]);
    course.title = tokens[1];
    course.prerequisites.clear();

    for (size_t i = 2; i < tokens.size(); ++i) {
        if (!tokens[i].empty()) {
            course.prerequisites.push_back(ToUpperCase(tokens[i]));
        }
    }

    return true;
}

bool LoadCourses(string fileName, BinarySearchTree& courses) {
    ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        cout << "Error: Unable to open file " << fileName << "." << endl;
        return false;
    }

    vector<Course> parsedCourses;
    unordered_set<string> courseNumbers;
    string line;
    int lineNumber = 0;
    bool validFile = true;

    while (getline(inputFile, line)) {
        ++lineNumber;

        if (Trim(line).empty()) {
            continue;
        }

        Course course;
        vector<string> tokens = SplitCsvLine(line);

        if (!BuildCourse(tokens, course)) {
            cout << "Error: Line " << lineNumber
                 << " must include a course number and course title." << endl;
            validFile = false;
            continue;
        }

        if (courseNumbers.find(course.courseNumber) != courseNumbers.end()) {
            cout << "Error: Duplicate course number " << course.courseNumber
                 << " found on line " << lineNumber << "." << endl;
            validFile = false;
            continue;
        }

        courseNumbers.insert(course.courseNumber);
        parsedCourses.push_back(course);
    }

    inputFile.close();

    for (const Course& course : parsedCourses) {
        for (const string& prerequisite : course.prerequisites) {
            if (courseNumbers.find(prerequisite) == courseNumbers.end()) {
                cout << "Error: Prerequisite " << prerequisite << " for "
                     << course.courseNumber << " does not exist in the file." << endl;
                validFile = false;
            }
        }
    }

    if (!validFile) {
        cout << "The course data was not loaded because the file contains errors." << endl;
        return false;
    }

    courses.Clear();
    for (const Course& course : parsedCourses) {
        courses.Insert(course);
    }

    cout << courses.Size() << " courses loaded successfully." << endl;
    return true;
}

void PrintCourseInformation(const BinarySearchTree& courses, string courseNumber) {
    Course* course = courses.Search(ToUpperCase(Trim(courseNumber)));

    if (course == nullptr) {
        cout << "Course not found." << endl;
        return;
    }

    cout << course->courseNumber << ", " << course->title << endl;
    cout << "Prerequisites: ";

    if (course->prerequisites.empty()) {
        cout << "None" << endl;
        return;
    }

    for (size_t i = 0; i < course->prerequisites.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }

        cout << course->prerequisites[i];
    }

    cout << endl;
    cout << "Prerequisite Course Titles:" << endl;

    for (const string& prerequisiteNumber : course->prerequisites) {
        Course* prerequisite = courses.Search(prerequisiteNumber);

        if (prerequisite != nullptr) {
            cout << prerequisite->courseNumber << ", " << prerequisite->title << endl;
        }
        else {
            cout << prerequisiteNumber << ", title not found" << endl;
        }
    }
}

int ReadMenuChoice() {
    string input;
    int choice = 0;

    cout << endl;
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
    cout << "What would you like to do? ";

    getline(cin, input);
    stringstream inputStream(input);

    if (!(inputStream >> choice)) {
        return -1;
    }

    return choice;
}

int main() {
    BinarySearchTree courses;
    bool dataLoaded = false;
    int choice = 0;

    cout << "Welcome to the course planner." << endl;

    while (choice != 9) {
        choice = ReadMenuChoice();

        switch (choice) {
        case 1: {
            string fileName;
            cout << "Enter the course data file name: ";
            getline(cin, fileName);
            dataLoaded = LoadCourses(RemoveSurroundingQuotes(fileName), courses);
            break;
        }
        case 2:
            if (!dataLoaded || courses.IsEmpty()) {
                cout << "Please load the course data first." << endl;
            }
            else {
                cout << "Here is a sample schedule:" << endl;
                courses.PrintCourseList();
            }
            break;
        case 3:
            if (!dataLoaded || courses.IsEmpty()) {
                cout << "Please load the course data first." << endl;
            }
            else {
                string courseNumber;
                cout << "What course do you want to know about? ";
                getline(cin, courseNumber);
                PrintCourseInformation(courses, courseNumber);
            }
            break;
        case 9:
            cout << "Thank you for using the course planner!" << endl;
            break;
        default:
            cout << choice << " is not a valid option." << endl;
            break;
        }
    }

    return 0;
}
