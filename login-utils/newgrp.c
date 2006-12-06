/* setgrp.c - by Michael Haardt. Set the gid if possible */
/* Added a bit more error recovery/reporting - poe */
/* Vesa Roukonen added code for asking password */
/* Currently maintained at ftp://ftp.daimi.aau.dk/pub/linux/poe/ */

#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "pathnames.h"

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

static int
allow_setgid(struct passwd *pe, struct group *ge) 
{
    char **look;
    int notfound = 1;
	
    if (getuid() == 0) return TRUE;	/* root may do anything */

    look = ge->gr_mem;
    while (*look && (notfound = strcmp(*look++,pe->pw_name)));

    if(!notfound) return TRUE;		/* member of group => OK */

    /* Ask for password. Often there is no password in /etc/group, so
       contrary to login et al. we let an empty password mean the same
       as * in /etc/passwd */

    if(ge->gr_passwd && ge->gr_passwd[0] != 0) {
	if(strcmp(ge->gr_passwd, 
		  crypt(getpass("Password: "), ge->gr_passwd)) == 0) {
	    return TRUE;		/* password accepted */
	}
    }

    return FALSE;			/* default to denial */
}

int 
main(int argc, char *argv[])
{
    struct passwd *pw_entry;
    struct group *gr_entry;
    char *shell;
    
    if (!(pw_entry = getpwuid(getuid()))) {
	perror("newgrp: Who are you?");
	exit(1);
    }
    
    shell = (pw_entry->pw_shell[0] ? pw_entry->pw_shell : _PATH_BSHELL);
    
    if (argc < 2) {
	if(setgid(pw_entry->pw_gid) < 0) {
	    perror("newgrp: setgid");
	    exit(1);
	}
    } else {
	if (!(gr_entry = getgrnam(argv[1]))) {
	    perror("newgrp: No such group.");
	    exit(1);
	} else {
	    if(allow_setgid(pw_entry, gr_entry)) {
		if(setgid(gr_entry->gr_gid) < 0) {
		    perror("newgrp: setgid");
		    exit(1);
		}
	    } else {
		puts("newgrp: Permission denied");
		exit(1);
	    }
	}
    }

    if(setuid(getuid()) < 0) {
	perror("newgrp: setuid");
	exit(1);
    }

    fflush(stdout); fflush(stderr);
    execl(shell,shell,(char*)0);
    perror("No shell");
    fflush(stderr);
    exit(1);
}
