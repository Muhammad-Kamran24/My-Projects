#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace std;

// =========================================================
//                CONFIGURATION & CONSTANTS
// =========================================================
const int MAX_STUDENTS = 1000;
const int MAX_TEACHERS = 100;
const int PASSING_MARKS = 40;
const int GRADE_A = 90;
const int CLASS_START_HOUR = 8;
const int DEFAULT_GRADE = 0;
const int DEFAULT_ATTENDANCE = 0;
const char ID_SEPARATOR = '-';

const string SCHOOL_NAME = "ABC INTERNATIONAL SCHOOL";

// File Paths
const string STUDENT_FILE = "students.csv";
const string TEACHER_FILE = "teachers.csv";
const string CLASS_FILE = "classes.csv";
const string ATTENDANCE_FILE = "attendance.csv";
const string ID_FILE_NAME = "id_counter.csv";

// Login Credentials
const string ADMIN_ID = "admin123";
const string ADMIN_PASSWORD = "pass123";

// =========================================================
//                   THEME & COLORS
// =========================================================
// Modern ANSI Color Codes for a CLI Interface
const string C_RESET = "\033[0m";
const string C_BOLD = "\033[1m";
const string C_CYAN = "\033[1;36m";    // Headers / Borders
const string C_GREEN = "\033[1;32m";   // Success / Key Info
const string C_YELLOW = "\033[1;33m";  // Warnings / Menus
const string C_RED = "\033[1;31m";     // Errors / Critical
const string C_BLUE = "\033[1;34m";    // Info / Tables
const string C_MAGENTA = "\033[1;35m"; // Highlights
const string C_DIM = "\033[2m";        // Subtle text

// Legacy color variables mapped to new theme for compatibility
string green = C_GREEN;
string red = C_RED;
string reset = C_RESET;
string cyan = C_CYAN;
string yellow = C_YELLOW;

// =========================================================
//                   STRUCT DEFINITIONS
// =========================================================
struct Student
{
    string id;
    string name;
    int age;
    int marks;
    char grade;
    string classID;
    int attendance;
};

struct Teacher
{
    string id;
    string name;
    int age;
    string subject;
};

struct Class
{
    string id;
    char section;
    string studentIDs[MAX_STUDENTS];
    int studentCount;
    string teacherID;
    string timetable;
};

// =========================================================
//                  GLOBAL DATA STORAGE
// =========================================================
Student students[MAX_STUDENTS];
Teacher teachers[MAX_TEACHERS];
Class classes[MAX_STUDENTS];

int studentCount = 0;
int teacherCount = 0;
int classCount = 0;

// =========================================================
//                 FUNCTION PROTOTYPES
// =========================================================

// --- UI Helpers ---
void printHeader(string title);
void printSubHeader(string title);
void printDivider();
void printPrompt(string label);
void printError(string message);
void printSuccess(string message);

// --- Core Logic ---
int generateUniqueID();
void validateInput();
void saveDataToFile();
void loadDataFromFile();
void backupData();

// --- Menus ---
void displayMainMenu();
void handleUserSelection(int choice);

// --- Student Module ---
void studentManagement();
void addStudent();
void viewAllStudents();
void searchStudentByID(string studentID);
void updateStudent(string studentID);
void deleteStudent(string studentID);
void recordAttendance(string studentID);
void generateStudentReport(string studentID);
bool isValidStudentID(const string &studentID);

// --- Teacher Module ---
void teacherManagement();
void addTeacher();
void viewAllTeachers();
void searchTeacherByID(string teacherID);
void updateTeacher(string teacherID);
void deleteTeacher(string teacherID);

// --- Class Module ---
void classManagement();
void addClass();
void viewAllClasses();
void assignTeacherToClass(string classID, string teacherID);
void assignStudentToClass(string classID, string studentID);
void createTimetable(string classID);
void viewTimetable(string classID);
bool isValidClassID(const string &classID);

// --- Exam & Attendance Module ---
void ExamAndGradeManagement();
void recordExamResult(string studentID, string subject, int marks, char grades);
void viewStudentGrades(string studentID);
void generateClassPerformanceReport(string classID);

void attendanceManagement();
void markStudentAttendance(string classID, string studentID, string date, bool present);
void viewAttendance(string studentID);
void viewClassAttendance(string classID);

