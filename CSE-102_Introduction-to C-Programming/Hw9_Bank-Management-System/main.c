#include <stdio.h>
#include <stdlib.h>
#include <string.h>



union Person
{
    char name[50];
    char address[50];
    int phone;
};

union Loan
{
    float amount;
    float interestRate;
    int period;
};

struct BankAccount
{
    union Person Customer;
    union Loan Loans[3];
};

// struct that using for register a user and read from file
typedef struct  {
    int ID;
    union Person name;
    union Person address;
    union Person phone;
}user_info;

// struct that using for a new user and read from file
typedef struct  {
    int ID;
    union Loan amount;
    union Loan interestRate;
    union Loan period;
}loan_info;

float calculateLoan(float amount, int period, float interestRate);

int take_all_customers(struct BankAccount *Customers, int *loan_count);

void listCustomers(struct BankAccount *Customers, int cc, int *loan_c);

void loan_deatils(loan_info ul);

void addCustomer(int cc);

void newLoan (struct BankAccount *Customers, int cc, int *loan_c);

void getReport(int cc);

int main() {
    int cas, customer_count, userid;
    int t = 0, p = 0, loan_count[50];
    char c;
    struct BankAccount Customers[50];

    while (1) {
        t = 0;
        while (!t){

            printf(
                    "\n\n====================================\n"
                    "Welcome to the Bank Managment System\n"
                    "====================================\n"
                    "   1. List All Customers\n"
                    "   2. Add new Customer\n"
                    "   3. New Loan Application \n"
                    "   4. Report Menu \n"
                    "   5. Exit.\n"
                    "------------------------------\n"
                    );
                printf("\nYOUR CHOICE: ");

            if((scanf("%d",&cas) != 1) || cas>5 || cas<1) {
                    printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
                    p = 1;
                    continue;
                }
            while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time
            if(p == 0) {t = 1; };
            p = 0;
        }

        switch(cas) { // chose apllier
        case 1: // List All Customers
            customer_count = take_all_customers(Customers, loan_count);
            listCustomers(Customers, customer_count, loan_count);

            break;
        case 2:  // Add new Customer
            customer_count = take_all_customers(Customers, loan_count);
            if (customer_count >= 50) printf("We have maximum amount of customers, Please try again later\n");
            else addCustomer (customer_count);
            break;
        case 3: // New Loan Application
            customer_count = take_all_customers(Customers, loan_count);
            newLoan(Customers, customer_count, loan_count);

            break;
        case 4:  // Report Menu
            customer_count = take_all_customers(Customers, loan_count);
            getReport(customer_count);

            break;

        case 5:
            printf("\n\n\t\tThank you for trying our software and we wish you a safe stay.\n\n\t\t\t\t\t--(^-^)--\n\n"); // ʕ•́ᴥ•̀ʔっ♡  ʕ•́ᴥ•̀ʔっ♡  ʕ•́ᴥ•̀ʔっ♡  ʕ•́ᴥ•̀ʔっ♡
            return 0;
        }
    }
    return 987;
}

// refund amount calculator 
float calculateLoan(float amount, int period, float interestRate) {
    if (period == 0) return amount; 
    return (1+interestRate)*calculateLoan(amount, period-1, interestRate);
}

int take_all_customers(struct BankAccount *Customers, int *loan_count) {
    FILE *F1, *F2;
    user_info info2;
    loan_info Linf;
    int i = 0, k;
    F2 = fopen("Loans.txt", "a");
    fclose(F2);
    memset(loan_count, 0, 50*sizeof(int));
    F1 = fopen("Customers.txt", "r");
    F2 = fopen("Loans.txt", "r");
    if(F1 == NULL) {
        printf("Upss, I think there is no customers\n");
        return 0;
    }
    while (fread(&info2, sizeof(user_info), 1, F1)) { // read user data from file
        strcpy(Customers[i].Customer.name, info2.name.name);
        rewind(F2);
        k = 0;
        while(fread(&Linf, sizeof(loan_info), 1, F2)) { // read loan data from file  
            if(Linf.ID == info2.ID) {
                Customers[i].Loans[k].amount = Linf.amount.amount ;
                k++;
            }
        }
        loan_count[i] = k;
        i++;
    }
    fclose(F1);
    fclose(F2);
    return i;
}

