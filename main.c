#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>


/*
    COURTNEY MCGILL HW02- Worked with Parker Reynolds
    We couldn't get stage 2 to work fully but think we have the right idea for it. It seems to only take in the last line of the file... and it still doesnt run it properlly if that last file does create a valid command. Part one should still work!!
*/

char** getinput();
int mode_command(char **cmd, int *par_mode);


int main(int argc, char **argv) {
   FILE *datafile = NULL;
   char filestream[2000000];
    /* find out how we got invoked and deal with it */
    switch (argc) {
	case 1:
            /* only one program argument (the program name) */ 
            /* just equate NULL with our datafile */
            datafile = NULL;        
            break;
        case 2:
            /* two arguments: program name and input file */
            /* open the file, assign to datafile */
            datafile = fopen(argv[1], "r");
            if (datafile == NULL) {
                printf("Unable to open file %s: %s\n", argv[1], strerror(errno));
                exit(-1);
            }
            break;
        default:
            /* more than two arguments?  throw hands up in resignation */
            //usage(argv[0]);
	    ;
    }
    int b=1;
    int par_mode = 0;
    int new_mode = 0;
    int num_children = 0;
    int mode_changed = 0;
    int childrv = 0;
    int toexit = 1;
    while(b){
	char** cmd = getinput(par_mode);
     	for(int i = 0; cmd[i] != NULL; i++){
		char **new_array = malloc(512 * sizeof(char*));
    		char *temps = strdup(cmd[i]);
    		char *token = strtok(temps," \n\t");
    		if(token!=NULL){
    		    new_array[0] = strdup(token);
    		}
    		int j = 1;
    		while(token!=NULL){
			token = strtok(NULL," \n\t");
			if (token!=NULL){
				new_array[j] =strdup(token); 
				j++;
			}
    		}
		for(int z = 0; new_array[z]!= NULL; z++){
		     int match = strcasecmp(new_array[z], "mode");
		     toexit = strcasecmp(new_array[z], "exit");
	     	     if (match == 0){
	                new_mode = mode_command(new_array, &par_mode);
		     	mode_changed = 1;
	    	     }
		     if (toexit ==0){
//	                printf("j value %d\n", j);
		        if ((z==0) && (j == 1)){
		        exit(1);
			}
	    	     }
		}
    		new_array[j] = NULL;
		free(token);
    		free(temps);
		pid_t pid = fork();
		num_children ++;
        	if(pid==0){
			char* thiscommand = new_array[0];
			char* newcommand = thiscommand;
			char **this_array = malloc(512 * sizeof(char*));
			if (datafile != NULL){
				while(fgets(filestream, 2000000, datafile)!=NULL){
					int slen = strlen(filestream);
					filestream[slen-1] = '\0';
					
					char *token = strtok(filestream,"\n");
					if(token!=NULL){
    		    				this_array[0] = strdup(token);
    					}
					int j = 1;
    					while(token!=NULL){
						token = strtok(NULL,"\n");
						if (token!=NULL){
						this_array[j] = strdup(token);
		//				printf("all items: %s\n", this_array[j]);
						}
    					}
				}
				struct stat statresult;
				int rv = stat(thiscommand, &statresult);
				int i=0;
				while (rv < 0) {
    					// stat failed; file definitely doesn't exist
					newcommand = strcat(this_array[i], thiscommand);
					printf("the new command is %s\n", newcommand);
					rv = stat(newcommand, &statresult);
					i++;
					if(this_array[i] == NULL){
					        newcommand = thiscommand;
						break;
					}
				
				}
				i = 0;
				new_array[0] = newcommand;
			}
				
			if ((execv(new_array[0], new_array) < 0)&& mode_changed == 0 && toexit != 0) {
            			fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
				num_children --;
        		}
        	}	
        	else if(par_mode == 0){
			wait(&childrv);
       		}
		free(new_array);
	}
        if (par_mode ==1){
	    int f = 0;
	    while (f < num_children){
		wait(&childrv);
		f++;
	    }
	}
   	if (toexit == 0){
      	    exit(1);
        }
    	par_mode = new_mode;
    	toexit = 1;
    }
    return 0;
}
char** getinput(int par_mode){
     printf("type a command: ");
     fflush(stdout);
     char command[1024];
     char **new_array = malloc(512 * sizeof(char*));
     if(fgets(command, 1024, stdin)!= NULL){ 
	int commandlen = strlen(command);
	command[commandlen-1] = '\0'; 
	for(int i = 0; i < strlen(command); i++){
		if(command[i] == '#'){
			command[i] = '\0';
			break;
		}
	}
    	char *temps = strdup(command);
   	char *token = strtok(temps,";");
    	if(token!=NULL){
    		new_array[0] = strdup(token);
    	}
    	int j = 1;
    	while(token!=NULL){
		token = strtok(NULL,";");
		if (token!=NULL){
			new_array[j] =strdup(token); 
			j++;
		}	
    	}
    	new_array[j] = NULL;
    	free(temps);
     }  
     else{
		printf(" \n");
		exit(1);
	}
     return new_array;   
}



int mode_command(char **cmd, int *par_mode){
//    printf("cmd 1 = %s\n", cmd[1]);
    if (cmd[1] == NULL){
	if (*par_mode == 0){
	    printf("In sequential mode! \n");
	    }
	else{
	    printf("In parallel mode! \n");
	}
    }
    else if ((strcasecmp(cmd[1], "parallel") == 0) || (strcasecmp(cmd[1], "p") == 0)) {       printf("Swittching to parallel mode! \n");
	return 1;
    }
    else if ((strcasecmp(cmd[1], "sequential") == 0) || (strcasecmp(cmd[1], "s") == 0)) {   printf("Swittching to sequential mode! \n");
        return 0;
    }
    else{
	fprintf(stderr, "Not a mode");
	exit(1);
    }
    return *par_mode;
}