// =========================================================
//                     MAIN FUNCTION
// =========================================================
int main()
{
    loadDataFromFile();
    int choice;
    string id;
    string pass;

login_label:
    cout << endl;
    cout << C_CYAN << "  ╔═══════════════════════════════════════════════╗" << C_RESET << endl;
    cout << C_CYAN << "  ║               SYSTEM LOGIN                    ║" << C_RESET << endl;
    cout << C_CYAN << "  ╚═══════════════════════════════════════════════╝" << C_RESET << endl;

    printPrompt("User ID");
    cin >> id;
    printPrompt("Password");
    cin >> pass;

    if (id == ADMIN_ID && pass == ADMIN_PASSWORD)
    {
        printSuccess("Access Granted! Welcome to " + SCHOOL_NAME);

        do
        {
            displayMainMenu();
            printPrompt("Enter your choice");
            cin >> choice;
            validateInput();
            handleUserSelection(choice);
            saveDataToFile();
        } while (choice != 6);
    }
    else
    {
        printError("Invalid Credentials! Please try again.");
        goto login_label;
    }

    printHeader("GOODBYE!");
    return 0;
}

// =========================================================
//                 UI HELPER IMPLEMENTATIONS
// =========================================================
void printHeader(string title)
{
    cout << endl;
    cout << C_CYAN << "============================================================" << C_RESET << endl;
    cout << C_BOLD << C_BLUE << "   " << title << C_RESET << endl;
    cout << C_CYAN << "============================================================" << C_RESET << endl;
}

void printSubHeader(string title)
{
    cout << endl
         << C_DIM << "--- " << title << " ---" << C_RESET << endl;
}

void printDivider()
{
    cout << C_CYAN << "------------------------------------------------------------" << C_RESET << endl;
}

void printPrompt(string label)
{
    cout << C_YELLOW << " >> " << C_RESET << label << ": ";
}

void printError(string message)
{
    cout << C_RED << " [!] Error: " << message << C_RESET << endl;
}

void printSuccess(string message)
{
    cout << C_GREEN << " [✔] " << message << C_RESET << endl;
}

// =========================================================
//                 UTILITY FUNCTIONS
// =========================================================
int generateUniqueID()
{
    static int idCounter = 1;

    ifstream inputFile(ID_FILE_NAME);
    if (inputFile.is_open())
    {
        inputFile >> idCounter;
        inputFile.close();
    }

    int newID = idCounter++;
    ofstream outputFile(ID_FILE_NAME);
    if (outputFile.is_open())
    {
        outputFile << idCounter;
        outputFile.close();
    }
    else
    {
        cerr << "Error: Unable to save ID to file." << endl;
    }

    return newID;
}

void validateInput()
{
    if (cin.fail())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        printError("Invalid input type.");
    }
}

bool isValidStudentID(const string &studentID)
{
    return studentID.rfind("STU", 0) == 0;
}

bool isValidClassID(const string &classID)
{
    return classID.rfind("CLS", 0) == 0;
}

// =========================================================
//                  FILE MANAGEMENT
// =========================================================
void saveDataToFile()
{
    // Save Students
    ofstream studentFile(STUDENT_FILE);
    if (!studentFile)
    {
        printError("Unable to open students.csv for writing.");
        return;
    }
    studentFile << "ID,Name,Age,Grade,Class ID,Attendance\n";
    for (int i = 0; i < studentCount; i++)
    {
        studentFile << students[i].id << ","
                    << students[i].name << ","
                    << students[i].age << ","
                    << students[i].grade << ","
                    << students[i].classID << ","
                    << students[i].attendance << endl;
    }
    studentFile.close();

    // Save Teachers
    ofstream teacherFile(TEACHER_FILE);
    if (!teacherFile)
    {
        printError("Unable to open teachers.csv for writing.");
        return;
    }
    teacherFile << "ID,Name,Age,Subject\n";
    for (int i = 0; i < teacherCount; i++)
    {
        teacherFile << teachers[i].id << ","
                    << teachers[i].name << ","
                    << teachers[i].age << ","
                    << teachers[i].subject << endl;
    }
    teacherFile.close();
}