void listCustomers(struct BankAccount *Customers, int cc, int *loan_c) {
    int i, j;
    float top;
    for (i = 0; i < cc; i++) {
        top = 0.0;
        printf("\nCustomer Name = %s\nLoans = [", Customers[i].Customer.name);
        for (j = 0; j < loan_c[i]; j++) {
            top +=  Customers[i].Loans[j].amount;
            printf("%f", Customers[i].Loans[j].amount);
            if (j != loan_c[i] - 1) printf(" + ");
        }
        printf("] => %f\n", top);
    }
}

void addCustomer(int cc) {
    int a;
    user_info new_user;
    FILE *F1;
    int i = 0, k;
    char name[50], add[50];
    F1 = fopen("Customers.txt", "a");
    if (F1 == NULL) printf("I think there is no customer");

    new_user.ID = cc + 1;
    printf("New Customer's name: ");
    scanf("%[^\n]%*c", name);
    strcpy(new_user.name.name, name);
    printf("New Customer's Address: ");
    scanf("%[^\n]%*c", add);
    strcpy(new_user.address.address, add);
    printf("New Customer's phone: ");
    scanf("%d", &new_user.phone.phone);
    a = fwrite(&new_user, sizeof(user_info), 1, F1);  // write user data to file
    if(a>0) printf("\nUser succesfully added to system\n");
    fclose(F1);
}

void newLoan (struct BankAccount *Customers, int cc, int *loan_c) {
    int i;
    loan_info user_loan;
    FILE *F1;

    printf("Please enter User's ID for take Loan (for cancelling enter 50+ number): ");
    scanf("%d", &user_loan.ID);
    if (user_loan.ID > cc || user_loan.ID < 1) {
        printf("Invalid User ID, You are redirected to the menu...\n");
        return ;
    }
    if (loan_c[user_loan.ID-1] >= 3) {
        printf("This user have already 3 Loan, Please try diffrent user. You are redirected to the menu...\n");
        return ;
    }

    F1 = fopen("Loans.txt", "a");

    printf("Please enter the Loan amount you want: ");
    scanf("%f", &user_loan.amount.amount);
    printf("Please enter the Loan interest rate: ");
    scanf("%f", &user_loan.interestRate.interestRate);
    printf("Please enter the Loan period you want: ");
    scanf("%d", &user_loan.period.period);

    printf("\n");
    loan_deatils(user_loan);
    i = fwrite(&user_loan, sizeof(loan_info), 1, F1);  // write loan data to file
    fclose(F1);
    if(i>0) printf("\nLoan succesfully added to system\n");

}

void loan_deatils(loan_info ul) {
    int i;
    float amt, amtPm;
    amt = calculateLoan(ul.amount.amount, ul.period.period, ul.interestRate.interestRate);
    printf("    Total credit value = %f\n", amt);

    amtPm = amt/ul.period.period;
    for(i = 1; i <= ul.period.period; i++) {
        printf("    %d. Mounth Installment = %f\n", i, amtPm);
    }
}

void getReport(int cc) {
    FILE *F1, *F2;
    user_info info2;
    loan_info Linf;
    int i = 0, k, ID;

    printf("Enter User's ID for get report(for cancelling enter 50+ number): ");
    scanf("%d", &ID);
    if (ID > cc || ID < 1) {  
        printf("Invalid User ID, You are redirected to the menu...\n");
        return ;
    }

    F1 = fopen("Customers.txt", "r");
    F2 = fopen("Loans.txt", "r");
    if(F1 == NULL) {
        printf("Upss, I think there is no customers\n");
        return ;
    }
    while (fread(&info2, sizeof(user_info), 1, F1)) { // read user data from file  
        if (info2.ID == ID) {  // if wanted user is this
            printf("Customer ID = %d\n", info2.ID);
            printf("Customer Name = %s\n", info2.name.name);
            printf("Customer Phone = %d\n", info2.phone.phone);
            printf("Customer Address = %s\n", info2.address.address);
            printf("\nLOANs and DEATILs\n----------------\n\n");
            rewind(F2);
            k = 0;
            while(fread(&Linf, sizeof(loan_info), 1, F2)) { // read loan data from file  
                if(Linf.ID == info2.ID) { // print loan deatils 
                    loan_deatils(Linf);
                    printf("\n");
                }
            }
        }
    }
    fclose(F1);
    fclose(F2);
}
