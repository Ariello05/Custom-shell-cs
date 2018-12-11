#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#define TRUE (1==1)
#define FALSE (!TRUE)

#define COUNT 32
#define COUNT2 16

void freememory(char *var[COUNT])
{
	for(int i = 0; i < COUNT; ++i)
		free(var[i]);
}

void freememory2(char *var[COUNT2])
{
	for(int i = 0; i < COUNT2; ++i){
		if(var[i] != NULL)
		free(var[i]);
	}
}

void readcheck(char *var[COUNT])
{	
	printf("--------CHECK-------\n");
	for(int i = 0; i < COUNT; ++i)
	{
		printf("ID: %d \t %s\n",i ,var[i]);
	}
	printf("------ENDCHECK------\n");
}

void readcheck2(char *var[COUNT2])
{
	printf("--------CHECK-------\n");
	for(int i = 0; i < COUNT2; ++i)
	{
		if(var [i] != NULL )
			printf("ID: %d \t %s\n",i ,var[i]);
	}
	printf("------ENDCHECK------\n");
}


void reset(char *var[COUNT], char * c)
{
	printf("------STARTSET------\n");
	for(int i = 0; i < COUNT; ++i)
	{
		strncpy(var[i],c,strlen(c));
	}
	printf("-------ENDSET-------\n");
}

void repair(char *var[COUNT])
{
	for(int i = 0; i< COUNT; ++i)
	{
		if(var[i] == NULL)
		{
			//printf("REALOKACJA W:%d\n",i);
			var[i] = malloc(sizeof(char) * 129);
		}
	}
}

void exitCommand(char *str[COUNT])
{
	freememory(str);
	exit(1);
}

void cdCommand(char *str)
{
	if(chdir(str) != 0)
	{
		perror("cd");
	}
}

void jobsCommand(int kids[64])
{
	int status;
	printf("JOBS: \n");
	for(int i = 0; i < 64; ++i)
	{
		int ret = waitpid(kids[i], &status, WNOHANG);
		if(ret == 0)
		printf("I:%d \t Status:%d\n ",i,status);
	}
}

void killCommand(int pid, int kids[64])
{
	int status;
	for(int i = 0; i < 64; ++i)
	{
		int ret = waitpid(kids[i], &status, WNOHANG);
		if(status == pid)
		{
			kill(pid,SIGKILL);
		}
	}

}

int checkBuiltinCommands(char *str[COUNT],int kids[64])
{
	if(strcmp(str[0],"exit") == 0 )
	{
		exitCommand(str);	
		return TRUE;//exits anyway
	}
	else if(strcmp(str[0],"cd") == 0)
	{
		cdCommand(str[1]);
		return TRUE;
	}
	else if(strcmp(str[0],"jobs") == 0)
	{
		jobsCommand(kids);
		return TRUE;
	}
	else if(strcmp(str[0],"kill") == 0)
	{
		killCommand(atoi(str[1]),kids);
		return TRUE;
	}
	return FALSE;
}

void checkContr_D(int c, char *str[COUNT])
{
	if(c == -1)
	{
		freememory(str);
		printf("^D\n");
		exit(1);
	}
}

int readarg(int k, char *str[COUNT])
{
	return scanf("%127s", str[k]);//reads 127char into string
}

//int doWait()
//{
//
//}
int insertNewKid(int pid, int kids[64])
{
	for(int i = 0; i < 64; ++i)
	{
		if(kids[i] == 0)
		{
			kids[i] = pid;
			return 1;
		}
	}

	return 0;
}

void cleanup(int kids[64])
{
	for(int i = 0; i < 64; ++i)
	{
		if( kids[i] == 0 )
		{	continue;	}
		else
		{
			//printf("OBSLUGA:%d\n", i);
		}

		int status;
		int ret = waitpid(kids[i], &status, WNOHANG);
		
		if( ret == -1)
		{
			//printf(" UNKOWN ERROR: %d\n", status);
			kids[i] = 0;
		}
		else if( ret == 0 )
		{
			//printf(" KID IS RUNNING: %d\n", status);
		}
		else if( ret == kids[i] )//finished
		{
			//printf(" FINISHED?:  %d\n", status);
			waitpid(kids[i], &status, 0);//we invoke wait on zombie
			kids[i] = 0;
		}
	}
}

int getPipePos(int *type, char *str[COUNT], int k)
{
	int pipeK = -1;
	for(int i = 0; i < k - 1; ++i)
	{
		if(strcmp(str[i],"|") == 0)
		{
			*type = 1;
			pipeK = i;
			break;
		}
		else if(strcmp(str[i],">") == 0)
		{
			*type = 2;
			pipeK = i;
			break;
		}
		else if(strcmp(str[i],"<") == 0)
		{
			*type = 3;
			pipeK = i;
			break;
		}
	}
	return pipeK;
}

void normalExecute(char *str[COUNT], int czekaj, int kids[64])
{
	int status = 0;
	int fo, wpid;
	fo = fork();

	if(fo == 0)
	{
		if(!insertNewKid(fo, kids))//as a check
		{
			printf("WARNING: Couldn`t initialize new child process!\n");
		       	exit(1);	
		}

		//printf("%s \t %s \n", str[0], str[1]);
		execvp(str[0], str);
		perror("execvp");
		exit(1);
	}
	else
	{
		if(!insertNewKid(fo, kids))
		{
			printf("WARNING: Too many child proccess running!\n");
		}

		if(czekaj == 1)
		{
			while((wpid = waitpid(fo, &status, 0)) > 0)
			{
				//printf("Returned: %d\n", wpid);
			}
		}

	}	


}

