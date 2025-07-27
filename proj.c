#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define DATA_FILE "atmdata.txt"
#define MAX_TRANSACTIONS 100
#define MAX_USERS 100
#define PIN_LENGTH 5
#define CARD_LENGTH 20

typedef struct {
    char type[20];
    double amount;
    char datetime[20];
} Transaction;

typedef struct {
    char cardNumber[CARD_LENGTH];
    char pin[PIN_LENGTH];
    double balance;
    Transaction transactions[MAX_TRANSACTIONS];
    int transactionCount;
} Account;

Account accounts[MAX_USERS];
int userCount = 0;

// Function declarations
int loadAccounts();
int saveAccounts();
int findAccountIndexByCard(const char* cardNumber);
int login();
int registerNewUser();
void menu(int accountIndex);
void balanceInquiry(Account *account);
void cashWithdrawal(Account *account);
void depositAmount(Account *account);
void pinChange(Account *account);
void transactionHistory(Account *account);
void recordTransaction(Account *account, const char* type, double amount);
void getDateTime(char *buffer, int len);

int main() {
    if (!loadAccounts()) {
        // No data file found, start fresh with zero users
        userCount = 0;
    }

    printf("=== Welcome to ATM Simulation System ===\n");
    printf("Are you a new user? (1 = Yes, 0 = No): ");
    int isNewUser = -1;
    while (isNewUser != 0 && isNewUser != 1) {
        scanf("%d", &isNewUser);
        if (isNewUser != 0 && isNewUser != 1) {
            printf("Invalid input. Enter 1 for Yes or 0 for No: ");
        }
    }

    int accountIndex = -1;
    if (isNewUser == 1) {
        accountIndex = registerNewUser();
        if (accountIndex == -1) {
            printf("Registration failed. Exiting.\n");
            return 1;
        }
    } else {
        accountIndex = login();
        if (accountIndex == -1) {
            printf("Login failed. Exiting.\n");
            return 1;
        }
    }

    menu(accountIndex);

    return 0;
}

int loadAccounts() {
    FILE *file = fopen(DATA_FILE, "r");
    if (!file) return 0;

    userCount = 0;
    while (userCount < MAX_USERS) {
        Account *acc = &accounts[userCount];
        int res = fscanf(file, "%19s %4s %lf %d\n", acc->cardNumber, acc->pin, &acc->balance, &acc->transactionCount);
        if (res != 4) break;

        for (int i = 0; i < acc->transactionCount; i++) {
            fscanf(file, "%19s %lf %19s\n", acc->transactions[i].type, &acc->transactions[i].amount, acc->transactions[i].datetime);
        }
        userCount++;
    }

    fclose(file);
    return 1;
}

int saveAccounts() {
    FILE *file = fopen(DATA_FILE, "w");
    if (!file) {
        printf("Error saving data.\n");
        return 0;
    }

    for (int i = 0; i < userCount; i++) {
        Account *acc = &accounts[i];
        fprintf(file, "%s %s %.2lf %d\n", acc->cardNumber, acc->pin, acc->balance, acc->transactionCount);
        for (int j = 0; j < acc->transactionCount; j++) {
            fprintf(file, "%s %.2lf %s\n", acc->transactions[j].type, acc->transactions[j].amount, acc->transactions[j].datetime);
        }
    }

    fclose(file);
    return 1;
}

int findAccountIndexByCard(const char* cardNumber) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(accounts[i].cardNumber, cardNumber) == 0)
            return i;
    }
    return -1;
}

int login() {
    char cardNumber[CARD_LENGTH];
    char pin[PIN_LENGTH];
    int attempts = 0;
    printf("Enter your card number: ");
    scanf("%19s", cardNumber);

    int idx = findAccountIndexByCard(cardNumber);
    if (idx == -1) {
        printf("Card number not found.\n");
        return -1;
    }

    while (attempts < 3) {
        printf("Enter your PIN: ");
        scanf("%4s", pin);
        if (strcmp(pin, accounts[idx].pin) == 0) {
            printf("Login successful!\n\n");
            return idx;
        } else {
            printf("Incorrect PIN. Try again.\n");
        }
        attempts++;
    }

    return -1;
}

