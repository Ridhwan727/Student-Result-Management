// Student Result Management System
// OOPS (C++) PBL / Mini Project
// Concepts used: classes & objects, abstract class, inheritance, polymorphism,
// encapsulation, operator overloading, static members, STL vectors, file handling.
// Unique feature: Performance Trend Analyzer + Next-Exam Score Prediction.

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int NSUB = 5;
const char* SUBJECTS[NSUB] = {"Maths", "Physics", "Chemistry", "English", "Programming"};
const double PASS_MARK = 40.0;
const string DATA_FILE = "students.txt";

// ---------------------------------------------------------------- helpers
string toFile(string s) {
    replace(s.begin(), s.end(), ' ', '_');
    return s;
}
string fromFile(string s) {
    replace(s.begin(), s.end(), '_', ' ');
    return s;
}

void clearLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string readLine(const string& prompt) {
    string s;
    do {
        cout << prompt;
        getline(cin, s);
    } while (s.empty() && cin.good());
    return s;
}

int readInt(const string& prompt, int lo, int hi) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) {
            clearLine();
            return v;
        }
        if (cin.eof()) exit(0);
        cout << "  Enter a number between " << lo << " and " << hi << ".\n";
        clearLine();
    }
}

double readMarks(const string& prompt) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= 0 && v <= 100) {
            clearLine();
            return v;
        }
        if (cin.eof()) exit(0);
        cout << "  Marks must be between 0 and 100.\n";
        clearLine();
    }
}

// ---------------------------------------------------------------- ExamRecord
struct ExamRecord {
    string examName;
    double marks[NSUB];

    double total() const {
        double t = 0;
        for (int i = 0; i < NSUB; i++) t += marks[i];
        return t;
    }
    double percentage() const { return total() / NSUB; }

    string grade() const {
        double p = percentage();
        if (p >= 90) return "A+";
        if (p >= 80) return "A";
        if (p >= 70) return "B";
        if (p >= 60) return "C";
        if (p >= 50) return "D";
        if (p >= PASS_MARK) return "E";
        return "F";
    }

    bool passed() const {
        for (int i = 0; i < NSUB; i++)
            if (marks[i] < PASS_MARK) return false;
        return true;
    }
};

// ---------------------------------------------------------------- Person (abstract)
class Person {
protected:
    string name;

public:
    Person(const string& n = "") : name(n) {}
    virtual ~Person() {}
    string getName() const { return name; }
    void setName(const string& n) { name = n; }
    virtual void display() const = 0;  // pure virtual -> abstract class
};

// ---------------------------------------------------------------- Student
class Student : public Person {
private:
    int rollNo;
    vector<ExamRecord> exams;
    static int totalStudents;  // static member: counts students in memory

public:
    Student(int roll = 0, const string& n = "") : Person(n), rollNo(roll) { totalStudents++; }
    Student(const Student& other) : Person(other), rollNo(other.rollNo), exams(other.exams) {
        totalStudents++;
    }
    Student& operator=(const Student& other) {
        if (this != &other) {
            name = other.name;
            rollNo = other.rollNo;
            exams = other.exams;
        }
        return *this;
    }
    ~Student() { totalStudents--; }
    static int getTotalStudents() { return totalStudents; }

    int getRoll() const { return rollNo; }
    bool hasExams() const { return !exams.empty(); }
    const vector<ExamRecord>& getExams() const { return exams; }
    void addExam(const ExamRecord& e) { exams.push_back(e); }

    double latestPercentage() const { return exams.empty() ? 0.0 : exams.back().percentage(); }

    double averagePercentage() const {
        if (exams.empty()) return 0.0;
        double s = 0;
        for (size_t i = 0; i < exams.size(); i++) s += exams[i].percentage();
        return s / exams.size();
    }

    // index of lowest-scoring subject in the latest exam
    int weakestSubject() const {
        if (exams.empty()) return -1;
        const ExamRecord& e = exams.back();
        int w = 0;
        for (int i = 1; i < NSUB; i++)
            if (e.marks[i] < e.marks[w]) w = i;
        return w;
    }

    // ----- UNIQUE FEATURE: trend analysis and prediction -----
    // Least-squares line through the percentages of all exams; returns the
    // predicted percentage for the next exam, or -1 if fewer than 2 exams.
    double predictNext() const {
        size_t n = exams.size();
        if (n < 2) return -1;
        double sx = 0, sy = 0, sxy = 0, sxx = 0;
        for (size_t i = 0; i < n; i++) {
            double x = (double)i, y = exams[i].percentage();
            sx += x;
            sy += y;
            sxy += x * y;
            sxx += x * x;
        }
        double denom = n * sxx - sx * sx;
        double slope = (n * sxy - sx * sy) / denom;
        double intercept = (sy - slope * sx) / n;
        double p = intercept + slope * n;
        return max(0.0, min(100.0, p));
    }

