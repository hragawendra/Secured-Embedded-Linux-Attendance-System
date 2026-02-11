/*
   Linux UART Attendance System 
   ----------------------------
   MCU  -> Linux : ID,DATE,DAY,TIME,AM/PM\n
   Linux-> MCU   : '1' = VALID , '0' = INVALID
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

/* ================= CONFIGURATION ================= */

#define UART_DEVICE     "/dev/ttyUSB0"
#define BAUDRATE        B9600
#define MASTER_ID       "4900C8FDDCA0"  
#define DEFAULT_PASS    "admin@123" 
#define EMPLOYEE_FILE   "employee.dat"
#define PASSWORD_FILE   "config.bin" 
#define ANONYMOUS_LOGIN "Anonymous_Login.log"
#define ENCRYPT_KEY     0x5A

/* ================= GLOBALS ================= */

typedef struct EMP
{
	char id[15];
	char name[30];
	unsigned char status;   // 0 = OUT , 1 = IN
	time_t last_in;
	struct EMP *next;
} EMP;

EMP *head = NULL;
int emp_count = 0;
int master_add_mode = 0;
int master_remove_mode = 0;

char master_password[50]; // Global password variable

/* Security Globals */
static int wrong_attempts = 0;
static int locked_day_of_year = -1;

const char *month_names[] = {"", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"};

/* ================= HELPER FUNCTIONS ================= */

EMP *find_employee(const char *id)
{
	EMP *ptr = head;
	while (ptr != NULL) 
	{
		if (strcmp(ptr->id, id) == 0) 
		{
			return ptr;
		}
		ptr = ptr->next;
	}
	return NULL;
}

void log_attendance(const EMP *e, const char *date, const char *day, const char *time_str, const char *ampm)
{
	int mon = atoi(&date[3]);
	if(mon < 1 || mon > 12) 
	{
		mon = 0;
	}

	FILE *fp = fopen(month_names[mon], "a");
	if (fp) 
	{
		fprintf(fp, "ID: %s | Name: %-35s | Status:%-4s | Date:%s Day:%s Time:%s %s\n", e->id, e->name, (e->status == 1) ? "IN" : "OUT", date, day, time_str, ampm);
		fclose(fp);
	}
	else
	{
		printf("\033[31m[ERROR] Opening log file.\033[0m\n");
	}
}

/* ================= SECURITY LOGIC ================= */

void encrypt_decrypt(char *input, char *output)
{
	int i, len = strlen(input);
	for(i = 0; i < len; i++)
	{
		output[i] = input[i] ^ ENCRYPT_KEY; 
	}
	output[i] = '\0';
}

void save_password_to_file(const char *plain_pass)
{
	FILE *fp = fopen(PASSWORD_FILE, "w");
	if (fp == NULL)
	{
		printf("\033[31m[ERROR] Could not save password config!\033[0m\n");
		return;
	}

	char encrypted[50]; 
	char temp_pass[50];
	strcpy(temp_pass, plain_pass);

	encrypt_decrypt(temp_pass, encrypted);
	fprintf(fp, "%s", encrypted);
	fclose(fp);
	printf("\033[32m[SUCCESS] Password Saved.\033[0m\n");
}

void load_master_password(void)
{
	FILE *fp = fopen(PASSWORD_FILE, "r");
	if (fp == NULL) 
	{
		printf("\033[33m[ERROR] \"Config.bin\" not found.\nCreating default password.\033[0m\n");
		strcpy(master_password, DEFAULT_PASS);
		save_password_to_file(DEFAULT_PASS);
	}
	else
	{
		char encrypted[50];
		if (fscanf(fp, "%s", encrypted) == 1)
		{
			encrypt_decrypt(encrypted, master_password);
		}
		else
		{
			strcpy(master_password, DEFAULT_PASS);
		}
		fclose(fp);
	}
}

/* Function to read password silently (sudo style) */
void get_silent_input(char *buffer, int max_len)
{
	struct termios old_props, new_props;

	tcgetattr(STDIN_FILENO, &old_props);
	new_props = old_props;
	new_props.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_props);

	scanf("%49s", buffer);
	while((getchar()) != '\n'); 

	tcsetattr(STDIN_FILENO, TCSANOW, &old_props);
	printf("\n"); 
}

void change_password_logic(void)
{
	char new_pass[50];
	char confirm_pass[50];

	printf("\n--- CHANGE PASSWORD ---\n");
	printf("Enter New Password: ");
	get_silent_input(new_pass, 50);

	printf("Confirm New Password: ");
	get_silent_input(confirm_pass, 50);

	if (strcmp(new_pass, confirm_pass) == 0)
	{
		strcpy(master_password, new_pass);
		save_password_to_file(new_pass);
		printf("\033[32m[SUCCESS] Password Changed Successfully!\033[0m\n");
	}
	else
	{
		printf("\033[31m[ERROR] Passwords do not match.\033[0m\n");
	}
}

/* ================= DATABASE LOGIC ================= */

