#include <iostream>
#include <sqlite3.h> 
#include <string>
#include <limits>  // For numeric_limits
using namespace std;
bool isValidString(const string& str);
bool isValidMobile(const string& mobile);
// Employee class
class Employee {
public:
    string employeeId;
    string employeeName;
    int age;
    string mobileNumber;
    string department;
    double salary;

    // Constructor
    Employee() {}

    Employee(const string& id, const string& name, int age, const string& mobile, const string& dept, double sal)
        : employeeId(id), employeeName(name), age(age), mobileNumber(mobile), department(dept), salary(sal) {}

    // Method to display employee details
    void displayDetails() const {
        cout << "Employee ID: " << employeeId << endl;
        cout << "Employee Name: " << employeeName << endl;
        cout << "Age: " << age << endl;
        cout << "Mobile Number: " << mobileNumber << endl;
        cout << "Department: " << department << endl;
        cout << "Salary: " << salary << endl;
        cout << "-----------------------------" << endl;
    }
};

// DatabaseManager class to manage SQLite operations
class DatabaseManager {
private:
    sqlite3* db;

public:
    DatabaseManager(const string& dbName) {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            cout << "Error opening database: " << sqlite3_errmsg(db) << endl;
            db = nullptr;
        }
        else {
            cout << "Database opened successfully." << endl;
        }
    }

    ~DatabaseManager() {
        if (db) {
            sqlite3_close(db);
            cout << "Database closed." << endl;
        }
    }

    bool executeQuery(const string& query) {
        char* errMsg = nullptr;
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cout << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    sqlite3* getDatabase() const {
        return db;
    }
};

// EmployeeManager class to manage employee operations
class EmployeeManager {
private:
    DatabaseManager& dbManager;

public:
    EmployeeManager(DatabaseManager& db) : dbManager(db) {
        // Create the employee table if it does not exist
        string createTableQuery = "CREATE TABLE IF NOT EXISTS Employee ("
            "EmployeeID TEXT PRIMARY KEY, "
            "Name TEXT NOT NULL, "
            "Age INTEGER, "
            "MobileNumber TEXT, "
            "Department TEXT, "
            "Salary REAL);";
        dbManager.executeQuery(createTableQuery);
    }