    string trend() const {
        size_t n = exams.size();
        if (n < 2) return "Not enough data (need at least 2 exams)";
        double diff = exams[n - 1].percentage() - exams[n - 2].percentage();
        ostringstream os;
        os << fixed << setprecision(2);
        if (diff > 1.0)
            os << "IMPROVING (+" << diff << "%)";
        else if (diff < -1.0)
            os << "DECLINING (" << diff << "%)";
        else
            os << "STEADY (" << (diff >= 0 ? "+" : "") << diff << "%)";
        return os.str();
    }

    // polymorphic display (overrides Person::display)
    void display() const override {
        cout << "\n==============================================\n";
        cout << " REPORT CARD\n";
        cout << "==============================================\n";
        cout << " Name    : " << name << "\n Roll No : " << rollNo << "\n";
        if (exams.empty()) {
            cout << " No exam results recorded yet.\n";
            return;
        }
        for (size_t k = 0; k < exams.size(); k++) {
            const ExamRecord& e = exams[k];
            cout << "----------------------------------------------\n";
            cout << " Exam: " << e.examName << "\n";
            for (int i = 0; i < NSUB; i++)
                cout << "   " << left << setw(14) << SUBJECTS[i] << right << setw(6) << fixed
                     << setprecision(1) << e.marks[i] << (e.marks[i] < PASS_MARK ? "  (fail)" : "")
                     << "\n";
            cout << "   Total: " << e.total() << "/" << NSUB * 100 << "   Percentage: "
                 << setprecision(2) << e.percentage() << "%   Grade: " << e.grade()
                 << "   Result: " << (e.passed() ? "PASS" : "FAIL") << "\n";
        }
        cout << "==============================================\n";
    }

    // operator overloading: compare students by latest percentage
    bool operator>(const Student& other) const {
        return latestPercentage() > other.latestPercentage();
    }

    // file handling
    void save(ostream& out) const {
        out << rollNo << " " << toFile(name) << " " << exams.size() << "\n";
        for (size_t k = 0; k < exams.size(); k++) {
            out << toFile(exams[k].examName);
            for (int i = 0; i < NSUB; i++) out << " " << exams[k].marks[i];
            out << "\n";
        }
    }

    static bool load(istream& in, Student& s) {
        int roll;
        string nm;
        size_t cnt;
        if (!(in >> roll >> nm >> cnt)) return false;
        s = Student(roll, fromFile(nm));
        for (size_t k = 0; k < cnt; k++) {
            ExamRecord e;
            string en;
            in >> en;
            e.examName = fromFile(en);
            for (int i = 0; i < NSUB; i++) in >> e.marks[i];
            s.addExam(e);
        }
        return (bool)in;
    }
};
int Student::totalStudents = 0;

// operator overloading: stream insertion for a one-line summary
ostream& operator<<(ostream& os, const Student& s) {
    os << left << setw(6) << s.getRoll() << setw(20) << s.getName() << right << fixed
       << setprecision(2) << setw(8) << s.latestPercentage() << "%";
    return os;
}

// ---------------------------------------------------------------- ResultManager
class ResultManager {
private:
    vector<Student> students;

    int indexOf(int roll) const {
        for (size_t i = 0; i < students.size(); i++)
            if (students[i].getRoll() == roll) return (int)i;
        return -1;
    }

public:
    void load() {
        ifstream in(DATA_FILE);
        if (!in) return;
        Student s;
        while (Student::load(in, s)) students.push_back(s);
    }

    void save() const {
        ofstream out(DATA_FILE);
        for (size_t i = 0; i < students.size(); i++) students[i].save(out);
    }

    void addStudent() {
        int roll = readInt("Roll number: ", 1, 999999);
        if (indexOf(roll) != -1) {
            cout << "A student with this roll number already exists.\n";
            return;
        }
        string name = readLine("Student name: ");
        students.push_back(Student(roll, name));
        save();
        cout << "Student added.\n";
    }