void loadDataFromFile()
{
    ifstream studentFile(STUDENT_FILE);
    ifstream teacherFile(TEACHER_FILE);
    string line;

    studentCount = 0;
    teacherCount = 0;

    if (studentFile.is_open())
    {
        getline(studentFile, line); // Skip header
        while (!studentFile.eof() && studentCount < MAX_STUDENTS)
        {
            getline(studentFile, students[studentCount].id, ',');
            if (students[studentCount].id.empty())
                continue; // simple EOF check
            getline(studentFile, students[studentCount].name, ',');
            studentFile >> students[studentCount].age;
            studentFile.ignore();
            studentFile >> students[studentCount].grade;
            studentFile.ignore();
            getline(studentFile, students[studentCount].classID, ',');
            studentFile >> students[studentCount].attendance;
            studentFile.ignore();
            studentCount++;
        }
        studentFile.close();
    }

    if (teacherFile.is_open())
    {
        getline(teacherFile, line); // Skip header
        while (!teacherFile.eof() && teacherCount < MAX_TEACHERS)
        {
            getline(teacherFile, teachers[teacherCount].id, ',');
            if (teachers[teacherCount].id.empty())
                continue; // simple EOF check
            getline(teacherFile, teachers[teacherCount].name, ',');
            teacherFile >> teachers[teacherCount].age;
            teacherFile.ignore();
            getline(teacherFile, teachers[teacherCount].subject);
            teacherCount++;
        }
        teacherFile.close();
    }
}

void backupData()
{
    saveDataToFile();
    printSuccess("Backup created successfully!");
}

// =========================================================
//                    MENU FUNCTIONS
// =========================================================
void displayMainMenu()
{
    cout << endl;
    cout << C_CYAN << "╔══════════════════════════════════════════════════════════╗" << C_RESET << endl;
    cout << C_CYAN << "║           " << C_BOLD << C_YELLOW << SCHOOL_NAME << C_RESET << C_CYAN << "                       ║" << C_RESET << endl;
    cout << C_CYAN << "╠══════════════════════════════════════════════════════════╣" << C_RESET << endl;
    cout << C_CYAN << "║ " << C_GREEN << "1." << C_RESET << " Student Management                                    ║" << C_RESET << endl;
    cout << C_CYAN << "║ " << C_GREEN << "2." << C_RESET << " Teacher Management                                    ║" << C_RESET << endl;
    cout << C_CYAN << "║ " << C_GREEN << "3." << C_RESET << " Class Management                                      ║" << C_RESET << endl;
    cout << C_CYAN << "║ " << C_GREEN << "4." << C_RESET << " Attendance Management                                 ║" << C_RESET << endl;
    cout << C_CYAN << "║ " << C_GREEN << "5." << C_RESET << " Examination Module                                    ║" << C_RESET << endl;
    cout << C_CYAN << "║ " << C_GREEN << "6." << C_RESET << " Save and Exit                                         ║" << C_RESET << endl;
    cout << C_CYAN << "╚══════════════════════════════════════════════════════════╝" << C_RESET << endl;
}

void handleUserSelection(int choice)
{
    switch (choice)
    {
    case 1:
        studentManagement();
        break;
    case 2:
        teacherManagement();
        break;
    case 3:
        classManagement();
        break;
    case 4:
        attendanceManagement();
        break;
    case 5:
        ExamAndGradeManagement();
        break;
    case 6:
        cout << C_YELLOW << "Exiting system..." << C_RESET << endl;
        break;
    default:
        printError("Invalid choice.");
    }
}

// =========================================================
//                 STUDENT MANAGEMENT
// =========================================================
void studentManagement()
{
    int choice;
    do
    {
        printHeader("STUDENT MANAGEMENT");
        cout << " 1. Add Student" << endl;
        cout << " 2. View All Students" << endl;
        cout << " 3. Search Student by ID" << endl;
        cout << " 4. Update Student" << endl;
        cout << " 5. Delete Student" << endl;
        cout << " 6. Record Attendance" << endl;
        cout << " 7. Generate Student Report" << endl;
        cout << " 8. Return to Main Menu" << endl;
        printDivider();

        printPrompt("Select Option");
        cin >> choice;
        validateInput();

        switch (choice)
        {
        case 1:
            addStudent();
            break;
        case 2:
            viewAllStudents();
            break;
        case 3:
        {
            string id;
            printPrompt("Student ID");
            cin >> id;
            searchStudentByID(id);
            break;
        }
        case 4:
        {
            string id;
            printPrompt("Student ID to Update");
            cin >> id;
            updateStudent(id);
            break;
        }
        case 5:
        {
            string id;
            printPrompt("Student ID to Delete");
            cin >> id;
            deleteStudent(id);
            break;
        }
        case 6:
        {
            string id;
            printPrompt("Student ID");
            cin >> id;
            recordAttendance(id);
            break;
        }
        case 7:
        {
            string id;
            printPrompt("Student ID");
            cin >> id;
            generateStudentReport(id);
            break;
        }
        case 8:
            break;
        default:
            printError("Invalid Option.");
        }
    } while (choice != 8);
}

