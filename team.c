static sigjmp_buf ToPrompt;
static volatile sig_atomic_t IndexJump = 0;

static void jumphd(int signalnum) {
 	if (!IndexJump) 
		return;
 	IndexJump = 0;
 	siglongjmp(ToPrompt, 1);
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

