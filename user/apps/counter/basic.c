/*                         Portable BASIC                          */
/*             Copyright (C) Waterfront Software 2012              */
/*                          Version 1.39                           */
/*                                                                 */
/* This file, basic.c, is the source code for the Portable BASIC   */
/* interpreter, which is copyright Waterfront Software 2012. You   */
/* may utilize portions of this code in your code, but Waterfront  */
/* Software retains all rights to the distribution and sale of     */
/* this program. Making profit from the distribution of this       */
/* source code in any form without the express permission of       */
/* Waterfront Software is illegal under international copyright    */
/* law.                                                            */
/*                                                                 */
/* Compiling:                                                      */
/* Android:                                                        */
/* Download the C4droid app, and open it. Copy this code into the  */
/* app, and run it.                                                */
/*                                                                 */
/* Linux:                                                          */
/* To compile Portable BASIC in desktop Linux, you're going to     */
/* need the Tiny C Compiler (TCC.) There is a tar file available;  */
/* just Google it and install it.                                  */
/*                                                                 */
/* This Version:                                                   */
/* This version of Portable BASIC contains the complete PRINT      */
/* statement, the first I/O statement available in Portable BASIC! */
/* It also has command line arguments; type basic --help after     */
/* compilation for more information.                               */
/*                                                                 */
/* This version is also the first to contain the shell statement,  */
/* which allows commands to be passed to the system.               */



//Libraries are imported here.
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

//#include <netbsd/nbsdfilestruc.h>

//Global static variables are defined here.
//Remember to edit the print string in help token.
#define VERSION 1.39
#define REV 7
#define NUMTOKENS 8
#define TERMHEIGHT 100

int system(const char *command)

 {
return -1; //unimpl
}

int exit() {

return 0;
}

//Types are defined here.
typedef char * string;

// Global variables are defined here.
//string tokens[2];
//tokens[0] = "rem";
//tokens[1] = "exit";
bool c = true;
int i;
//struct sysinfo info;
int singDigits[10] = {0,1,2,3,4,5,6,7,8,9};

int main (int argc, char *argv[])
{
  string tokendefs[NUMTOKENS];
  string tokens[NUMTOKENS];
  tokens[6] = "rem";
  tokendefs[6] = "Represents a comment";
  tokens[5] = "print";
  tokendefs[5]="Writes strings to the output buffer";
  tokens[1] = "exit";
  tokendefs[1] = "Exits Portable BASIC";
  tokens[3] = "mem";
  tokendefs[3] = "Displays information about system memory";
  tokens[0] = "cls";
  tokendefs[0] = "Clears the screen";
  tokens[2] = "help";
  tokendefs[2] = "Displays this help text";
  tokens[4] = "nop";
  tokendefs[4] = "No operation, for testing purposes.";
  tokens [7] = "shell";
  tokendefs [7] = "Starts a program";
  if(argc > 1)
  {
      //printf ("args encountered %s", argv[1]);
      //printf(argv[1]);
      //printf(argv[2]);
      argv[1] = tolower(argv[1]);
      //printf("%c", argv[1][0]);
      //putchar((argv[1] == "--help"));
      if(argv[1][0] == '-' && argv [1][1] == '-' && argv [1][2]=='h' && argv[1][3]=='e' && argv[1][4] == 'l' && argv[1][5] == 'p' && (strlen(argv[1])) == 6)
      {
          printf("Portable BASIC v%1.2f, rev%d\nCopyright (C) Waterfront Software 2012\n\nUsage: \n\tbasic [options] [file | code]\n\n\nOption\t\tPurpose\n\n[file]\t\tSpecifies a BASIC code file to be executed\n[code]\t\tBASIC code to pass, use with -i \n--help\t\tDisplays program usage guidelines\n-i\t\tInterprets BASIC code that has been passed", VERSION, REV);
          exit (0);
      }
      else if(argv[1][0]=='-' && argv[1][1] == 'i' && (strlen (argv[1])) == 2)
      {
          //printf ("Interpret command line received; code not yet implemented\nStarting interpreter...\n\n");
          //lineInt (argv[2]);
          //printf(argv[2]);
            lineInt(argv[2], tokens, tokendefs);
            exit (0);
      }
      else
      {
          //FILE *f;
          char line[65536];
int dummy = 9;
int ymmud = 9000;
   if(dummy != ymmud)
   {
       //printf("!seY");
       while(false)
   {
       //
       //line [strlen(line)-2] = '\0';
       lineInt(line, tokens, tokendefs);
       //printf(line);
       //line [strlen(line)-1] = '\0';
       //printf("%s", line);
       
	 /* get a line, up to 80 chars from fr.  done if NULL */
	 //sscanf (line, "%ld", &elapsed_seconds);
	 /* convert the string to a long int */
	 //printf ("%ld\n", elapsed_seconds)
   }
   
   exit(0);
   }
  // else
   //{
     //     printf("Error: Invalid file or option %s\n\n", argv[1]);
   // }
      }
  }
  int availMem = GetAvailableMemory();
 
  printf("Portable BASIC v%1.2f, rev%d\nCopyright (C) Waterfront Software 2012\n\nApprox. %i bytes free to C", VERSION, REV, availMem);
  //for(i=0; i <= 9; i++)
  //{
      //printf("%i", singDigits[i]);
  //}
  while(c)
  {
    printf("\n> ");
    char str[65536];
   // gets(str);
//    str = *GetPolledKbdLine();

str = getc();
	char strBackup[65536];
    //strBackup = str;
    for(i=0; i <= strlen(str) - 1; i++)
    {
        strBackup[i]=str[i];
    }
    char * tok;
    tok = strtok(str," ");
    //printf("%s\n",tok);
    for(i=0; i <= strlen(tok) - 1; i++)
    {
        tok[i]=tolower(tok[i]);
    }
    //tok = tolower(tok);
    //printf("%s %s", tok, tokens[0]);
    for(i=0; i <= NUMTOKENS - 1; i++)
    {
      if(tok[0]=='\n') break;
      if (tok[0] == tokens[i][0])
      {
          //puts(strBackup);
          if(tokDet(tok, strBackup, tokens, tokendefs, VERSION, REV))
          {
              //printf("here");
              break;
          }
          else if(i==NUMTOKENS-1)
          {
              printf("Error: Unknown token %s\n", tok);
              break;
          }
      }
      else 
      {
          if(i==NUMTOKENS-1)
          {
              printf("Error: Unknown token %s\n", tok);
              break;
          }
      }
    }
  }
  return 0;
}

