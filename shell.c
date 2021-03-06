#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/* Found at > source: http://stanford.io/14ElVhr */
typedef struct node Node;
typedef struct stack Stack;
typedef struct fileinfo FileInfo;

struct fileinfo {
	char* name;
	char* path;
	char* pwd;
};


///////////////////////////////////////* Global Declarations *///////////////////////////////////////

static FileInfo **info = NULL;
static Stack stack;
static int numFiles = 0;
char currDirPath[100];// = "/home/umarali/Projects/Shell";
char currDirName[25];

/////////////////////////////////////* End Global Declarations */////////////////////////////////////


//////////////////////////////////* Helper Function Declarations *//////////////////////////////////

void findMyAbsolutePath();
char* findMyDirName(char* path);
char* strCat(char* str, const char* append);
void setCurrDirName();
void displayPwdProtectedFilesInfo();
int isPwdProtected(char* filename, char* filepath);
int validatePassword(char* pwd);

/////////////////////////////////* End Helper Function Declarations *////////////////////////////////


/////////////////////////////////////////////* Stack *///////////////////////////////////////////////

struct node {
	char *data;
	Node *next;
};

struct stack {
	Node* head;
};

Node* createNode(char* data) {
	Node* n = malloc(sizeof(Node*));
	n->data = malloc(sizeof(data));
	sprintf(n->data, "%s", data);
	//memccpy(n->data, data, '\0', sizeof(data));
	n->next = NULL;

	return n;
}

void push(char* data) {
	Node* n = createNode(data);

	if(!stack.head) {
		stack.head = n;
	}
	else {
		n->next = stack.head;
		stack.head = n;
	}
}

Node* pop() {
	Node* n;

	if(stack.head) {
		n = stack.head;
		stack.head = stack.head->next;
		return n;
	}
	else {
		printf("Error: Stack is Empty\n");
	}

	return NULL;
}

void display() {
	Node* n = stack.head;

	while(n) {
		printf("%s\n", n->data);
		n = n->next;
	}
}

////////////////////////////////////////////* End Stack *////////////////////////////////////////////


///////////////////////////////////////////* Core Methods *//////////////////////////////////////////

void cmdLS(int bool) {
	/* 1. ls - Show the list of files and directories in the current directory. */

	DIR *dir;
	struct dirent *dirent;
	dir = opendir(currDirPath);

	if(dir == NULL)
		printf("Error: Cannot Open Directory - %s\n", currDirPath);
	else {
		/* Invokes for password, to show password protected files */
		if(bool) {
			char* pwd = malloc(sizeof(char*));

			printf("Password: ");
			fgets(pwd, sizeof(pwd), stdin);

			if(validatePassword(pwd)) {
				while ((dirent = readdir(dir)) != NULL) {
					if(strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)
						printf("%s\n", dirent->d_name);
				}
			}

			free(pwd);
		}
		else {
			while ((dirent = readdir(dir)) != NULL) {
				if(!isPwdProtected(dirent->d_name, currDirPath) && strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)
					printf("%s\n", dirent->d_name);
			}
		}
	}
	closedir(dir);
}

void cmdCD(char* path) {
	/* 2. cd – change the current working directory to a specific folder. */

	if(!path){
		printf("Error: No path specified.\n");
		return;
	}

	DIR* currDir,
	   * absDir;
	struct dirent *dirent;
	char newPath[100];

	sprintf(newPath, "%s/%s", currDirPath, path);
	/* Check for current path */
	currDir = opendir(newPath);

	if(currDir == NULL) {
		/* Check for absolute path */
		absDir = opendir(path);

		if(absDir == NULL) {
			printf("Error: Invalid Directory - %s\n", path);
		}
		else {
			sprintf(currDirPath, "%s", path);
			/* Updates the directory name */
			setCurrDirName();
			closedir(absDir);
		}
	}
	else {
		sprintf(currDirPath, "%s/%s", currDirPath, path);
		/* Updates the directory name */
		setCurrDirName();
	}

	closedir(currDir);
}