void load_database(void)
{
	FILE *fp = fopen(EMPLOYEE_FILE, "r");
	if (fp == NULL) 
	{
		printf("\033[33m[ERROR] Employee database file not found.\033[0m\n");
		return;
	}

	while (1)
	{
		EMP *new_node = malloc(sizeof(EMP));
		if (new_node == NULL) 
		{
			printf("\033[31m[CRITICAL] Memory allocation failed\033[0m\n");
			break;
		}

		if (fscanf(fp, "%s %[^\n]", new_node->id, new_node->name) != 2)
		{
			free(new_node);
			break;
		}

		new_node->status = 0; 
		new_node->last_in = 0;
		new_node->next = NULL;

		if (head == NULL)
		{
			head = new_node;
		}
		else
		{
			EMP *last = head;
			while (last->next) last = last->next;
			last->next = new_node;
		}
		emp_count++;
	}
	fclose(fp);
	printf("[INFO] Database Loaded. %d Records found.\n", emp_count);
}

void add_employee(const char *id)
{
	if(find_employee(id))
	{
		printf("\033[31m[ERROR] ID already in Database\033[0m\n");
		master_add_mode = 0;
		return;
	}

	EMP *new_node = malloc(sizeof(EMP));
	if (new_node == NULL) 
	{
		printf("\033[31m[CRITICAL] Memory allocation failed\033[0m\n");
		master_add_mode = 0;
		return;
	}

	printf("New ID Scanned: %s\n", id);
	strcpy(new_node->id, id);
	printf("Enter Name: ");
	scanf(" %[^\n]", new_node->name);
	while((getchar()) != '\n'); 

	new_node->status=0;
	new_node->last_in=0;
	new_node->next = 0;

	if(head == NULL) 
	{
		head = new_node;
	}
	else
	{    
		EMP *last = head;
		while(last->next) last=last->next;
		last->next = new_node;
	}

	emp_count++;

	FILE *fp = fopen(EMPLOYEE_FILE, "a+");
	if (fp) 
	{
		fprintf(fp, "%s %s\n", new_node->id, new_node->name);
		fclose(fp);
		printf("\033[32m[SUCCESS] Employee Added and Database Updated...\033[0m\n");
		master_add_mode = 0;
	}
	else 
	{
		master_add_mode = 0;
		printf("[ERROR] Opening Database file.\n");
	}
}

void remove_employee(const char *id)
{
	if (head == NULL) 
	{
		printf("\033[31m[ERROR] Database is empty.\033[0m\n");
		return;
	}

	EMP *curr = head;
	EMP *prev = NULL;
	int found = 0;

	while (curr != NULL) 
	{
		if (strcmp(curr->id, id) == 0) 
		{
			if (prev == NULL)
			{
				head = curr->next;
			}
			else
			{
				prev->next = curr->next;
			}
			free(curr);
			found = 1;
			break;
		}
		prev = curr;
		curr = curr->next;
	}

	if (!found) 
	{
		printf("\033[31m[ERROR] ID not found in Database.\033[0m\n");
		return;
	}

	emp_count--;

	FILE *fp = fopen(EMPLOYEE_FILE, "w");
	if (fp) 
	{
		curr = head;
		while (curr) 
		{
			fprintf(fp, "%s %s\n", curr->id, curr->name);
			curr = curr->next;
		}
		fclose(fp);
		master_remove_mode = 0;
		printf("\033[32m[SUCCESS] Employee Removed and Database Updated.\033[0m\n");
	}
	else 
	{
		master_remove_mode = 0;
		printf("\033[31m[ERROR] Writing to database file.\033[0m\n");
	}
}

void list_employees(void)
{
	if (head == NULL) 
	{
		printf("\033[31m[ERROR] Database is empty.\033[0m\n");
		return;
	}

	printf("\n--- REGISTERED EMPLOYEES ---\n");
	printf("--------------------------------------------------------\n");

	EMP *ptr = head;
	while(ptr != NULL)
	{
		printf("%-15s %-35s %s\n", ptr->id, ptr->name, (ptr->status == 1) ? "IN" : "OUT");
		ptr = ptr->next;
	}

	printf("--------------------------------------------------------\n");
	printf("Total Records: %d\n\n", emp_count);
}

/* ================= MASTER MODE LOGIC ================= */

