#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <filesystem>

using namespace std;
const string DATABASE = "database.csv";

struct Account
{
    int accountNumber; // 5-digit, auto-generated, start at 10001
    string accountName;
    string birthday; // MM/DD/YYY
    string contactNumber;
    double balance;      // initial deposit, min 5000
    int pinShiftKey;     // random key used for Caesar on this account's PIN, hindi siya fixed shift
    string encryptedPin; // PIN after Caesar applied
};

struct Node
{
    Account data;
    Node *next;
    Node(Account data)
    {
        this->data = data;
        this->next = NULL;
    }
};

class ATM
{
private:
    Node *head;

public:
    ATM()
    {
        head = NULL;
    }
    ~ATM()
    {
        clearList();
    };

    // ----- Operations -----
    bool insertAccount(const Account &acc);
    Node *searchByAccountNumber(int accNum);
    void displayAll(); // for testing
    int generateNextAccountNumber();
    bool registerNewAccount(const Account &acc, char driveLetter); // generateAccNumber, generateRandomShiftKey, encryption, insert sa list, tas write to card
    void clearList();

    // ----- Validations -----
    bool validateDeposit(double amount); // >= 5000
    bool validateContactNumber(const string &num);
    bool validateBirthday(const string &bday);                 // format check
    bool validateAmount(double amount, double currentBalance); // pang withdraw, bawal 0 at overdraw
    bool validatePin(const string &pin);                       // max 6 digits, Enter if only 4

    // ----- File Handling -----
    bool saveToFile(); // saves every transaction
    bool retrieveFromFile();

    // ----- PIN encryption -----
    string encryptPin(const string &rawPin, int shiftKey);
    string decryptPin(const string &encryptedPin, int shiftKey);
    int generateRandomShiftKey(); // called once at registration

    // ----- Flash Drive -----
    bool detectFlashDrive(char &driveLetter);
    string getMaskedPinInput();
    bool writeToCard(char driveLetter, int accNum, const string &encryptedPin, int shiftKey);
    bool readFromCard(char driveLetter, int &accNum, string &encryptedPin, int &shiftKey);
    bool authenticate(char driveLetter, int accNum, const string &enteredPin);

    // ----- For Users -----
    void checkBalance(int accNum);
    bool withdraw(int accNum, double amount);
    bool deposit(int accNum, double amount);
    bool fundTransfer(int fromAccNum, int toAccNum, double amount);
    bool changePin(int accNum, const string &oldPin, const string &newPin);
};

// ----- Operations -----
bool insertAccount(const Account &acc)
{
}

Node *searchByAccountNumber(int accNum)
{
}

void displayAll()
{ // for testing
}

int generateNextAccountNumber()
{
}

bool registerNewAccount(const Account &acc, char driveLetter)
{ // generateAccNumber, generateRandomShiftKey, encryption, insert sa list, tas write to card
}

void clearList()
{
}

// ----- Validations -----
bool validateDeposit(double amount)
{ // >= 5000
}

bool validateContactNumber(const string &num)
{
}

bool validateBirthday(const string &bday)
{ // format check
}

bool validateAmount(double amount, double currentBalance)
{ // pang withdraw, bawal 0 at overdraw
}

bool validatePin(const string &pin)
{ // max 6 digits, Enter if only 4
}

// ----- File Handling -----
bool saveToFile()
{ // saves every transaction
}

bool retrieveFromFile()
{
}

// ----- PIN encryption -----
string encryptPin(const string &rawPin, int shiftKey)
{
}

string decryptPin(const string &encryptedPin, int shiftKey)
{
}

int generateRandomShiftKey()
{ // called once at registration
}

// ----- Flash Drive -----
bool detectFlashDrive(char &driveLetter)
{
}

string getMaskedPinInput()
{
}

bool writeToCard(char driveLetter, int accNum, const string &encryptedPin, int shiftKey)
{
}

bool readFromCard(char driveLetter, int &accNum, string &encryptedPin, int &shiftKey)
{
}

bool authenticate(char driveLetter, int accNum, const string &enteredPin)
{
}

// ----- For Users -----
void checkBalance(int accNum)
{
}

bool withdraw(int accNum, double amount)
{
}

bool deposit(int accNum, double amount)
{
}

bool fundTransfer(int fromAccNum, int toAccNum, double amount)
{
}

bool changePin(int accNum, const string &oldPin, const string &newPin)
{
}
