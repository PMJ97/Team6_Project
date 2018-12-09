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