void handle_master_mode(int serial_port)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int current_day_val = t->tm_yday;

	if (locked_day_of_year != -1 && current_day_val != locked_day_of_year) 
	{
		wrong_attempts = 0;
		locked_day_of_year = -1;
		printf("\033[32m[SYSTEM INFO] Security Lock Reset.\033[0m\n");
	}

	if (wrong_attempts >= 3) 
	{
		printf("\n\033[1;41mSYSTEM LOCKED: Contact Network Admin.\033[0m\n");
		char reply = '!'; 
		write(serial_port, &reply, 1);
		return;
	}

	char reply = 'M';
	write(serial_port, &reply, 1);

	char input_pass[50];
	printf("\nENTER ADMIN PASSWORD: ");
	get_silent_input(input_pass, 50);

	if (strcmp(input_pass, master_password) == 0) 
	{
		wrong_attempts = 0;
		printf("\033[1;32mACCESS GRANTED\033[0m\n");

		int choice;
		printf("1. Add Employee\n2. Remove Employee\n3. List of all Employees\n4. Change Password\nChoice: ");
		scanf("%d", &choice);
		while((getchar()) != '\n'); 

		if (choice == 1) 
		{
			master_add_mode = 1;
			printf("\033[33m[MASTER MODE] ADD MODE ENABLED. Scan new card now.\033[0m\n");
		}
		else if (choice == 2) 
		{
			master_remove_mode = 1;
			printf("\033[33m[MASTER MODE] REMOVE MODE ENABLED. Scan new card now.\033[0m\n");
		}
		else if (choice == 3)
		{
			list_employees();
		}
		else if (choice == 4)
		{
			change_password_logic();
		}
		else
		{
			printf("\033[31m[ERROR] Invalid Choice\033[0m\n");
		}
	}
	else 
	{
		wrong_attempts++;
		printf("\033[1;31mWRONG PASSWORD! Attempts left: %d\033[0m\n", 3 - wrong_attempts);
		if (wrong_attempts >= 3) 
		{
			locked_day_of_year = current_day_val;
			printf("\033[1;41mSYSTEM LOCKED: Contact Network Admin.\033[0m\n");
		}
	}
}

/* ================= UART ================= */

int uart_init(const char *dev)
{
	int serial_port = open(dev, O_RDWR | O_NOCTTY);
	if (serial_port < 0)
	{
		perror("\033[31m[CRITICAL] Serial Port ERROR: UART open /dev/ttyUSB0\033[0m\n");
		return -1;
	}

	struct termios tty;
	tcgetattr(serial_port, &tty);

	cfsetispeed(&tty, BAUDRATE);
	cfsetospeed(&tty, BAUDRATE);

	tty.c_cflag = CS8 | CREAD | CLOCAL;
	tty.c_iflag = 0;
	tty.c_oflag = 0;
	tty.c_lflag = 0;

	tty.c_cc[VMIN]  = 1;
	tty.c_cc[VTIME] = 10;

	tcsetattr(serial_port, TCSANOW, &tty);
	return serial_port;
}

/* ================= MAIN LOOP ================= */

int main(void)
{
	load_database();
	load_master_password(); 

	int serial_port = uart_init(UART_DEVICE);
	if (serial_port < 0)
	{
		return -1;
	}

	char rx_buf[256];
	int idx = 0;
	char ch;

	printf("System Ready. Scan the cards...\n");

	while (1)
	{
		int n = read(serial_port, &ch, 1);

		if(n > 0)
		{
			if (ch == '\n' || ch == '\r')
			{
				if (idx > 0)
				{
					rx_buf[idx] = '\0'; 

					char *id    = strtok(rx_buf, ",");
					char *date  = strtok(NULL, ",");
					char *day   = strtok(NULL, ",");
					char *time_s= strtok(NULL, ",");
					char *ampm  = strtok(NULL, ",");

					if (id && date && day && time_s && ampm)
					{
						if (strcmp(id, MASTER_ID) == 0)
						{
							handle_master_mode(serial_port);
							idx = 0; 
							memset(rx_buf, 0, sizeof(rx_buf));
						}
						else if (master_add_mode == 1)
						{
							char reply = 'A';
							write(serial_port, &reply, 1);
							add_employee(id);
						}
						else if(master_remove_mode == 1)
						{
							char reply = 'R';
							write(serial_port, &reply, 1);
							remove_employee(id);
						}
						else
						{
							EMP *e = find_employee(id);
							char reply;

							if (!e) 
							{
								reply = '0';
								write(serial_port, &reply, 1);
								printf("\033[1;31mUnknown ID: %s\033[0m\n", id);

								FILE *fp = fopen(ANONYMOUS_LOGIN, "a+");
								if (fp) 
								{
									fprintf(fp, "ID:%s Date:%s Day:%s Time:%s %s\n", id, date, day, time_s, ampm);
									fclose(fp);
								}
							} 
							else 
							{
								reply = '1';
								write(serial_port, &reply, 1);

								if (e->status == 0) 
								{
									e->status = 1; 
									e->last_in = time(NULL); 
									printf("\033[32m%s LOGGED IN\033[0m\n", e->name);
								}
								else
								{
									e->status = 0; 
									printf("\033[33m%s LOGGED OUT\033[0m\n", e->name);
								}
								log_attendance(e, date, day, time_s, ampm);
							}
						}
					}
					idx = 0;
					memset(rx_buf, 0, sizeof(rx_buf));
				}
			}
			else
			{
				if (idx < sizeof(rx_buf) - 1)
				{
					rx_buf[idx++] = ch;
				}
			}
		}
	}
	close(serial_port);
	return 0;
}