void addStudent()
{
    if (studentCount >= MAX_STUDENTS)
    {
        printError("Maximum student limit reached!");
        return;
    }

    Student newStudent;
    newStudent.id = "STU-" + to_string(generateUniqueID());

    printSubHeader("ADD NEW STUDENT");
    printPrompt("Name");
    cin.ignore();
    getline(cin, newStudent.name);
    printPrompt("Age");
    cin >> newStudent.age;
    validateInput();

    newStudent.grade = DEFAULT_GRADE;
    newStudent.attendance = DEFAULT_ATTENDANCE;

    students[studentCount++] = newStudent;
    printSuccess("Student added! ID: " + newStudent.id);
}

void viewAllStudents()
{
    if (studentCount == 0)
    {
        printError("No students recorded.");
        return;
    }

    printHeader("ALL STUDENTS LIST");
    cout << C_BLUE << left
         << setw(15) << "ID"
         << setw(25) << "Name"
         << setw(5) << "Age" << C_RESET << endl;
    printDivider();

    for (int i = 0; i < studentCount; i++)
    {
        cout << left
             << setw(15) << students[i].id
             << setw(25) << students[i].name
             << setw(5) << students[i].age << endl;
    }
    printDivider();
}

void searchStudentByID(string studentID)
{
    bool found = false;
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            found = true;
            printHeader("STUDENT DETAILS");
            cout << C_CYAN << left << setw(15) << "Field" << "Value" << C_RESET << endl;
            printDivider();
            cout << left << setw(15) << "ID" << students[i].id << endl;
            cout << left << setw(15) << "Name" << students[i].name << endl;
            cout << left << setw(15) << "Age" << students[i].age << endl;
            cout << left << setw(15) << "Grade" << (int)students[i].grade << endl;
            cout << left << setw(15) << "Class ID" << students[i].classID << endl;
            cout << left << setw(15) << "Attendance" << students[i].attendance << endl;
            printDivider();
            break;
        }
    }
    if (!found)
        printError("Student ID " + studentID + " not found.");
}

void updateStudent(string studentID)
{
    bool found = false;
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            found = true;
            printSubHeader("UPDATE STUDENT: " + students[i].name);

            printPrompt("New Name (enter to skip)");
            cin.ignore();
            string newName;
            getline(cin, newName);
            if (!newName.empty())
                students[i].name = newName;

            printPrompt("New Age (-1 to skip)");
            int newAge;
            cin >> newAge;
            if (newAge != -1)
                students[i].age = newAge;

            printPrompt("New Grade (-1 to skip)");
            int newGrade;
            cin >> newGrade;
            if (newGrade != -1)
                students[i].grade = newGrade;

            printPrompt("New Class ID (enter to skip)");
            cin.ignore();
            string newClassID;
            getline(cin, newClassID);
            if (!newClassID.empty())
                students[i].classID = newClassID;

            printSuccess("Student updated successfully.");
            break;
        }
    }
    if (!found)
        printError("Student ID not found.");
}

void deleteStudent(string studentID)
{
    bool found = false;
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            found = true;
            for (int j = i; j < studentCount - 1; j++)
            {
                students[j] = students[j + 1];
            }
            studentCount--;
            printSuccess("Student record deleted.");
            break;
        }
    }
    if (!found)
        printError("Student ID not found.");
}

void recordAttendance(string studentID)
{
    int status;
    bool found = false;
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            found = true;
            printPrompt("Attendance (1=Present, 0=Absent)");
            cin >> status;
            if (status == 1)
            {
                students[i].attendance++;
                printSuccess("Marked Present.");
            }
            else
            {
                printError("Marked Absent.");
            }
            break;
        }
    }
    if (!found)
        printError("Student ID not found.");
}

