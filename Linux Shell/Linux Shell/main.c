#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h> 
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>



/*
  Function Declarations for builtin shell commands:
 */
void recursive(const char *name, int indent);

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_cat(char **args);
int lsh_ls(int argc,char **args);
int lsh_cp(int argc,char **args);
void lsh_rm(int argc,char **args);
void lsh_touch(int argc,char **args);
void lsh_rmdir(int argc,char **args);
int lsh_mkdir(const char *path);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "cat",
  "ls",
  "cp",
  "rm",
  "touch",
  "rmdir",
  "mkdir"
};

int(*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_cat,
  &lsh_ls,
  &lsh_cp,
  &lsh_rm,
  &lsh_touch,
  &lsh_rmdir,
  &lsh_mkdir
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cat(char **args)
{
	int fp;
	char ch[99];
	int op;

	fp = open(args[1], O_RDONLY);

	while (op = read(fp, ch, 99)) {

		write(1, ch, op);
	}
	printf("\n");
	close(fp);

}

int lsh_ls(int argc,char **args)
{
	struct dirent **name;
	int n;
	//printf("%s",argv[1]);
	if (argc == 1)
	{
		n = scandir(".", &name, NULL, alphasort);
		while (n--)
		{
			printf("%s\n", name[n]->d_name);
			free(name[n]);
		}
		free(name);
	}
	else if (argc == 2 || args[1] == "-R") {
		//printf("sd\n");
		recursive(".", 0);
		// return 0;

	}
	exit(EXIT_SUCCESS);
}


void recursive(const char *name, int indent)
{
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(name)))
		return;

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR) {
			char path[1024];
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			sprintf(path, "%s", entry->d_name);
			printf("%*s[[%s]]\n", indent, "", entry->d_name);
			recursive(path, indent + 3);
		}
		else {
			printf("%*s-> %s\n", indent, "", entry->d_name);
		}
	}
	closedir(dir);
}

#define SIZE 1024

int lsh_cp(int argc, char** args)
{
	int Source, Destination, ReadBuffer, WriteBuffer;
	char *buff[SIZE];

	if (argc != 3 || args[1] == "--help")
	{
		printf("\nUsage: cpcmd source destination\n");
		exit(EXIT_FAILURE);
	}

	//printf("%s\n",argv[1]);
	//printf("%s\n",argv[2]);
	//printf("%d\n",argv[3]);
	Source = open(args[1], O_RDONLY);

	if (Source == -1)
	{
		printf("\nError opening file %s errno = %d\n", args[1], errno);
		exit(EXIT_FAILURE);
	}

	Destination = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if (Destination == -1)
	{
		printf("\nError opening file %s errno = %d\n", args[2], errno);
		exit(EXIT_FAILURE);
	}


	while ((ReadBuffer = read(Source, buff, SIZE)) > 0)
	{
		if (write(Destination, buff, ReadBuffer) != ReadBuffer)
			printf("\nError in writing data to \n");
	}


	if (close(Source) == -1)
		printf("\nError in closing file\n");

	if (close(Destination) == -1)
		printf("\nError in closing file\n");

}

void lsh_rm(int argc, char** args) {
	if (argc != 2 || args[1] == "--help")
	{
		printf("\nusage: rm FileTodelete\n");
	}
	int status;
	status = remove(args[1]);
	if (status == 0)
	{
		printf("successfull\n");
	}
	else
	{
		printf("Unsuccessfull\n");
	}
}

int retvalue;
void lsh_touch(int argc, char** args) {
	if (argc != 2 || args[1] == "--help") {
		printf("Usage::touch textfile To modify\n");
	}
	retvalue = utime(args[1], NULL);
	if (retvalue == 0) {
		printf("Timestamp modified\n");
	}
}

int lsh_mkdir(const char *path)
{

	const size_t len = strlen(path);
	char _path[PATH_MAX];
	char *p;

	errno = 0;

	/* Copy string so its mutable */
	if (len > sizeof(_path) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++) {
		if (*p == '/') {
			/* Temporarily truncate */
			*p = '\0';

			if (mkdir(_path, S_IRWXU) != 0) {
				if (errno != EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if (mkdir(_path, S_IRWXU) != 0) {
		if (errno != EEXIST)
			return -1;
	}

	return 0;
}

void lsh_rmdir(int argc, char** args) {


	if (argc != 2 || args[1] == "--help")
	{
		printf("\nusage: rm File To delete\n");
		// break;
	}
	char *cmd = "mkdir";
	char *argv[3];
	argv[0] = "mkdir";
	argv[1] = argv[1];
	argv[2] = NULL;

	execvp(cmd, args);
}

int lsh_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	}
	else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}

int lsh_help(char **args)
{
	int i;
	printf("Ivan Odak's LSH\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < lsh_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

int lsh_exit(char **args)
{
	return 0;
}

int lsh_launch(char **args)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {
		// Error forking
		perror("lsh");
	}
	else {
		// Parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int lsh_execute(char **args)
{
	int i;

	if (args[0] == NULL) {
		// An empty command was entered.
		return 1;
	}

	for (i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024

char *lsh_read_line(void)
{
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// Read a character
		c = getchar();

		if (c == EOF) {
			exit(EXIT_SUCCESS);
		}
		else if (c == '\n') {
			buffer[position] = '\0';
			return buffer;
		}
		else {
			buffer[position] = c;
		}
		position++;

		// If we have exceeded the buffer, reallocate.
		if (position >= bufsize) {
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line)
{
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token, **tokens_backup;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += LSH_TOK_BUFSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				free(tokens_backup);
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}


void lsh_loop(void)
{
	char *line;
	char **args;
	int status;

	do {
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char **argv)
{
	// Load config files, if any.

	// Run command loop.
	lsh_loop();

	// Perform any shutdown/cleanup.

	return EXIT_SUCCESS;
}