    // Method to check if EmployeeID already exists
    bool isEmployeeIdExists(const string& employeeId) {
        string query = "SELECT COUNT(*) FROM Employee WHERE EmployeeID = '" + employeeId + "';";
        sqlite3_stmt* stmt;
        int count = 0;

        if (sqlite3_prepare_v2(dbManager.getDatabase(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);

        return (count > 0);
    }

    // Method to add an employee to the database
    void addEmployee(const Employee& employee) {
        string query = "INSERT INTO Employee (EmployeeID, Name, Age, MobileNumber, Department, Salary) VALUES ('" +
            employee.employeeId + "', '" + employee.employeeName + "', " + to_string(employee.age) + ", '" +
            employee.mobileNumber + "', '" + employee.department + "', " +
            to_string(employee.salary) + ");";
        if (dbManager.executeQuery(query)) {
            cout << "Employee added successfully." << endl;
        }
    }

    // Method to display all employees
    static int displayCallback(void* NotUsed, int argc, char** argv, char** azColName) {
        for (int i = 0; i < argc; i++) {
            cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << endl;
        }
        cout << "-----------------------------" << endl;
        return 0;
    }

    void displayAllEmployees() {
        string query = "SELECT * FROM Employee;";
        if (sqlite3_exec(dbManager.getDatabase(), query.c_str(), displayCallback, nullptr, nullptr) != SQLITE_OK) {
            cout << "Failed to retrieve employee records." << endl;
        }
    }

    // Method to delete an employee from the database by EmployeeID
    void deleteEmployeeById(const string& employeeId) {
        string query = "DELETE FROM Employee WHERE EmployeeID = '" + employeeId + "';";
        if (dbManager.executeQuery(query)) {
            cout << "Employee with ID " << employeeId << " deleted successfully." << endl;
        }
        else {
            cout << "Failed to delete employee. Please check if the EmployeeID is valid." << endl;
        }
    }

    // Method to update employee by EmployeeID
    void updateEmployeeById(const string& employeeId) {
        if (!isEmployeeIdExists(employeeId)) {
            cout << "Employee with ID " << employeeId << " does not exist!" << endl;
            return;
        }

        string name, department, mobile;
        int age;
        double salary;

        // Get valid inputs for each field
        do {
            cout << "Enter new Name: ";
            cin >> name;
            if (!isValidString(name)) {
                cout << "Invalid name! Name should contain only alphabets. Please try again." << endl;
            }
        } while (!isValidString(name));

        while (true) {
            cout << "Enter new Age: ";
            cin >> age;
            if (cin.fail() || age <= 0) {
                cout << "Invalid age! Age should be a positive number. Please try again." << endl;
                cin.clear();  
                cin.ignore(numeric_limits<streamsize>::max(), '\n');  
            }
            else {
                break;  // Exit the loop if input is valid
            }
        }

        do {
            cout << "Enter new Mobile Number: ";
            cin >> mobile;
            if (!isValidMobile(mobile)) {
                cout << "Invalid mobile number! It should contain exactly 10 digits. Please try again." << endl;
            }
        } while (!isValidMobile(mobile));

        do {
            cout << "Enter new Department: ";
            cin >> department;
            if (!isValidString(department)) {
                cout << "Invalid department! It should contain only alphabets. Please try again." << endl;
            }
        } while (!isValidString(department));

        cout << "Enter new Salary: ";
        cin >> salary;

        // Prepare the update query
        string query = "UPDATE Employee SET "
            "Name = '" + name + "', "
            "Age = " + to_string(age) + ", "
            "MobileNumber = '" + mobile + "', "
            "Department = '" + department + "', "
            "Salary = " + to_string(salary) +
            " WHERE EmployeeID = '" + employeeId + "';";

        if (dbManager.executeQuery(query)) {
            cout << "Employee updated successfully." << endl;
        }
    }
};

// Helper functions for validation (name, mobile number, etc.)
bool isValidString(const string& str) {
    for (char c : str) {
        if (!isalpha(c)) return false;
    }
    return true;
}

bool isValidMobile(const string& mobile) {
    if (mobile.length() != 10) return false;
    for (char c : mobile) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Main function
int main() {
    DatabaseManager dbManager("employee.db");
    EmployeeManager empManager(dbManager);

    int choice;
    while (true) {
        cout << "\nEmployee Management System\n";
        cout << "1. Add Employee\n";
        cout << "2. Display All Employees\n";
        cout << "3. Delete Employee\n";
        cout << "4. Update Employee\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 1) {
            string id, name, department, mobile;
            int age;
            double salary;

            // Get a valid, unique Employee ID
            do {
                cout << "Enter Employee ID: ";
                cin >> id;

                if (empManager.isEmployeeIdExists(id)) {
                    cout << "Employee ID already exists! Please enter a unique ID." << endl;
                }
            } while (empManager.isEmployeeIdExists(id));

            // Get other valid inputs
            do {
                cout << "Enter Name: ";
                cin >> name;
                if (!isValidString(name)) {
                    cout << "Invalid name! Name should contain only alphabets. Please try again." << endl;
                }
            } while (!isValidString(name));

            while (true) {
                cout << "Enter Age: ";
                cin >> age;
                if (cin.fail() || age <= 0) {
                    cout << "Invalid age! Age should be a positive number. Please try again." << endl;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                else {
                    break;
                }
            }

            do {
                cout << "Enter Mobile Number: ";
                cin >> mobile;
                if (!isValidMobile(mobile)) {
                    cout << "Invalid mobile number! It should contain exactly 10 digits. Please try again." << endl;
                }
            } while (!isValidMobile(mobile));

            do {
                cout << "Enter Department: ";
                cin >> department;
                if (!isValidString(department)) {
                    cout << "Invalid department! It should contain only alphabets. Please try again." << endl;
                }
            } while (!isValidString(department));

            cout << "Enter Salary: ";
            cin >> salary;

            Employee emp(id, name, age, mobile, department, salary);
            empManager.addEmployee(emp);

        }
        else if (choice == 2) {
            empManager.displayAllEmployees();

        }
        else if (choice == 3) {
            string id;
            cout << "Enter Employee ID to delete: ";
            cin >> id;
            empManager.deleteEmployeeById(id);

        }
        else if (choice == 4) {
            string id;
            cout << "Enter Employee ID to update: ";
            cin >> id;
            empManager.updateEmployeeById(id);

        }
        else if (choice == 5) {
            cout << "Exiting Employee Management System." << endl;
            break;

        }
        else {
            cout << "Invalid choice! Please try again." << endl;
        }
    }

    return 0;
}