void generateStudentReport(string studentID)
{
    bool found = false;
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            found = true;
            printHeader("OFFICIAL REPORT CARD");
            cout << " Student: " << C_BOLD << students[i].name << C_RESET << " (" << students[i].id << ")" << endl;
            printDivider();

            cout << left << setw(20) << "Class Enrolled" << ": " << students[i].classID << endl;
            cout << left << setw(20) << "Attendance Days" << ": " << students[i].attendance << endl;
            cout << left << setw(20) << "Grade Obtained" << ": " << (int)students[i].grade << endl;

            cout << endl
                 << " Performance Status: ";
            if (students[i].grade >= GRADE_A)
                cout << C_GREEN << C_BOLD << "EXCELLENT (A)" << C_RESET << endl;
            else if (students[i].grade >= PASSING_MARKS)
                cout << C_BLUE << "PASSING" << C_RESET << endl;
            else
                cout << C_RED << "NEEDS IMPROVEMENT" << C_RESET << endl;

            printDivider();
            break;
        }
    }
    if (!found)
        printError("Student ID not found.");
}

// =========================================================
//                 TEACHER MANAGEMENT
// =========================================================
void teacherManagement()
{
    int choice;
    do
    {
        printHeader("TEACHER MANAGEMENT");
        cout << " 1. Add Teacher" << endl;
        cout << " 2. View All Teachers" << endl;
        cout << " 3. Search Teacher by ID" << endl;
        cout << " 4. Update Teacher" << endl;
        cout << " 5. Delete Teacher" << endl;
        cout << " 6. Return to Main Menu" << endl;
        printDivider();

        printPrompt("Select Option");
        cin >> choice;
        validateInput();

        switch (choice)
        {
        case 1:
            addTeacher();
            break;
        case 2:
            viewAllTeachers();
            break;
        case 3:
        {
            string id;
            printPrompt("Teacher ID");
            cin >> id;
            searchTeacherByID(id);
            break;
        }
        case 4:
        {
            string id;
            printPrompt("Teacher ID to Update");
            cin >> id;
            updateTeacher(id);
            break;
        }
        case 5:
        {
            string id;
            printPrompt("Teacher ID to Delete");
            cin >> id;
            deleteTeacher(id);
            break;
        }
        case 6:
            break;
        default:
            printError("Invalid Option.");
        }
    } while (choice != 6);
}

void addTeacher()
{
    if (teacherCount >= MAX_TEACHERS)
    {
        printError("Maximum teacher limit reached!");
        return;
    }
    Teacher newT;
    newT.id = "TEA-" + to_string(generateUniqueID());

    printSubHeader("ADD NEW TEACHER");
    printPrompt("Name");
    cin.ignore();
    getline(cin, newT.name);
    printPrompt("Age");
    cin >> newT.age;
    printPrompt("Subject");
    cin.ignore();
    getline(cin, newT.subject);

    teachers[teacherCount++] = newT;
    printSuccess("Teacher added! ID: " + newT.id);
}

void viewAllTeachers()
{
    if (teacherCount == 0)
    {
        printError("No teachers recorded.");
        return;
    }
    printHeader("FACULTY LIST");
    cout << C_BLUE << left
         << setw(15) << "ID"
         << setw(25) << "Name"
         << setw(20) << "Subject" << C_RESET << endl;
    printDivider();

    for (int i = 0; i < teacherCount; i++)
    {
        cout << left
             << setw(15) << teachers[i].id
             << setw(25) << teachers[i].name
             << setw(20) << teachers[i].subject << endl;
    }
    printDivider();
}

void searchTeacherByID(string teacherID)
{
    bool found = false;
    for (int i = 0; i < teacherCount; i++)
    {
        if (teachers[i].id == teacherID)
        {
            found = true;
            printHeader("TEACHER DETAILS");
            cout << " ID     : " << teachers[i].id << endl;
            cout << " Name   : " << teachers[i].name << endl;
            cout << " Age    : " << teachers[i].age << endl;
            cout << " Subject: " << teachers[i].subject << endl;
            printDivider();
            break;
        }
    }
    if (!found)
        printError("Teacher ID not found.");
}

void updateTeacher(string teacherID)
{
    bool found = false;
    for (int i = 0; i < teacherCount; i++)
    {
        if (teachers[i].id == teacherID)
        {
            found = true;
            printSubHeader("UPDATE TEACHER");

            printPrompt("New Name (enter to skip)");
            cin.ignore();
            string newName;
            getline(cin, newName);
            if (!newName.empty())
                teachers[i].name = newName;

            printPrompt("New Age (-1 to skip)");
            int newAge;
            cin >> newAge;
            if (newAge != -1)
                teachers[i].age = newAge;

            printPrompt("New Subject (enter to skip)");
            cin.ignore();
            string newSub;
            getline(cin, newSub);
            if (!newSub.empty())
                teachers[i].subject = newSub;

            printSuccess("Teacher updated.");
            break;
        }
    }
    if (!found)
        printError("Teacher ID not found.");
}

