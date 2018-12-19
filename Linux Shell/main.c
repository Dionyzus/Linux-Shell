/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Thursday,  8 January 2015

  @brief        LSH (Libstephen SHell)

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h>


/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_touch(char **args);
int lsh_ls(char **args);
int lsh_mkdir(char **args);
int lsh_cp(char **args);
int lsh_cat(char **args);
int lsh_rm(char **args);
int lsh_mv(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "touch",
  "ls",
  "mkdir",
  "cp",
  "cat",
  "rm",
  "mv"
};

int(*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_touch,
  &lsh_ls,
  &lsh_mkdir,
  &lsh_cp,
  &lsh_cat,
  &lsh_rm,
  &lsh_mv
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */

static int filter(const struct dirent *unused) {
	return 1;
}

void print_error(char *this) {
	
	puts("ERROR: ");
	if (*this == "cat")
	{
		fprintf(stderr, "%s cannot concat given arguments\n",
			this,strerror(errno));
	}
	if (*this == "cp")
	{
		fprintf(stderr, "%s cannot copy given arguments\n", this, strerror(errno));
	}
	if (*this == "mkdir")
	{
		fprintf(stderr, "%s cannot make directory\n",
			this,strerror(errno));
	}
	if (*this == "ls")
	{
		fprintf(stderr, "%s cannot list from given directory\n", this,strerror(errno));
	}
	if (*this == "rm")
	{
		fprintf(stderr, "%s: could not delete file\n",
			this, strerror(errno));
	}
	if (*this == "mv")
	{
		fprintf(stderr, "%s cannot move given arguments\n",
			this, strerror(errno));
	}

	exit(EXIT_FAILURE);
}

void print_usage(char *this) {
	puts("ERROR: ");
	if (*this == "cat")
	{
		fprintf(stderr, "Usage: %s [filename]", this);
	}
	if (*this == "cp")
	{
		fprintf(stderr, "Usage: %s [filename] [new filename]\n", this);
	}
	if (*this == "mkdir")
	{
		fprintf(stderr, "Usage: %s [dir_name]", this);
	}
	if (*this == "ls")
	{
		fprintf(stderr, "Usage: %s [directory]\n", this);
	}
	if (*this == "rm")
	{
		fprintf(stderr, "Usage: %s [filename]\n", this);
	}
	if (*this == "mv")
	{
		fprintf(stderr, "Usage: %s [old filename] [new filename]\n", this);
	}

	exit(EXIT_FAILURE);
}

int lsh_mv(char **args)
{
	errno = 0;
	FILE *fpr, *fpw;
	char ch;

	if (args[2]==NULL) {
		print_usage(args[0]);
	}
	else if (rename(args[1], args[2]) == -1) {
		print_error(args[0]);
	}
	return 1;
}

int lsh_rm(char **args) {
	errno = 0;

	if (args[1] != NULL) {
		if (remove(args[1])) {
			print_error(args[0]);
		}
	}
	else {
		print_usage(args[0]);
	}

	return 1;
}

int lsh_cat(char **args) {
	errno = 0;
	FILE *fp;
	int maxline = 1024;
	char line[maxline];

	if (args[1] != NULL) {
		if ((fp = fopen(args[1], "rb")) == NULL) {
			print_error(args[0]);
		}
	}
	else {
		print_usage(args[0]);
	}

	while (fgets(line, maxline, fp)) {
		printf("%s", line);
	}

	fclose(fp);
	return 1;
}

int lsh_cp(char **args) {
	errno = 0;
	FILE *fpr, *fpw;
	char ch;

	if (args[2] == NULL) {
		print_usage(args[0]);
	}
	if ((fpr = fopen(args[1], "rb")) == NULL) {
		print_error(args[0]);
	}
	if ((fpw = fopen(args[2], "rb")) != NULL) {
		errno = EEXIST;
		print_error(args[0]);
	}
	if ((fpw = fopen(args[2], "wb")) == NULL) {
		print_error(args[0]);
	}

	while ((ch = getc(fpr)) != EOF) {
		putc(ch, fpw);
	}

	fclose(fpr);
	fclose(fpw);

	return 1;
}

int lsh_mkdir(char **args) {
	errno = 0;

	if (args[1] != NULL) {
		if (mkdir(args[1], (S_IRWXG | S_IRWXU))) {
			print_error(args[0]);
		}
	}
	else {
		print_usage(args[0]);
	}

	return 1;
}

int lsh_ls(char **args) {
	errno = 0;
	struct dirent **contents;
	int content_count;

	if (args[1] == NULL) {
		if ((content_count = scandir("./", &contents, filter, alphasort)) < 0) {
			print_error(args[0]);
		}
	}
	else if (args[1] != NULL) {
		if ((content_count = scandir(args[1], &contents, filter, alphasort)) < 0) {
			print_error(args[0]);
		}
	}

	int i;
	for (i = 0; i < content_count; i++) {
		puts(contents[i]->d_name);
	}

	return 1;
}

int lsh_touch(char **args) {
	FILE *new;

	if (args[1] != NULL) {
		if (fopen(args[1], "r") != NULL) {
			puts("ERROR: File already exists");
		}
		else if ((new = fopen(args[1], "w")) == NULL) {
			puts("ERROR");
		}
		fclose(new);
	}
	else {
		puts("ERROR:");
		puts("Usage: touch [file name]");
	}

	return 1;
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

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
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

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
	return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
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

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
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
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
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
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
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

/**
   @brief Loop getting input and executing it.
 */
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

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
	// Load config files, if any.

	// Run command loop.
	lsh_loop();

	// Perform any shutdown/cleanup.

	return EXIT_SUCCESS;
}