int oneExecute(char *Astring[COUNT2], char *Bstring[COUNT2], int kids[64], int czekaj)
{
	int t2, t1, i;
	int pipefd[2];

	i = pipe(pipefd);
	if( i < 0 ){
		perror("Pipe:");
		return -1;
	}

	int id1 = fork();
	if( id1 == 0)
	{
		if(!insertNewKid(getpid(),kids))//dummy
		{
			printf("Can`t add another kid");
			exit(1);
		}
		if(dup2(pipefd[1], 1) != 1)
		{
			perror("pipefd[1]: ");
			exit(1);
		}
		close(pipefd[1]);
		close(pipefd[0]);
		execvp(Astring[0],Astring);
		perror("exec 1: ");
		exit(1);
	}
	int id2 = fork();
	if( id2 == 0)
	{
		if(!insertNewKid(getpid(),kids))//dummy
		{
			printf("Can`t add another kid");
			exit(1);
		}
		
		if(dup2(pipefd[0], 0) != 0)
		{
			perror("pipefd[0]: ");
			exit(1);
		}
		close(pipefd[1]);
		close(pipefd[0]);
		execvp(Bstring[0],Bstring);
		perror("exec 2: ");
		exit(1);
	}
	else
	{
		if(!insertNewKid(id1,kids) ||	!insertNewKid(id2,kids))
		{
			close(pipefd[1]);
			close(pipefd[0]);
			
			return -1;
		}

		close(pipefd[1]);
		close(pipefd[0]);

		if(czekaj == 1)
		{

			int wpid;
			while((wpid = waitpid(id1, &t1, 0)) > 0)
			while((wpid = waitpid(id2, &t2, 0)) > 0)

			if(WEXITSTATUS(t1) || WEXITSTATUS(t2))
			{
				fprintf(stderr, "Blad pipe. \n");
				return -1;
			}
		}
	}
	return 0;
}


//0 nothing
//1 |
//2 >
//3 <
void execute(char *str[COUNT], int k, int  kids[64])
{
	int type = 0;
	int pipeK = getPipePos(&type, str, k);

	//char * old = str[k];
	int czekaj = 1;

	if(type == 0)
	{
		str[k] = NULL;
	
		int czekaj = 1;
		char * s = str[k-1];
		int len = strlen(s);

		if(s[len-1] == '&')//s[len]=='/0'
		{
			czekaj = 0;
			if(len > 1)
			{
				strncpy(s, s, len-2);//so eg.: command -c& can work
				s[len-1] = '\0';
			}
			else if(len == 0)
			{
				s[0] = '\0';
			}
		}
		normalExecute(str, czekaj, kids);
	}
	else
	{	
		int czekaj = 1;
		if( strcmp(str[k-1], "&") == 0)
		{
			czekaj = 0;
		}

		//printf("naszeD:%d \t naszeT:%d\n",pipeK,type);
		char *Astring[COUNT2];
		for (int i = 0; i < COUNT2; ++i)
		{		
			Astring[i] = malloc(sizeof(char) * 127);
			if( i < pipeK )
			{
				strncpy(Astring[i],str[i],strlen(str[i]));
			}
			else
				Astring[i] = NULL;
		}
		Astring[pipeK] = NULL;

		int range = (k-pipeK - 1);
		char *Bstring[COUNT2];
		int posAfter = pipeK + 1;
		for(int i = posAfter; i < COUNT2; ++i)
		{
			Bstring[i-posAfter] = malloc(sizeof(char) * 127);

			if( i < k )
			{
				strncpy(Bstring[i-posAfter],str[i],strlen(str[i]));
				if(strcmp(Bstring[i-posAfter],"&") == 0)//getting rid of &
				{
					Bstring[i-posAfter] = NULL;
				}
			}
			else
				Bstring[i] = NULL;
		}
		Bstring[range] = NULL;
		//readcheck2(Bstring);

		if(type != 0)//only | :(
		{
			oneExecute(Astring,Bstring, kids, czekaj);
		}
			
	}

}

int checkLoop(int k, int valid, char *str[COUNT])
{
	checkContr_D(valid, str);
	if(k > 32)
	{
		printf("WARNING: Too many arguments\n");// args no.32+ will go to the next loop
		return FALSE;
	}
	
	char temp;
	while((temp=getchar()) != EOF && temp != '\n')
	{
		if(temp != ' ')//Getting rid of spaces
		{
			ungetc(temp,stdin);
			return TRUE;
		}
	}

	return FALSE;
}

void cntl_c_handler(int dummy)
{
	signal(SIGINT, cntl_c_handler);
	//printf("^C\n"); it`s already written out
}


int main()
{
	signal(SIGINT, cntl_c_handler);

	int kids[64];
	for(int i = 0; i < 64; ++i)
	{
		kids[i] = 0;
	}

	char *string[COUNT];
	const int sizeString = 128;

	for(int i = 0 ; i < COUNT ; ++i)
	{
		string[i] = malloc(sizeof(char) * (sizeString + 1));
	}
	

	int k = 0;
	int valid;
	while(TRUE)
	{
		
		printf("lsh> ");
		do{
			valid = readarg(k,string);
			++k;

		}while(checkLoop(k,valid,string));
		
		if(!checkBuiltinCommands(string, kids)){//didn`t see any bultins
			execute(string, k, kids);
		}
		//sleep(1);
		cleanup(kids);
		repair(string);
		k = 0;
	}

	freememory(string);	
	
}