void deleteTeacher(string teacherID)
{
    bool found = false;
    for (int i = 0; i < teacherCount; i++)
    {
        if (teachers[i].id == teacherID)
        {
            found = true;
            for (int j = i; j < teacherCount - 1; j++)
            {
                teachers[j] = teachers[j + 1];
            }
            teacherCount--;
            printSuccess("Teacher deleted.");
            break;
        }
    }
    if (!found)
        printError("Teacher ID not found.");
}

// =========================================================
//                 CLASS MANAGEMENT
// =========================================================
void classManagement()
{
    int choice;
    do
    {
        printHeader("CLASS MANAGEMENT");
        cout << " 1. Add Class" << endl;
        cout << " 2. View All Classes" << endl;
        cout << " 3. Assign Teacher to Class" << endl;
        cout << " 4. Assign Student to Class" << endl;
        cout << " 5. Create Timetable" << endl;
        cout << " 6. View Timetable" << endl;
        cout << " 7. Return to Main Menu" << endl;
        printDivider();

        printPrompt("Select Option");
        cin >> choice;
        validateInput();

        switch (choice)
        {
        case 1:
            addClass();
            break;
        case 2:
            viewAllClasses();
            break;
        case 3:
        {
            string cid, tid;
            printPrompt("Class ID");
            cin >> cid;
            printPrompt("Teacher ID");
            cin >> tid;
            assignTeacherToClass(cid, tid);
            break;
        }
        case 4:
        {
            string cid, sid;
            printPrompt("Class ID");
            cin >> cid;
            printPrompt("Student ID");
            cin >> sid;
            assignStudentToClass(cid, sid);
            break;
        }
        case 5:
        {
            string cid;
            printPrompt("Class ID");
            cin >> cid;
            createTimetable(cid);
            break;
        }
        case 6:
        {
            string cid;
            printPrompt("Class ID");
            cin >> cid;
            viewTimetable(cid);
            break;
        }
        case 7:
            break;
        default:
            printError("Invalid Option.");
        }
    } while (choice != 7);
}

void addClass()
{
    if (classCount >= MAX_STUDENTS)
    {
        printError("Limit reached.");
        return;
    }
    Class newC;
    newC.id = "CLS-" + to_string(generateUniqueID());
    printPrompt("Section (A/B/C)");
    cin >> newC.section;
    newC.studentCount = 0;
    newC.teacherID = "";
    newC.timetable = "";

    classes[classCount++] = newC;
    printSuccess("Class created! ID: " + newC.id);
}

void viewAllClasses()
{
    if (classCount == 0)
    {
        printError("No classes found.");
        return;
    }
    printHeader("CLASS LIST");
    cout << C_BLUE << left << setw(15) << "Class ID" << setw(10) << "Section" << setw(15) << "Teacher ID" << C_RESET << endl;
    printDivider();
    for (int i = 0; i < classCount; i++)
    {
        cout << left << setw(15) << classes[i].id << setw(10) << classes[i].section;
        if (!classes[i].teacherID.empty())
            cout << setw(15) << classes[i].teacherID;
        else
            cout << setw(15) << "N/A";
        cout << endl;
    }
    printDivider();
}

void assignTeacherToClass(string classID, string teacherID)
{
    bool cFound = false, tFound = false;
    for (int i = 0; i < classCount; i++)
    {
        if (classes[i].id == classID)
        {
            cFound = true;
            for (int j = 0; j < teacherCount; j++)
            {
                if (teachers[j].id == teacherID)
                {
                    tFound = true;
                    classes[i].teacherID = teacherID;
                    printSuccess("Teacher " + teacherID + " assigned to " + classID);
                    break;
                }
            }
            break;
        }
    }
    if (!cFound)
        printError("Class not found.");
    else if (!tFound)
        printError("Teacher not found.");
}

void assignStudentToClass(string classID, string studentID)
{
    bool cFound = false, sFound = false;
    for (int i = 0; i < classCount; i++)
    {
        if (classes[i].id == classID)
        {
            cFound = true;
            for (int j = 0; j < studentCount; j++)
            {
                if (students[j].id == studentID)
                {
                    sFound = true;
                    if (classes[i].studentCount < MAX_STUDENTS)
                    {
                        classes[i].studentIDs[classes[i].studentCount++] = studentID;
                        students[j].classID = classID;
                        printSuccess("Student assigned to class.");
                    }
                    else
                        printError("Class full.");
                    break;
                }
            }
            break;
        }
    }
    if (!cFound)
        printError("Class not found.");
    else if (!sFound)
        printError("Student not found.");
}