void cmdPWD() {
	/* 3. pwd – print complete absolute path of current working directory. */
	
	printf("%s\n", currDirPath);
}

void cmdPUSH(char* path) {
	/* 4. push - Push the current directory on to the stack and move to the new directory. */

	if(!path){
		printf("Error: No path specified\n");
		return;
	}

	push(currDirPath);
	/* Changes the directory */
	cmdCD(path);
}

void cmdPOP() {
	/* 5. pop - Pop the directory from the stack and go to that directory. */

	Node* n = pop();

	if(n){
		/* Changes the directory */
		cmdCD(n->data);
		/* Frees memory */
		free(n->data);
		free(n);
	}
}

void cmdPrivate(char* filename) {
	DIR *dir;
	struct dirent *dirent;
	char* input;
	int found = 0;

	/* Checks if FileInfo already exists */
	if(!info) {
		info = malloc(sizeof(FileInfo*));
	}

	dir = opendir(currDirPath);

	if(dir == NULL)
		printf("Error: Cannot Open Directory - %s\n", currDirPath);
	else {
		while ((dirent = readdir(dir)) != NULL) {
			if(strcmp(dirent->d_name, filename) == 0 && strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0) {
				found = 1;
				input = malloc(sizeof(char*));
				
				printf("\nPassword: ");
				fgets(input, sizeof(input), stdin);
				
				info[numFiles] = malloc(sizeof(FileInfo));
				info[numFiles]->name = malloc(sizeof(char*));
				info[numFiles]->path = malloc(sizeof(char*));
				info[numFiles]->pwd = malloc(sizeof(char*));

				sprintf(info[numFiles]->name, "%s", dirent->d_name);
				sprintf(info[numFiles]->path, "%s", currDirPath);
				sprintf(info[numFiles]->pwd, "%s", input);

				numFiles++;
				free(input);

				break;
			}
		}

		if(!found) {
			printf("Error: Invalid directory or file name: %s\n", filename);
		}
	}
	closedir(dir);
}

/////////////////////////////////////////* End Core Methods *////////////////////////////////////////


//////////////////////////////////////////* Helper Methods */////////////////////////////////////////

void findMyAbsolutePath() {
	const char* up = "/..";
	char* path = ".",
		* name = findMyDirName(path),
		**arr;
	int limit = 10,
		i = 0;

	arr = malloc(sizeof(char*)*limit);

	for(i = 0; i < limit; i++)
		*(arr+i) = malloc(sizeof(char*));

	i = 0;
	//name = findMyDirName(path);
	//name[strlen(name) - 1] = '\0';

	do {
		path = strCat(path, up);
		//printf("%s\n", name);
		sprintf(*(arr+i), "%s", name);
		i++;
	} while((name = findMyDirName(path)) != NULL && i < limit);

	i--;

	int j = 0;
	strcpy(currDirPath, "");

	for(j = i; j >= 0; j--){
		char* ptr = strCat(currDirPath, strCat("/", (const char*) *(arr + j)));
		strcpy(currDirPath, ptr);
		free(ptr);
	}

	//name = findMyDirName(".");
	//sprintf(currDirPath, "%s%s", currDirPath, name);
	
	/* Frees the memory */
	for(i = 0; i < limit; i++)
		free(*(arr+i));
	free(arr);
	arr = NULL;
}

char* findMyDirName(char* path) {
	DIR * me,
		* parent;
	struct dirent *dirent;
	/* Declares a ino_t (file serial no.) type variable */
	ino_t sno;
	char* name = NULL;

	me = opendir(path);

	if(me == NULL)
		printf("Error: Cannot Open Directory - %s\n", path);
	else {
		while ((dirent = readdir(me)) != NULL) {
			if(strcmp(dirent->d_name, ".") == 0) {
				sno = dirent->d_ino;
				break;
			}
		}
		closedir(me);

		if(sno) {
			const char* up = "/..";
			char* ppath = strCat(path, up);
			parent = opendir(ppath);
			free(ppath);

			if(parent != NULL) {
				while ((dirent = readdir(parent)) != NULL) {
					if(	(sno == dirent->d_ino) &&
						(strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)) {
						name = dirent->d_name;
						break;
					}
				}
				closedir(parent);
			}
		}
	}

	return name;
}

