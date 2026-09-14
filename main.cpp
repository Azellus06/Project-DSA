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

struct Account{
    int accountNumber; // 5-digit, auto-generated, start at 10001
    string accountName;
    string birthday; // MM/DD/YYY
    string contactNumber;
    double balance;      // initial deposit, min 5000
    int pinShiftKey;     // random key used for Caesar on this account's PIN, hindi siya fixed shift
    string encryptedPin; // PIN after Caesar applied
};
struct Node{
    Account data;
    Node *next;
    Node(Account data){
        this->data = data;
        this->next = NULL;
    }
};
class ATM{
private:
    Node *head;
public:
    ATM(){
        head = NULL;
    }
    ~ATM(){
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
    bool validateDeposit(double amount);          // >= 5000
    bool validateAccountName(const string &name); // bawal empty, no commas
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
    bool changePin(int accNum, const string &oldPin, const string &newPin, char driveLetter);
};

// ===================== main() and menu flows =====================
bool isValidWholeNumber(const string &str){
    if (str.empty() || str.length() > 9) return false;

    for (char c : str) if (c < '0' || c > '9') return false;

    return true;
}
bool isValidDecimalNumber(const string &str){
    if (str.empty() || str.length() > 15) return false;

    bool decimalSeen = false;
    int digitCount = 0;

    for (char c : str){
        if (c == '.' && !decimalSeen){
            decimalSeen = true;
            continue;
        }

        if (c < '0' || c > '9') return false;

        digitCount++;
    }

    return digitCount > 0;
}
void transactionMenu(ATM &atm, int accNum, char driveLetter){
    bool loggedIn = true;

    while (loggedIn){
        cout << "\n--- Transaction Menu (Account #" << accNum << ") ---\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Fund Transfer\n";
        cout << "5. Change PIN\n";
        cout << "6. Logout\n";
        cout << "Choice: ";

        string choiceStr;
        getline(cin, choiceStr);

        if (!isValidWholeNumber(choiceStr)){
            cout << "Invalid choice.\n";
            continue;
        }

        int choice = stoi(choiceStr);
        string line;
        double amount;

        switch (choice){
        case 1:
            atm.checkBalance(accNum);
            system("pause");
            system("cls");
            break;
        case 2:
            cout << "Enter deposit amount: ";
            getline(cin, line);

            if (!isValidDecimalNumber(line)){
                cout << "Invalid amount.\n";
                break;
            }

            amount = stod(line);

            if (atm.deposit(accNum, amount)) cout << "Deposit successful.\n";
            else cout << "Deposit failed. Amount must be greater than 0.\n";
            system("pause");
            system("cls");
            break;
        case 3:
            cout << "Enter withdrawal amount: ";
            getline(cin, line);

            if (!isValidDecimalNumber(line)){
                cout << "Invalid amount.\n";
                break;
            }

            amount = stod(line);

            if (atm.withdraw(accNum, amount)) cout << "Withdrawal successful.\n";
            else cout << "Withdrawal failed. Check your amount and balance.\n";
            system("pause");
            system("cls");
            break;
        case 4:{
            cout << "Enter recipient account number: ";
            string toStr;
            getline(cin, toStr);

            if (!isValidWholeNumber(toStr)){
                cout << "Invalid account number.\n";
                break;
            }

            int toAccNum = stoi(toStr);

            cout << "Enter amount to transfer: ";
            getline(cin, line);

            if (!isValidDecimalNumber(line)){
                cout << "Invalid amount.\n";
                break;
            }

            amount = stod(line);

            if (atm.fundTransfer(accNum, toAccNum, amount)) cout << "Transfer successful.\n";
            else cout << "Transfer failed. Check the recipient account and amount.\n";
            system("pause");
            system("cls");
            break;
        }
        case 5:{
            cout << "Enter current PIN: ";
            string oldPin = atm.getMaskedPinInput();

            cout << "Enter new PIN (4 to 6 digits): ";
            string newPin = atm.getMaskedPinInput();

            if (atm.changePin(accNum, oldPin, newPin, driveLetter)) cout << "PIN changed successfully.\n";
            else cout << "PIN change failed. Check your current PIN and the new PIN format.\n";
            system("pause");
            system("cls");
            break;
        }
        case 6:
            cout << "Logging out...\n";
            loggedIn = false;
            break;
        default:
            cout << "Invalid choice.\n";
            break;
        }
    }
}
void registerFlow(ATM &atm){
    char driveLetter;
    Account acc;
    string line;
    string pin;

    cout << "\n--- Register New Account ---\n";

    if (!atm.detectFlashDrive(driveLetter)){
        cout << "No USB flash drive detected. Please insert your flash drive and try again.\n";
        return;
    }

    while (true){
        cout << "Enter full name: ";
        getline(cin, acc.accountName);
        if (atm.validateAccountName(acc.accountName)) break;
        cout << "Invalid name. Names cannot be empty or contain a comma.\n";
    }

    while (true){
        cout << "Enter birthday (MM/DD/YYYY): ";
        getline(cin, acc.birthday);
        if (atm.validateBirthday(acc.birthday)) break;
        cout << "Invalid birthday. Use MM/DD/YYYY and a real calendar date\n";
    }

    while (true){
        cout << "Enter contact number (11 digits, starts with 09): ";
        getline(cin, acc.contactNumber);
        if (atm.validateContactNumber(acc.contactNumber)) break;
        cout << "Invalid contact number. It must be 11 digits and start with 09.\n";
    }

    while (true){
        cout << "Enter initial deposit (minimum PHP 5000): ";
        getline(cin, line);

        if (!isValidDecimalNumber(line)){
            cout << "Please enter a valid number.\n";
            continue;
        }

        acc.balance = stod(line);

        if (atm.validateDeposit(acc.balance)) break;
        cout << "Initial deposit must be at least PHP 5000.\n";
    }

    while (true){
        cout << "Create your PIN (4 to 6 digits): ";
        pin = atm.getMaskedPinInput();
        if (atm.validatePin(pin)) break;
        cout << "Invalid PIN. It must be 4 to 6 digits, numbers only.\n";
    }

    acc.encryptedPin = pin;

    if (atm.registerNewAccount(acc, driveLetter)) cout << "\nAccount registered successfully!\n";
    else cout << "\nRegistration failed. Please check your flash drive and try again.\n";
}
void loginFlow(ATM &atm){
    char driveLetter;

    cout << "\n--- Login ---\n";

    if (!atm.detectFlashDrive(driveLetter)){
        cout << "No USB flash drive detected. Please insert your flash drive and try again.\n";
        return;
    }

    cout << "Enter PIN: ";
    string pin = atm.getMaskedPinInput();
    int accNum;

    if (!atm.authenticate(driveLetter, accNum, pin)){
        cout << "Login failed. Wrong PIN or unrecognized card.\n";
        return;
    }

    cout << "\nLogin successful. Welcome!\n";
    transactionMenu(atm, accNum, driveLetter);
}
int main(){
    srand(static_cast<unsigned int>(time(0)));
    ATM atm;
    atm.retrieveFromFile();
    bool running = true;

    while (running){
        cout << "\n===== Welcome to the ATM =====\n";
        cout << "1. Register New Account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Choice: ";

        string choiceStr;
        getline(cin, choiceStr);

        if (!isValidWholeNumber(choiceStr)){
            cout << "Invalid choice.\n";
            continue;
        }

        int choice = stoi(choiceStr);

        switch (choice){
        case 1: registerFlow(atm); break;
        case 2: loginFlow(atm); break;
        case 3:
            cout << "Thank you for using the ATM. Goodbye!\n";
            running = false;
            break;
        default:
            cout << "Invalid choice.\n";
            break;
        }
    }

    return 0;
}

// ----- Operations -----
bool ATM::insertAccount(const Account &acc){ // O(1) insertion
    Node *newNode = new Node(acc);
    newNode->next = head;
    head = newNode;

    return true;
}
Node *ATM::searchByAccountNumber(int accNum){
    Node *current = head;

    while (current){
        if (current->data.accountNumber == accNum) return current;
        current = current->next;
    }

    return NULL;
}
void ATM::displayAll(){ // for testing
    if (!head){
        cout << "No accounts found." << endl;
        return;
    }

    Node *current = head;

    while (current){
        cout << "Account Number: " << current->data.accountNumber << endl;
        cout << "Name: " << current->data.accountName << endl;
        cout << "Birthday: " << current->data.birthday << endl;
        cout << "Contact: " << current->data.contactNumber << endl;
        cout << "Balance: " << current->data.balance << endl;
        cout << "Shift Key: " << current->data.pinShiftKey << endl;
        cout << "Encrypted PIN: " << current->data.encryptedPin << endl;
        cout << "-----------------------------" << endl;

        current = current->next;
    }
}
int ATM::generateNextAccountNumber(){
    if (!head) return 10001;

    int maxNum = 10001;
    Node *current = head;

    while (current){
        if (current->data.accountNumber > maxNum) maxNum = current->data.accountNumber;
        current = current->next;
    }

    return maxNum + 1;
}
bool ATM::registerNewAccount(const Account &acc, char driveLetter){ // generateAccNumber, generateRandomShiftKey, encryption, insert sa list, tas write to card
    string rawPin = acc.encryptedPin;
    Account newAcc = acc; // copy so we can fill in the generated fields

    if (!validatePin(rawPin)) return false;
    if (!validateAccountName(acc.accountName)) return false;

    newAcc.accountNumber = generateNextAccountNumber();
    newAcc.pinShiftKey = generateRandomShiftKey();
    newAcc.encryptedPin = encryptPin(rawPin, newAcc.pinShiftKey);

    if (!writeToCard(driveLetter, newAcc.accountNumber, newAcc.encryptedPin, newAcc.pinShiftKey)) return false;
    if (!insertAccount(newAcc)) return false;

    saveToFile();
    return true;
}
void ATM::clearList(){
    Node *ptr;

    while (head){
        ptr = head;
        head = head->next;
        delete (ptr);
    }
}

// ----- Validations -----
bool ATM::validateDeposit(double amount){ return amount >= 5000; }
bool ATM::validateAccountName(const string &name){
    if (name.empty()) return false;

    for (char c : name) if (c == ',') return false;

    return true;
}
bool ATM::validateContactNumber(const string &num){
    if (num.length() != 11) return false;
    if (num[0] != '0' || num[1] != '9') return false;

    for (char c : num) if (!isdigit(c)) return false;

    return true;
}
bool ATM::validateBirthday(const string &bday){
    if (bday.length() != 10) return false;
    if (bday[2] != '/' || bday[5] != '/') return false;

    for (int i = 0; i < 10; i++){
        if (i == 2 || i == 5) continue; // skip slashes
        if (!isdigit(bday[i])) return false;
    }

    int month = stoi(bday.substr(0, 2));
    int day = stoi(bday.substr(3, 2));
    int year = stoi(bday.substr(6, 4));
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[month - 1];
    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    time_t t = time(0);
    tm *now = localtime(&t);
    int currentYear = now->tm_year + 1900; // updated current year

    if (month < 1 || month > 12) return false;
    if (year < 1900 || year > currentYear) return false;
    if (month == 2 && isLeapYear) maxDay = 29;
    if (day < 1 || day > maxDay) return false;

    return true;
}
bool ATM::validateAmount(double amount, double currentBalance){ return amount > 0 && amount <= currentBalance; }
bool ATM::validatePin(const string &pin)
{ // max 6 digits, Enter if only 4
    if (pin.length() < 4 || pin.length() > 6) return false;

    for (char c : pin) if (!isdigit(c)) return false;

    return true;
}

// ----- File Handling -----
bool ATM::saveToFile(){
    ofstream outputFile(DATABASE);
    Node *current = head;

    if (!outputFile.is_open()) return false;    

    while (current){
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
bool ATM::retrieveFromFile(){
    ifstream inputFile(DATABASE);
    string line;

    if (!inputFile.is_open()) return false;

    clearList();

    while (getline(inputFile, line)){
        if (line.empty()) continue;

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
string ATM::encryptPin(const string &rawPin, int shiftKey){
    string result = rawPin;

    for (char &c : result){
        int digit = (c - '0' + shiftKey) % 10;
        c = '0' + digit;
    }

    return result;
}
string ATM::decryptPin(const string &encryptedPin, int shiftKey){
    string result = encryptedPin;

    for (char &c : result){
        int digit = (c - '0' - shiftKey % 10 + 10) % 10;
        c = '0' + digit;
    }

    return result;
}
int ATM::generateRandomShiftKey(){ return rand() % 9 + 1; } // called once at registration

// ----- Flash Drive -----
bool ATM::detectFlashDrive(char &driveLetter){
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < 26; i++){
        // check kung nakita yung drive
        if (drives & (1 << i)){
            char letter = 'A' + i;
            string path = string(1, letter) + ":\\";
            UINT type = GetDriveTypeA(path.c_str());

            // check kung usb yung drive na nakita
            if (type == DRIVE_REMOVABLE){
                driveLetter = letter;
                return true;
            }
        }
    }

    return false; // di nakasaksak usb
}
string ATM::getMaskedPinInput(){
    string pin = "";
    char ch;

    while (true)
    {
        ch = _getch();

        if (ch == '\r' || ch == '\n') break; // Pinindot enter
        else if (ch == '\b'){// Pinindot backspace
            if (!pin.empty()){
                pin.pop_back();
                cout << "\b \b";
            }
        }else{
            pin += ch;
            cout << '*';
        }
    }

    cout << endl;
    return pin;
}
bool ATM::writeToCard(char driveLetter, int accNum, const string &encryptedPin, int shiftKey){
    string path = string(1, driveLetter) + ":\\pin.code";

    ofstream file(path);
    if (!file.is_open()) return false;

    file << accNum << "," << encryptedPin << "," << shiftKey;

    file.close();
    return true;
}
bool ATM::readFromCard(char driveLetter, int &accNum, string &encryptedPin, int &shiftKey){
    string path = string(1, driveLetter) + ":\\pin.code";

    ifstream file(path);
    if (!file.is_open()) return false;

    string line;
    getline(file, line);
    file.close();

    stringstream ss(line);
    string data;

    if (!getline(ss, data, ',')) return false;
    accNum = stoi(data);
    if (!getline(ss, data, ',')) return false;
    encryptedPin = data;
    if (!getline(ss, data, ',')) return false;
    shiftKey = stoi(data);

    return true;
}
bool ATM::authenticate(char driveLetter, int &accNum, const string &enteredPin){
    string storedEncryptedPin;
    int storedShiftKey;

    if (!readFromCard(driveLetter, accNum, storedEncryptedPin, storedShiftKey)) return false; // no card
    if (!searchByAccountNumber(accNum)) return false; // unrecognized card, no matching account sa database

    string decrypted = decryptPin(storedEncryptedPin, storedShiftKey);
    return decrypted == enteredPin;
}

// ----- For Users -----
void ATM::checkBalance(int accNum){
    Node *accountNode = searchByAccountNumber(accNum);

    if (!accountNode)
    {
        cout << "Account not found.\n";
        return;
    }

    cout << "Current Balance: " << accountNode->data.balance << endl;
}
bool ATM::withdraw(int accNum, double amount){
    Node *accountNode = searchByAccountNumber(accNum);

    if (!accountNode) return false;
    if (!validateAmount(amount, accountNode->data.balance)) return false;

    accountNode->data.balance -= amount;
    saveToFile();
    return true;
}
bool ATM::deposit(int accNum, double amount){
    Node *accountNode = searchByAccountNumber(accNum);

    if (amount <= 0) return false;
    if (!accountNode) return false;

    accountNode->data.balance += amount;
    saveToFile();
    return true;
}
bool ATM::fundTransfer(int fromAccNum, int toAccNum, double amount){
    Node *fromNode = searchByAccountNumber(fromAccNum);
    Node *toNode = searchByAccountNumber(toAccNum);

    if (!fromNode || !toNode) return false;
    if (fromAccNum == toAccNum) return false; // bawal transfer sa sarili
    if (!validateAmount(amount, fromNode->data.balance)) return false;

    fromNode->data.balance -= amount;
    toNode->data.balance += amount;

    saveToFile();
    return true;
}
bool ATM::changePin(int accNum, const string &oldPin, const string &newPin, char driveLetter){
    Node *accountNode = searchByAccountNumber(accNum);

    if (!accountNode) return false;

    string decryptedOld = decryptPin(accountNode->data.encryptedPin, accountNode->data.pinShiftKey);

    if (decryptedOld != oldPin) return false; // wrong old PIN
    if (!validatePin(newPin)) return false;

    string newEncrypted = encryptPin(newPin, accountNode->data.pinShiftKey);

    if (!writeToCard(driveLetter, accNum, newEncrypted, accountNode->data.pinShiftKey)) return false; // card write failed, don't update the database

    accountNode->data.encryptedPin = newEncrypted;

    saveToFile();
    return true;
}