void createTimetable(string classID)
{
    for (int i = 0; i < classCount; i++)
    {
        if (classes[i].id == classID)
        {
            printSubHeader("CREATE TIMETABLE");
            cout << C_DIM << " Format: Monday 8:00 AM - Math" << C_RESET << endl;
            cout << C_DIM << " Type 'done' to finish." << C_RESET << endl;
            string fullTable = "", line;
            cin.ignore();
            while (true)
            {
                printPrompt("Entry");
                getline(cin, line);
                if (line == "done")
                    break;
                fullTable += line + "\n";
            }
            classes[i].timetable = fullTable;
            printSuccess("Timetable saved.");
            return;
        }
    }
    printError("Class not found.");
}

void viewTimetable(string classID)
{
    for (int i = 0; i < classCount; i++)
    {
        if (classes[i].id == classID)
        {
            printHeader("TIMETABLE FOR " + classID);
            if (classes[i].timetable.empty())
                printError("No timetable set.");
            else
                cout << classes[i].timetable << endl;
            printDivider();
            return;
        }
    }
    printError("Class not found.");
}

// =========================================================
//              EXAM & ATTENDANCE MANAGEMENT
// =========================================================
void ExamAndGradeManagement()
{
    int choice;
    do
    {
        printHeader("EXAM & GRADES");
        cout << " 1. Record Exam Result" << endl;
        cout << " 2. View Student Grades" << endl;
        cout << " 3. Class Performance Report" << endl;
        cout << " 4. Return to Main Menu" << endl;
        printDivider();
        printPrompt("Select Option");
        cin >> choice;
        validateInput();

        switch (choice)
        {
        case 1:
        {
            string sid, sub;
            int marks, total;
            char grd;
            printPrompt("Student ID");
            cin >> sid;
            printPrompt("Subject");
            cin.ignore();
            getline(cin, sub);
            printPrompt("Total Marks");
            cin >> total;
            printPrompt("Marks Obtained");
            cin >> marks;

            while (marks < 0 || marks > total)
            {
                printError("Invalid marks. Try again.");
                printPrompt("Marks Obtained");
                cin >> marks;
            }

            if (marks >= 90)
                grd = 'A';
            else if (marks >= 80)
                grd = 'B';
            else if (marks >= 60)
                grd = 'C';
            else if (marks >= 40)
                grd = 'D';
            else
                grd = 'F';

            recordExamResult(sid, sub, marks, grd);
            break;
        }
        case 2:
        {
            string sid;
            printPrompt("Student ID");
            cin >> sid;
            viewStudentGrades(sid);
            break;
        }
        case 3:
        {
            string cid;
            printPrompt("Class ID");
            cin >> cid;
            generateClassPerformanceReport(cid);
            break;
        }
        case 4:
            break;
        default:
            printError("Invalid Option.");
        }
    } while (choice != 4);
}

void recordExamResult(string studentID, string subject, int marks, char grades)
{
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            students[i].marks = marks;
            students[i].grade = grades;
            printSuccess("Exam results recorded for " + students[i].name);
            return;
        }
    }
    printError("Student not found.");
}

void viewStudentGrades(string studentID)
{
    for (int i = 0; i < studentCount; i++)
    {
        if (students[i].id == studentID)
        {
            printHeader("GRADE CARD");
            cout << " ID    : " << studentID << endl;
            cout << " Name  : " << students[i].name << endl;
            cout << " Marks : " << students[i].marks << endl;
            cout << " Grade : " << students[i].grade << endl;
            printDivider();
            return;
        }
    }
    printError("Student not found.");
}

void generateClassPerformanceReport(string classID)
{
    bool found = false;
    for (int i = 0; i < classCount; i++)
    {
        if (classes[i].id == classID)
        {
            found = true;
            printHeader("PERFORMANCE REPORT: " + classID);
            cout << C_BLUE << left << setw(15) << "ID" << setw(20) << "Name" << setw(10) << "Grade" << C_RESET << endl;
            printDivider();

            for (int j = 0; j < classes[i].studentCount; j++)
            {
                string sid = classes[i].studentIDs[j];
                for (int k = 0; k < studentCount; k++)
                {
                    if (students[k].id == sid)
                    {
                        cout << left << setw(15) << students[k].id
                             << setw(20) << students[k].name
                             << setw(10) << students[k].grade << endl;
                        break;
                    }
                }
            }
            printDivider();
            break;
        }
    }
    if (!found)
        printError("Class not found.");
}