    void addResult() {
        int roll = readInt("Roll number: ", 1, 999999);
        int i = indexOf(roll);
        if (i == -1) {
            cout << "Student not found.\n";
            return;
        }
        ExamRecord e;
        e.examName = readLine("Exam name (e.g. MST-1): ");
        for (int k = 0; k < NSUB; k++)
            e.marks[k] = readMarks(string("  ") + SUBJECTS[k] + " marks (0-100): ");
        students[i].addExam(e);
        save();
        cout << "Result saved. Percentage: " << fixed << setprecision(2) << e.percentage()
             << "%  Grade: " << e.grade() << "\n";
    }

    void viewStudent() const {
        int roll = readInt("Roll number: ", 1, 999999);
        int i = indexOf(roll);
        if (i == -1) {
            cout << "Student not found.\n";
            return;
        }
        const Person& p = students[i];  // polymorphic call through base reference
        p.display();
    }

    void listRanked() const {
        if (students.empty()) {
            cout << "No students yet.\n";
            return;
        }
        vector<Student> sorted = students;
        sort(sorted.begin(), sorted.end(), [](const Student& a, const Student& b) { return a > b; });
        cout << "\nRank  Roll  Name                 Latest %\n";
        cout << "----------------------------------------------\n";
        for (size_t i = 0; i < sorted.size(); i++)
            cout << left << setw(6) << i + 1 << sorted[i] << "\n";
    }

    void removeStudent() {
        int roll = readInt("Roll number to delete: ", 1, 999999);
        int i = indexOf(roll);
        if (i == -1) {
            cout << "Student not found.\n";
            return;
        }
        students.erase(students.begin() + i);
        save();
        cout << "Student deleted.\n";
    }

    void classStats() const {
        int graded = 0, passed = 0;
        double sum = 0;
        const Student* top = nullptr;
        for (size_t i = 0; i < students.size(); i++) {
            if (!students[i].hasExams()) continue;
            graded++;
            sum += students[i].latestPercentage();
            if (students[i].getExams().back().passed()) passed++;
            if (!top || students[i] > *top) top = &students[i];
        }
        cout << "\nStudents in system : " << Student::getTotalStudents() << "\n";
        if (graded == 0) {
            cout << "No results recorded yet.\n";
            return;
        }
        cout << fixed << setprecision(2);
        cout << "Class average      : " << sum / graded << "%\n";
        cout << "Pass percentage    : " << 100.0 * passed / graded << "%\n";
        cout << "Topper             : " << top->getName() << " (" << top->latestPercentage()
             << "%)\n";
    }

    // UNIQUE FEATURE menu option
    void trendAnalysis() const {
        int roll = readInt("Roll number: ", 1, 999999);
        int i = indexOf(roll);
        if (i == -1) {
            cout << "Student not found.\n";
            return;
        }
        const Student& s = students[i];
        cout << "\nPerformance analysis for " << s.getName() << "\n";
        if (!s.hasExams()) {
            cout << "No results recorded yet.\n";
            return;
        }
        cout << fixed << setprecision(2);
        const vector<ExamRecord>& ex = s.getExams();
        for (size_t k = 0; k < ex.size(); k++)
            cout << "  " << left << setw(14) << ex[k].examName << right << setw(7)
                 << ex[k].percentage() << "%\n";
        cout << "Average            : " << s.averagePercentage() << "%\n";
        cout << "Trend              : " << s.trend() << "\n";
        cout << "Weakest subject    : " << SUBJECTS[s.weakestSubject()] << " (latest exam)\n";
        double p = s.predictNext();
        if (p < 0)
            cout << "Predicted next exam: add at least 2 exams to get a prediction\n";
        else
            cout << "Predicted next exam: " << p << "%\n";
    }
};

// ---------------------------------------------------------------- main
int main() {
    ResultManager rm;
    rm.load();
    while (true) {
        cout << "\n===== STUDENT RESULT MANAGEMENT SYSTEM =====\n"
             << " 1. Add student\n"
             << " 2. Add exam result\n"
             << " 3. View report card\n"
             << " 4. Ranked list\n"
             << " 5. Delete student\n"
             << " 6. Class statistics\n"
             << " 7. Performance trend & prediction\n"
             << " 8. Exit\n";
        int ch = readInt("Choice: ", 1, 8);
        switch (ch) {
            case 1: rm.addStudent(); break;
            case 2: rm.addResult(); break;
            case 3: rm.viewStudent(); break;
            case 4: rm.listRanked(); break;
            case 5: rm.removeStudent(); break;
            case 6: rm.classStats(); break;
            case 7: rm.trendAnalysis(); break;
            case 8:
                rm.save();
                cout << "Data saved. Goodbye!\n";
                return 0;
        }
    }
}