char* strCat(char* str, const char* append) {
	char* new = malloc(sizeof(str) + sizeof(append));
	sprintf(new, "%s%s", str, append);

	return new;
}

int isPwdProtected(char* filename, char* filepath) {
	int i;

	for(i = 0; i < numFiles; i++) {
		if(strcmp(info[i]->name, filename) == 0) {// && strcmp(info[i]->path, filepath) == 0) {
			return 1;
		}
	}

	return 0;
}

int validatePassword(char* pwd) {
	int i;

	for(i = 0; i < numFiles; i++) {
		if(strcmp(info[i]->pwd, pwd) == 0 && strcmp(info[i]->path, currDirPath) == 0 ) {
			return 1;
		}
	}

	return 0;
}

void displayPwdProtectedFilesInfo() {
	int i = 0;

	for(i = 0; i < numFiles; i++) {
		printf("%s %s\n", info[i]->name, info[i]->path);
	}
}

void setCurrDirName() {
	const char* delim = "/";

	/* Makes a deep copy of currDirPath (Why? strtok() updates
	   the buffer which also modifies the original buffer) */
	char path[100];
	strcpy(path, currDirPath);

	char* tmp = strtok(path, delim);

	do {
		strcpy(currDirName, tmp);
	} while((tmp = strtok(NULL, delim)));
}

////////////////////////////////////////* End Helper Methods *///////////////////////////////////////


void main() {

	//printf("%s\n", findMyDirName("."));
	/* Sets absolute path of the current directory */
	findMyAbsolutePath();
	/* Sets the current directory name */
	setCurrDirName();

	printf("\n");
	printf("/////////////////////////////////////////////////////////////////////////////////\n");
	printf("//                            Primitive Linux Shell                            //\n");
	printf("/////////////////////////////////////////////////////////////////////////////////\n");

	const char* delim = " \n";
	char input[100];
	char* cmd = malloc(sizeof(char*));
	
	printf("Note: Enter <exit> to leave the shell.\n");

	/* Keeps looping until the user enters <exit> */
	while(1) {
		printf("\n125041 - %s> ", currDirName);
		/* Gets the input from input channel
		 * Replaced scanf with fgets > source: http://bit.ly/Sz6Cob
		 */
		fgets(input, sizeof(input), stdin);

		if(!input) break;

		/* Tokenizes command from input */
		sprintf(cmd, "%s", strtok(input, delim));
		
		if(strcmp(cmd, "ls") == 0) {
			int bool = 0;
			char* str = strtok(NULL, delim);

			if(str && strcmp(str, "-private") == 0) {
				bool = 1;
			}

			cmdLS(bool);
		}
		else if(strcmp(cmd, "cd") == 0) {
			cmdCD(strtok(NULL, delim));
		}
		else if(strcmp(cmd, "pwd") == 0) {
			cmdPWD();
		}
		else if(strcmp(cmd, "push") == 0) {
			cmdPUSH(strtok(NULL, delim));
		}
		else if(strcmp(cmd, "pop") == 0) {
			cmdPOP();
		}
		else if(strcmp(cmd, "private") == 0) {
			cmdPrivate(strtok(NULL, delim));
		}
		else if(strcmp(cmd, "exit") == 0) {
			break;
		}
		else {
			printf("Error: No such command found\n");
		}
	}

	//displayPwdProtectedFilesInfo();
	/* Frees memory */
	free(cmd);

	int i;

	for (i = 0; i < numFiles; i++)
		free(info[i]);
	free(info);

}


/*

TODO: Validations

*/