int registerNewUser() {
    if (userCount >= MAX_USERS) {
        printf("User limit reached. Cannot register new user.\n");
        return -1;
    }

    char cardNumber[CARD_LENGTH];
    char pin1[PIN_LENGTH];
    char pin2[PIN_LENGTH];
    double initialBalance;

    printf("Enter new card number (max 19 chars): ");
    scanf("%19s", cardNumber);

    if (findAccountIndexByCard(cardNumber) != -1) {
        printf("Card number already registered.\n");
        return -1;
    }

    printf("Enter new PIN (4 digits): ");
    scanf("%4s", pin1);

    printf("Confirm new PIN: ");
    scanf("%4s", pin2);

    if (strcmp(pin1, pin2) != 0) {
        printf("PINs do not match.\n");
        return -1;
    }

    printf("Enter initial balance: $");
    scanf("%lf", &initialBalance);
    if (initialBalance < 0) {
        printf("Invalid initial balance.\n");
        return -1;
    }

    Account *newUser = &accounts[userCount];
    strcpy(newUser->cardNumber, cardNumber);
    strcpy(newUser->pin, pin1);
    newUser->balance = initialBalance;
    newUser->transactionCount = 0;

    // Record initial balance as transaction
    recordTransaction(newUser, "AccountCreated", initialBalance);

    userCount++;

    if (!saveAccounts()) {
        printf("Failed to save new user data.\n");
        userCount--;
        return -1;
    }

    printf("Registration successful! You can now login.\n\n");
    return findAccountIndexByCard(cardNumber);
}

void menu(int accountIndex) {
    int choice;
    Account *account = &accounts[accountIndex];
    do {
        printf("\n--- ATM Menu ---\n");
        printf("1. Balance Inquiry\n");
        printf("2. Deposit\n");
        printf("3. Cash Withdrawal\n");
        printf("4. PIN Change\n");
        printf("5. Transaction History\n");
        printf("6. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        getchar(); // consume newline

        switch (choice) {
            case 1:
                balanceInquiry(account);
                break;
            case 2:
                depositAmount(account);
                break;
            case 3:
                cashWithdrawal(account);
                break;
            case 4:
                pinChange(account);
                break;
            case 5:
                transactionHistory(account);
                break;
            case 6:
                printf("Thank you for using the ATM Simulation System.\n");
                saveAccounts();
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 6);
}

void balanceInquiry(Account *account) {
    printf("Your current balance is: $%.2lf\n", account->balance);
    recordTransaction(account, "BalanceInquiry", 0.0);
    saveAccounts();
}

void depositAmount(Account *account) {
    double amount;
    printf("Enter amount to deposit: $");
    scanf("%lf", &amount);
    getchar(); // consume newline

    if (amount <= 0) {
        printf("Invalid amount.\n");
        return;
    }

    account->balance += amount;
    printf("Amount successfully deposited: $%.2lf\n", amount);
    recordTransaction(account, "Deposit", amount);
    saveAccounts();
}

void cashWithdrawal(Account *account) {
    double amount;
    printf("Enter amount to withdraw: $");
    scanf("%lf", &amount);
    getchar(); // consume newline

    if (amount <= 0) {
        printf("Invalid amount.\n");
        return;
    }
    if (amount > account->balance) {
        printf("Insufficient balance.\n");
        recordTransaction(account, "WithdrawalFailed", amount);
    } else {
        account->balance -= amount;
        printf("Please take your cash: $%.2lf\n", amount);
        recordTransaction(account, "Withdrawal", amount);
        saveAccounts();
    }
}

void pinChange(Account *account) {
    char oldPin[PIN_LENGTH];
    char newPin[PIN_LENGTH];
    char confirmPin[PIN_LENGTH];

    printf("Enter old PIN: ");
    scanf("%4s", oldPin);
    getchar(); // consume newline

    if (strcmp(oldPin, account->pin) != 0) {
        printf("Old PIN incorrect.\n");
        return;
    }

    printf("Enter new PIN (4 digits): ");
    scanf("%4s", newPin);
    getchar();

    printf("Confirm new PIN: ");
    scanf("%4s", confirmPin);
    getchar();

    if (strcmp(newPin, confirmPin) != 0) {
        printf("New PIN and confirmation do not match.\n");
        return;
    }

    strcpy(account->pin, newPin);
    printf("PIN successfully changed.\n");
    recordTransaction(account, "PINChange", 0.0);
    saveAccounts();
}

void transactionHistory(Account *account) {
    if (account->transactionCount == 0) {
        printf("No transactions found.\n");
        return;
    }
    printf("\n--- Transaction History ---\n");
    for (int i = 0; i < account->transactionCount; i++) {
        printf("%d. %s: $%.2lf on %s\n", i+1, account->transactions[i].type, account->transactions[i].amount, account->transactions[i].datetime);
    }
}

void recordTransaction(Account *account, const char* type, double amount) {
    if (account->transactionCount >= MAX_TRANSACTIONS) {
        // Remove oldest transaction to make room
        memmove(account->transactions, account->transactions + 1, sizeof(Transaction) * (MAX_TRANSACTIONS - 1));
        account->transactionCount--;
    }

    strcpy(account->transactions[account->transactionCount].type, type);
    account->transactions[account->transactionCount].amount = amount;
    getDateTime(account->transactions[account->transactionCount].datetime, sizeof(account->transactions[account->transactionCount].datetime));
    account->transactionCount++;
}

void getDateTime(char *buffer, int len) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, len, "%04d-%02d-%02d %02d:%02d:%02d",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
}