void attendanceManagement()
{
    int choice;
    do
    {
        printHeader("ATTENDANCE MENU");
        cout << " 1. Mark Attendance" << endl;
        cout << " 2. View Student Attendance" << endl;
        cout << " 3. View Class Attendance" << endl;
        cout << " 4. Return to Main Menu" << endl;
        printDivider();
        printPrompt("Select Option");
        cin >> choice;
        validateInput();

        switch (choice)
        {
        case 1:
        {
            string cid, sid, date;
            bool p;
            do
            {
                printPrompt("Class ID");
                cin >> cid;
            } while (!isValidClassID(cid));
            do
            {
                printPrompt("Student ID");
                cin >> sid;
            } while (!isValidStudentID(sid));
            printPrompt("Date (YYYY-MM-DD)");
            cin >> date;
            do
            {
                printPrompt("Present? (1=Yes, 0=No)");
                cin >> p;
                if (cin.fail())
                {
                    cin.clear();
                    cin.ignore();
                    p = 2;
                }
            } while (p != 0 && p != 1);

            markStudentAttendance(cid, sid, date, p);
            break;
        }
        case 2:
        {
            string sid;
            printPrompt("Student ID");
            cin >> sid;
            viewAttendance(sid);
            break;
        }
        case 3:
        {
            string cid;
            printPrompt("Class ID");
            cin >> cid;
            viewClassAttendance(cid);
            break;
        }
        case 4:
            break;
        default:
            printError("Invalid Option.");
        }
    } while (choice != 4);
}

void markStudentAttendance(string classID, string studentID, string date, bool present)
{
    bool cFound = false, sInClass = false;
    for (int i = 0; i < classCount; i++)
    {
        if (classes[i].id == classID)
        {
            cFound = true;
            for (int j = 0; j < classes[i].studentCount; j++)
            {
                if (classes[i].studentIDs[j] == studentID)
                {
                    sInClass = true;
                    ofstream file(ATTENDANCE_FILE, ios::app);
                    file << studentID << "," << classID << "," << date << "," << (present ? "Present" : "Absent") << endl;
                    file.close();
                    printSuccess("Attendance Marked.");
                    break;
                }
            }
            break;
        }
    }
    if (!cFound)
        printError("Class not found.");
    else if (!sInClass)
        printError("Student not found in this class.");
}

void viewAttendance(string studentID)
{
    ifstream file(ATTENDANCE_FILE);
    if (!file.is_open())
    {
        printError("Cannot open attendance file.");
        return;
    }

    printHeader("ATTENDANCE LOG: " + studentID);
    cout << C_BLUE << left << setw(15) << "Class ID" << setw(15) << "Date" << setw(15) << "Status" << C_RESET << endl;
    printDivider();

    string line;
    bool found = false;
    while (getline(file, line))
    {
        stringstream ss(line);
        string sid, cid, date, status;
        getline(ss, sid, ',');
        getline(ss, cid, ',');
        getline(ss, date, ',');
        getline(ss, status, ',');

        if (sid == studentID)
        {
            cout << left << setw(15) << cid << setw(15) << date << setw(15) << status << endl;
            found = true;
        }
    }
    if (!found)
        cout << C_DIM << " No records found." << C_RESET << endl;
    printDivider();
    file.close();
}

void viewClassAttendance(string classID)
{
    ifstream file(ATTENDANCE_FILE);
    if (!file.is_open())
    {
        printError("Cannot open attendance file.");
        return;
    }

    printHeader("CLASS ATTENDANCE: " + classID);
    cout << C_BLUE << left << setw(15) << "Student ID" << setw(15) << "Date" << setw(15) << "Status" << C_RESET << endl;
    printDivider();

    string line;
    bool found = false;
    while (getline(file, line))
    {
        stringstream ss(line);
        string sid, cid, date, status;
        getline(ss, sid, ',');
        getline(ss, cid, ',');
        getline(ss, date, ',');
        getline(ss, status, ',');

        if (cid == classID)
        {
            cout << left << setw(15) << sid << setw(15) << date << setw(15) << status << endl;
            found = true;
        }
    }
    if (!found)
        cout << C_DIM << " No records found." << C_RESET << endl;
    printDivider();
    file.close();
}