//Determines which token is being requested.
//Returns 1 for success, 0 for failure.
int tokDet (char * tok, char * comStr[65536], string tokens[NUMTOKENS], string tokendefs[NUMTOKENS])
{
  //printf("here");
  //printf(tok);
  //printf("%c", tok[0]);
  //printf("%i", strlen(tok));
  if(tok[0]=='e' && tok[1]=='x' && tok[2]=='i' && tok[3]=='t' && (strlen(tok))==4)
  {
    //printf("Exit successful.");
    exit(0);
  }
  if(tok[0]=='h' && tok[1]=='e' && tok[2]=='l' && tok[3]=='p' && (strlen(tok))==4)
  {
    //printf("%1.2f", v);
    printf("Portable BASIC v1.39, rev%i\nCopyright (C) Waterfront Software 2012\n\n", REV);
    printf("Command\t\tFunction\n\n");
    for(i=0; i <= NUMTOKENS - 1; i++)
    {
        printf("%s\t\t%s\n", tokens[i], tokendefs[i]);
    }
    return 1;
  }
  else if(tok[0]=='r' && tok[1]=='e' && tok[2]=='m' && (strlen(tok)) == 3)
  {
      return 1;
  }
  else if(tok[0]=='n' && tok[1]=='o' && tok[2]=='p' && (strlen(tok)) == 3)
  {
      return 1;
  }
  else if(tok[0]=='m' && tok[1]=='e' && tok[2]=='m' && (strlen(tok)) == 3)
  {
      printf("Approx. %i bytes free to C", GetAvailableMemory());
     // if(info.procs==0 && info.uptime==0) printf("\nError retrieving data, the system returned zeroes. Are you root?\n");
      return 1;
  }
   else if(tok[0]=='c' && tok[1]=='l' && tok[2]=='s' && (strlen(tok)) == 3)
  {
      //printf("%i", system("clear"));
      //printf("here");
      if(system("cls")==32512)
      {
          if (system("clear")==32512)
          {
              printf("Shell has no recognized clear method.\nAttempting manual clear...");
              for(i=0; i <= TERMHEIGHT; i++)
              {
                  printf("\n");
              }
          }
      }
      return 1;
  }
  else if(tok[0]=='s' && tok[1]=='h' && tok[2]=='e' && tok[3]=='l' && tok[4]=='l' && (strlen(tok)) == 5)
  {
  //printf ("chrs");
      char *q1 = strchr (comStr,'"');
      if(q1==NULL)
      {
          //char *q4 = strchr(comStr, " ");
          printf("Non-literal string encountered; code has not yet been implemented");
      }
          //if (q4==NULL)
          //{
              //q4 = strlen(comStr);
              //printf("Herr");
              //printf("%.*s", q4-q1-1, q1 + 1);
          //}
      //}
       // if not NULL, q1 points to the first quote
      if (q1) {
          char *q2 = strchr(q1 + 1, '"'); // if not NULL, q2 points to the second quote
          if(q2==NULL)
      {
          printf("Error: Missing end quote.");
      return 1;
      }
      if (q2) 
      {
          //printf (("%.*s", q2-q1-1, q1));
          //int result = (system (("%.*s", q2-q1-2, q1-1)));
          char * str = ("%.*s", q1-1, q1+1);
          str[strlen(str)-1]=NULL;
          //printf("%s\n", str);
          int result = (system (str));
          if(result == 32512)
          {
              printf("Error: %s cannot be started", str);
          
          }
          else if(result == 32256)
          {
          }
          else if(result == 0) return 1;
          else
          {
              //printf ("Error: An unknown error code %i was encountered", result);
          }
          for(i=0;i<strlen(str);i++)
          {
              str[i]=NULL;
          }
          memset(&str[0], 0, sizeof(str));
          //printf("\n%sy", str);
      } 
      }
      return 1;
      }
      else if(tok[0]=='p' && tok[1]=='r' && tok[2]=='i' && tok[3]=='n' && tok[4]=='t' && (strlen(tok)) == 5)
  {
      if(comStr=="print")
      {
          printf("\n");
          return 1;
      }
  //printf ("chrs");
      char *q1 = strchr (comStr,'"');
      if(q1==NULL)
      {
          //char *q4 = strchr(comStr, " ");
          printf("Non-literal string encountered; code has not yet been implemented");
      }
          //if (q4==NULL)
          //{
              //q4 = strlen(comStr);
              //printf("Herr");
              //printf("%.*s", q4-q1-1, q1 + 1);
          //}
      //}
       // if not NULL, q1 points to the first quote
      if (q1) {
          char *q2 = strchr(q1 + 1, '"'); // if not NULL, q2 points to the second quote
          if(q2==NULL)
      {
          printf("Error: Missing end quote.");
      return 1;
      }
      if (q2) 
      {
          printf("%.*s", q2-q1-1, q1 + 1); // print whatever is between the quotes
      } 
    else 
    {
        /* maybe some error */
    }
    printf("\n");
} 
else {
    /* maybe some error */
}
      //char check[65536];
      //strcpy(check, comStr + 5);
      //printf("%s", check);
      //int b = strchr(comStr, '\"');
      //printf("%i", strlen(comStr));
      //string check;
      //char check[65536];
      //for(i=0;i <= strlen(comStr) - 1; i++)
      //{
          //check[i] = comStr[i+8];
      //}
      //check[i] = '\0';
      //printf("%s", comStr + 2);
      //for(i=0; i <= strlen(check) - 1; i++)
      //{
          //putchar(check[i]);
      //}
      //printf(b +1);
      //printf("\"");
      //char subbuff[5]
      //memcpy( subbuff, &buff[10], 4 );
      //subbuff[4] = '\0';
      //string x= strndup(comStr + 1.5, 11);
      //puts(x);
      //printf("here");
      //string * toks = strtok(comStr, " ");
      //char * x = substring(comStr, (size_t)5, (size_t)5);
      //puts(x);
      //printf(toks[0][0]);
      //char toks = strtok(comStr, " ");
      //for(i=0; i<=4; i++)
      //{
          //printf((char)toks[i]);
      //}
      //printf("comStr = %s\n", comStr);
      return 1;
  }
  else
  {
      return 0;
  }
}

