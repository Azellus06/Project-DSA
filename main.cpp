#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <filesystem>
#include <cstdlib>
#include <ctime>

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
    bool authenticate(char driveLetter, int &accNum, const string &enteredPin);

    // ----- For Users -----
    void checkBalance(int accNum);
    bool withdraw(int accNum, double amount);
    bool deposit(int accNum, double amount);
    bool fundTransfer(int fromAccNum, int toAccNum, double amount);
    bool changePin(int accNum, const string &oldPin, const string &newPin);
};

// ----- Operations -----
bool ATM::insertAccount(const Account &acc) // O(1) insertion
{
    Node *newNode = new Node(acc);
    newNode->next = head;
    head = newNode;

    return true;
}

Node *ATM::searchByAccountNumber(int accNum)
{
    Node *current = head;

    while (current)
    {
        if (current->data.accountNumber == accNum)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

void ATM::displayAll()
{ // for testing
}

int ATM::generateNextAccountNumber()
{
}

bool ATM::registerNewAccount(const Account &acc, char driveLetter)
{ // generateAccNumber, generateRandomShiftKey, encryption, insert sa list, tas write to card
}

void ATM::clearList()
{
    Node *ptr;

    while (head)
    {
        ptr = head;
        head = head->next;
        delete (ptr);
    }
}

// ----- Validations -----
bool ATM::validateDeposit(double amount)
{ // >= 5000
}

bool ATM::validateContactNumber(const string &num)
{
    if (num.length() != 11)
    {
        return false;
    }

    if (num[0] != '0' || num[1] != '9')
    {
        return false;
    }

    for (char c : num)
    {
        if (!isdigit(c))
        {
            return false;
        }
    }

    return true;
}

bool ATM::validateBirthday(const string &bday)
{
    int month, day, year;

    if (bday.length() != 10)
    {
        return false;
    }

    if (bday[2] != '/' || bday[5] != '/')
    {
        return false;
    }

    for (int i = 0; i < 10; i++)
    {
        if (i == 2 || i == 5) // skip slashes
        {
            continue;
        }

        if (!isdigit(bday[i]))
        {
            return false;
        }
    }

    month = stoi(bday.substr(0, 2));
    day = stoi(bday.substr(3, 2));
    year = stoi(bday.substr(6, 4));

    if (month < 1 || month > 12)
    {
        return false;
    }

    if (day < 1 || day > 31)
    {
        return false;
    }

    if (year < 1900 || year > 2025)
    {
        return false;
    }

    return true;
}

bool ATM::validateAmount(double amount, double currentBalance)
{
    return amount > 0 && amount <= currentBalance;
}

bool ATM::validatePin(const string &pin)
{ // max 6 digits, Enter if only 4
    if (pin.length() < 4 || pin.length() > 6)
    {
        return false;
    }

    for (char c : pin)
    {
        if (!isdigit(c))
        {
            return false;
        }
    }

    return true;
}

// ----- File Handling -----
bool ATM::saveToFile()
{
    ofstream outputFile(DATABASE);

    if (!outputFile.is_open())
    {
        return false;
    }

    Node *current = head;

    while (current != NULL)
    {
        outputFile << current->data.accountNumber << ","
                   << current->data.accountName << ","
                   << current->data.birthday << ","
                   << current->data.contactNumber << ","
                   << current->data.balance << ","
                   << current->data.pinShiftKey << ","
                   << current->data.encryptedPin << "\n";
        current = current->next;
    }

    outputFile.close();
    return true;
}

bool ATM::retrieveFromFile()
{
    ifstream inputFile(DATABASE);

    if (!inputFile.is_open())
    {
        return false;
    }

    clearList();

    string line;

    while (getline(inputFile, line))
    {
        if (line.empty())
            continue;

        stringstream ss(line);
        string data;
        Account acc;

        getline(ss, data, ',');
        acc.accountNumber = stoi(data);
        getline(ss, acc.accountName, ',');
        getline(ss, acc.birthday, ',');
        getline(ss, acc.contactNumber, ',');
        getline(ss, data, ',');
        acc.balance = stod(data);
        getline(ss, data, ',');
        acc.pinShiftKey = stoi(data);
        getline(ss, acc.encryptedPin, ',');

        insertAccount(acc);
    }

    inputFile.close();
    return true;
}

// ----- PIN encryption -----
string ATM::encryptPin(const string &rawPin, int shiftKey)
{
    string result = rawPin;

    for (char &c : result)
    {
        int digit = (c - '0' + shiftKey) % 10;
        c = '0' + digit;
    }

    return result;
}

string ATM::decryptPin(const string &encryptedPin, int shiftKey)
{
    string result = encryptedPin;

    for (char &c : result)
    {
        int digit = (c - '0' - shiftKey % 10 + 10) % 10;
        c = '0' + digit;
    }

    return result;
}

int ATM::generateRandomShiftKey()
{ // called once at registration
    return rand() % 9 + 1;
}

// ----- Flash Drive -----
bool ATM::detectFlashDrive(char &driveLetter)
{
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < 26; i++)
    {
        // check kung nakita yung drive
        if (drives & (1 << i))
        {
            char letter = 'A' + i;
            string path = string(1, letter) + ":\\";

            UINT type = GetDriveTypeA(path.c_str());

            // check kung usb yung drive na nakita
            if (type == DRIVE_REMOVABLE)
            {
                driveLetter = letter;
                return true;
            }
        }
    }

    return false; // di nakasaksak usb
}

string ATM::getMaskedPinInput()
{
    string pin = "";
    char ch;

    while (true)
    {
        ch = _getch();

        if (ch == '\r' || ch == '\n') // Pinindot enter
        {
            break;
        }
        else if (ch == '\b') // Pinindot backspace
        {
            if (!pin.empty())
            {
                pin.pop_back();
                cout << "\b \b";
            }
        }
        else
        {
            pin += ch;
            cout << '*';
        }
    }

    cout << endl;
    return pin;
}

bool ATM::writeToCard(char driveLetter, int accNum, const string &encryptedPin, int shiftKey)
{
    string path = string(1, driveLetter) + ":\\pin.code";

    ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    file << accNum << "," << encryptedPin << "," << shiftKey;

    file.close();
    return true;
}

bool ATM::readFromCard(char driveLetter, int &accNum, string &encryptedPin, int &shiftKey)
{
    string path = string(1, driveLetter) + ":\\pin.code";

    ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    string line;
    getline(file, line);
    file.close();

    stringstream ss(line);
    string data;

    if (!getline(ss, data, ','))
    {
        return false;
    }
    accNum = stoi(data);

    if (!getline(ss, data, ','))
    {
        return false;
    }
    encryptedPin = data;

    if (!getline(ss, data, ','))
    {
        return false;
    }
    shiftKey = stoi(data);

    return true;
}

bool ATM::authenticate(char driveLetter, int &accNum, const string &enteredPin)
{
    string storedEncryptedPin;
    int storedShiftKey;

    if (!readFromCard(driveLetter, accNum, storedEncryptedPin, storedShiftKey))
    {
        return false; // no card
    }

    string decrypted = decryptPin(storedEncryptedPin, storedShiftKey);

    return decrypted == enteredPin;
}

// ----- For Users -----
void ATM::checkBalance(int accNum)
{
    Node *accountNode = searchByAccountNumber(accNum);

    if (accountNode == NULL)
    {
        cout << "Account not found.\n";
        return;
    }

    cout << "Current Balance: " << accountNode->data.balance << endl;
}

bool ATM::withdraw(int accNum, double amount)
{
    Node *accountNode = searchByAccountNumber(accNum);

    if (accountNode == NULL)
    {
        return false;
    }

    if (!validateAmount(amount, accountNode->data.balance))
    {
        return false;
    }

    accountNode->data.balance -= amount;
    saveToFile();
    return true;
}

bool ATM::deposit(int accNum, double amount)
{
    if (amount <= 0)
    {
        return false;
    }

    Node *accountNode = searchByAccountNumber(accNum);

    if (accountNode == NULL)
    {
        return false;
    }

    accountNode->data.balance += amount;
    saveToFile();
    return true;
}

bool ATM::fundTransfer(int fromAccNum, int toAccNum, double amount)
{
    Node *fromNode = searchByAccountNumber(fromAccNum);
    Node *toNode = searchByAccountNumber(toAccNum);

    if (fromNode == NULL || toNode == NULL)
    {
        return false;
    }

    if (fromAccNum == toAccNum)
    {
        return false; // bawal transfer sa sarili
    }

    if (!validateAmount(amount, fromNode->data.balance))
    {
        return false;
    }

    fromNode->data.balance -= amount;
    toNode->data.balance += amount;

    saveToFile();
    return true;
}

bool ATM::changePin(int accNum, const string &oldPin, const string &newPin)
{
    Node *accountNode = searchByAccountNumber(accNum);

    if (accountNode == NULL)
    {
        return false;
    }

    string decryptedOld = decryptPin(accountNode->data.encryptedPin, accountNode->data.pinShiftKey);

    if (decryptedOld != oldPin)
    {
        return false; // wrong old PIN
    }

    if (!validatePin(newPin))
    {
        return false;
    }

    string newEncrypted = encryptPin(newPin, accountNode->data.pinShiftKey);
    accountNode->data.encryptedPin = newEncrypted;

    saveToFile();
    return true;
}