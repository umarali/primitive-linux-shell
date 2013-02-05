#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>


typedef struct node Node;
typedef struct stack Stack;
typedef struct fileinfo FileInfo;

struct fileinfo {
	char* name;
	char* path;
	char pwd[6];
};

//////////////////////////////////////* Function Declarations *//////////////////////////////////////

void findMyAbsolutePath();
char* findMyDirName(char* path);
char* strCat(char* str, const char* append);
void setCurrDirName();
int isPwdProtected(char* filename, char* filepath);

////////////////////////////////////* End Function Declarations *////////////////////////////////////


///////////////////////////////////////* Global Declarations *///////////////////////////////////////

static FileInfo **info = NULL;
static Stack stack;
static int numFiles = 0;
char currDirPath[100] = "/home/umarali/Projects/Shell";
char currDirName[25];

/////////////////////////////////////* End Global Declarations */////////////////////////////////////


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

void cmdLS() {
	/* 1. ls - Show the list of files and directories in the current directory. */

	DIR *dir;
	struct dirent *dirent;
	dir = opendir(currDirPath);

	if(dir == NULL)
		printf("Cannot Open Directory - %s\n", currDirPath);
	else {
		while ((dirent = readdir(dir)) != NULL) {
			if(!isPwdProtected(dirent->d_name, dirent->d_name) && strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)
				printf("%s\n", dirent->d_name);
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

	DIR *dir;
	struct dirent *dirent;
	dir = opendir(path);

	if(dir == NULL)
		printf("Error: Invalid Directory - %s\n", path);
	else {
		sprintf(currDirPath, "%s", path);
		/* Updates the directory name */
		setCurrDirName();
	}

	closedir(dir);
}

void cmdPWD() {
	/* 3. pwd – print complete absolute path of current working directory. */
	
	printf("%s\n", currDirPath);
}

void cmdPUSH(char* path) {
	/* 4. push - Push the current directory on to the stack and move to the new directory. */

	//printf("push %s and cd %s\n", currDirPath, path);
	push(currDirPath);
	/* Changes the directory */
	cmdCD(path);
}

void cmdPOP() {
	/* 5. pop - Pop the directory from the stack and go to that directory. */

	Node* n = pop();
	//printf("pop and cd %s\n", n->data);
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
				info[numFiles]->name = dirent->d_name;
				info[numFiles]->path = currDirPath;
				sprintf(info[numFiles]->pwd, "%s", input);

				printf("%s is now protected.\n", filename);

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
	name[strlen(name) - 1] = '\0';

	do {
		path = strCat(path, up);
		sprintf(*(arr+i), "%s", name);
		i++;
	} while((name = findMyDirName(path)) != NULL && i < limit);

	i--;

	int j = 0;
	strcpy(currDirPath, "");

	for(j = i; j >= 0; j--)
		strcpy(currDirPath, strCat(currDirPath, strCat("/", (const char*) *(arr + j))));

	/* Frees the memory */
	//for(i = 0; i < limit; i++)
		//free(*(arr+i));
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
		if(strcmp(info[i]->name, filename) == 0) {
			return 1;
		}
	}

	return 0;
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

	/* Sets absolute path of the current directory */
	//findMyAbsolutePath();
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

		/* Tokenizes command from input */
		sprintf(cmd, "%s", strtok(input, delim));
		
		if(strcmp(cmd, "ls") == 0) {
			cmdLS();
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
	}

}


/*

TODO: Validations

*/