//Gets maximum memory available to C.
int GetAvailableMemory()
{
    char *p;
    int siz = 1;

    p = calloc(1,(size_t)siz);
    while (p) {
        siz= siz + 32768; // Can be more to speed up things
        //printf("%i\n", siz);
        p = realloc(p,(size_t)siz);
    }
    free(p);
    return siz;
}

int lineInt(char * str, string tokens[NUMTOKENS], string tokendefs[NUMTOKENS])
{
    char strBackup[65536];
    //strBackup = str;
    for(i=0; i <= strlen(str) - 1; i++)
    {
        strBackup[i]=str[i];
    }
    char *tok;
    tok = strtok(str," ");
    //printf("%s\n",tok);
    for(i=0; i <= strlen(tok) - 1; i++)
    {
        tok[i]=tolower(tok[i]);
    }
    if(tok[strlen(tok)-1]=='\n')
    {
        //tok++;
        //memmove (tok, tok-1, strlen (tok-1));
        tok[strlen(tok)-1] = 0;
    }
    if(tok[strlen(tok)-1]=='\n') printf("ugh!");
    //tok = trimNL (*tok);
    //tok = tolower(tok);
    //printf("%s %s", tok, tokens[0]);
    for(i=0; i <= NUMTOKENS - 1; i++)
    {
      if(tok[0]=='\n') break;
      if (tok[0] == tokens[i][0])
      {
          //puts(strBackup);
          //if(tokDet(tok, strBackup, tokens, tokendefs, VERSION, REV))
          //{
              //printf("here");
              //break;
          //}
          if(tokDet(tok, strBackup, tokens, tokendefs))
          {
                //printf("here");
                break;
          }
          else if(i==NUMTOKENS-1)
          {
              printf("Error: Unknown token %s\n", tok);
              break;
          }
      }
      else 
      {
          if(i==NUMTOKENS-1)
          {
              printf("Error: Unknown token %s\n", tok);
              break;
          }
      }
    }
  //printf("here");
  return 0;
}

//char *trimNL(char *str)
//{
  //char *end;

  // Trim leading space
  //while((*str)=='\n') str++;

  //if(*str == 0)  // All spaces?
    //return str;

  // Trim trailing space
  //end = str + strlen(str) - 1;
  //while(end > str && (*end) == '\n') end--;

  // Write new null terminator
  //*(end+1) = 0;

  //return str;
//}
