#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <setjmp.h>

static sigjmp_buf ToPrompt;
static volatile sig_atomic_t IndexJump = 0;

static void jumphd(int signalnum) {
 	if (!IndexJump) 
		return;
 	IndexJump = 0;
 	siglongjmp(ToPrompt, 1);
}

int main(){
	if (strcmp(str[0], "cd") == 0) {
   			chdir(str[1]);
   			system("pwd");

  			continue;
  		}
	if (strcmp(str[0], "exit") == 0) {

   			return 0;
  		}
}

//문자열 포인터배열로 정의
int ArgvPointer(const char *s, const char *delimiters, char ***argvp) {
 	int error;
 	int i;
 	int numtokens;
 	const char *snew;
 	char *t;

 	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
  		errno = EINVAL;
  		return -1;
 	}
 	*argvp = NULL;
 	snew = s + strspn(s, delimiters);
 	if ((t = (char *)malloc(strlen(snew) + 1)) == NULL)
 		 return -1;
 	strcpy(t, snew);
 	numtokens = 0;
 	if (strtok(t, delimiters) != NULL)
  		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++);

 	if ((*argvp = malloc((numtokens + 1) * sizeof(char *))) == NULL) {
  		error = errno;
  		free(t);
  		errno = error;
  		return -1;
 	}

 	if (numtokens == 0)
  		free(t);
 	else {
  		strcpy(t, snew);
  		**argvp = strtok(t, delimiters);
  		for (i = 1; i < numtokens; i++)
   			*((*argvp) + i) = strtok(NULL, delimiters);
 	}
 	*((*argvp) + numtokens) = NULL;
 	return numtokens;
}

int SetSignal(struct sigaction *def, sigset_t *mask, void(*handler)(int)) { //시그널 설정
 	struct sigaction catch;

 	catch.sa_handler = handler;
 	def->sa_handler = SIG_DFL;
 	catch.sa_flags = 0;
 	def->sa_flags = 0;
 	if ((sigemptyset(&(def->sa_mask)) == -1) || (sigemptyset(&(catch.sa_mask)) == -1) || (sigaddset(&(catch.sa_mask), SIGINT) == -1) || (sigaddset(&(catch.sa_mask), SIGQUIT) == -1) || (sigaction(SIGINT, &catch, NULL) == -1) || (sigaction(SIGQUIT, &catch, NULL) == -1) || (sigemptyset(mask) == -1) || (sigaddset(mask, SIGINT) == -1) || (sigaddset(mask, SIGQUIT) == -1))
  		return -1;
 	return 0;
}

//재지향 >< 탐색 구현

int FindRedirectIn(char *cmd) { // < 찾고 분석
 	int error;
 	int infd;
 	char *infile;

 	if ((infile = strchr(cmd, '<')) == NULL)
  		return 0;
 	*infile = 0;
 	infile = strtok(infile + 1, " \t");
 	if (infile == NULL)
  		return 0;
 	if ((infd = open(infile, O_RDONLY)) == -1)
  		return -1;
 	if (dup2(infd, STDIN_FILENO) == -1) {
  		error = errno;
  		close(infd);
  		errno = error;
  		return -1;
 	}
 	return close(infd);
}

int FindRedircetOut(char *cmd) { // > 찾고 분석
 	int error;
 	int outfd;
 	char *outfile;

 	if ((outfile = strchr(cmd, '>')) == NULL)
  		return 0;
 	*outfile = 0;
 	outfile = strtok(outfile + 1, " \t");
 	if (outfile == NULL)
  		return 0;
 	if ((outfd = open(outfile, O_WRONLY)) == -1)
  		return -1;
 	if (dup2(outfd, STDOUT_FILENO) == -1) {
  		error = errno;
  		close(outfd);
  		errno = error;
  		return -1;
 	}
 	return close(outfd);
}


void Command_exe(char *cmds) {
 	int child;
 	int count;
 	int fds[2];
 	int i;
 	char **pipelist;

 	count = ArgvPointer(cmds, "|", &pipelist);
 	if (count <= 0) {
  		fprintf(stderr, "Failed\n");
  		exit(1);
 	}
 	for (i = 0; i < count - 1; i++) {
  		if (pipe(fds) == -1)
   			perror_exit("Failed to create pipes");
  		else if ((child = fork()) == -1)
   			perror_exit("Failed to create process to run command");
  		else if (child) {
   			if (dup2(fds[1], STDOUT_FILENO) == -1)
    				perror_exit("Failed to connect pipeline");
   			if (close(fds[0]) || close(fds[1]))
    				perror_exit("Failed to close needed files");

   			FileRedirect(pipelist[i], i == 0, 0);
   			exit(1);
  		}
  		if (dup2(fds[0], STDIN_FILENO) == -1)
   			perror_exit("Failed to connect last component");
 		if (close(fds[0]) || close(fds[1]))
   			perror_exit("Failed to do final close");
 	}
 	FileRedirect(pipelist[i], i == 0, 1);
 	exit(1);

}

void FileRedirect(char *s, int in, int out) { //4번 및 파일 재지향
 	char **chargv;
 	char *pin;
 	char *pout;
 	int i, j;

 	if (in && ((pin = strchr(s, '<')) != NULL) && out && ((pout = strchr(s, '>')) != NULL) && (pin > pout)) {
  		if (FindRedirectIn(s) == -1) {
   			perror("Failed to redirect input");
   			return;
  		}
 	}

 	if (out && FindRedircetOut(s) == -1)
  		perror("Failed to redirect output");
 	else if (in && FindRedirectIn(s) == -1)
  		perror("failed to redirect input");
 	else if (ArgvPointer(s, " \t", &chargv) <= 0)
  		fprintf(stderr, "failed to parse command line\n");
 	else {

  		for (i = 0; chargv[i] != 0; i++) {
   			for (j = 0; chargv[i][j] != 0; j++) {
    				write(desc[1], &chargv[i][j], sizeof(char));
   			}
   			write(desc[1], " ", sizeof(char));
  		}
  		execvp(chargv[0], chargv);
  		perror("failed to execute command");

  		write(desc[1], "/5999", sizeof("/5999"));
 	}
 	exit(1);
}
