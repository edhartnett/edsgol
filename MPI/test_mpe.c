/* Test program for MPE.

   Ed Hartnett, 11/1/08
   $Id: test_mpe.c,v 1.1 2008/11/01 21:36:03 edhartnett Exp $
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <mpe.h>

/* global err buffer for MPI. */
int resultlen;
char err_buffer[MPI_MAX_ERROR_STRING];

/* Error handling code derived from an MPI example here: 
   http://www.dartmouth.edu/~rc/classes/intro_mpi/mpi_error_functions.html */
#define MPIERR(e) do {							\
      MPI_Error_string(e, err_buffer, &resultlen);			\
      printf("MPI error, line %d, file %s: %s\n", __LINE__, __FILE__, err_buffer); \
      MPI_Finalize();							\
      return 2;								\
   } while (0) 

#define ERR 2

#define NUM_EVENTS 7
#define START 0
#define END 1
#define INIT 0
#define UPDATE 1

int
main(int argc, char* argv[]) 
{
   int n, my_rank;
   int event_num[2][NUM_EVENTS];
   int ret;

   /* Initialize MPI. */
   MPI_Init(&argc, &argv);
   MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

   /* Learn my rank and the total number of processors. */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &n);

   event_num[START][INIT] = MPE_Log_get_event_number();
   event_num[END][INIT] = MPE_Log_get_event_number();
   if (!my_rank)
      MPE_Describe_state(event_num[START][INIT], event_num[END][INIT], "init", "yellow");
   event_num[START][UPDATE] = MPE_Log_get_event_number();
   event_num[END][UPDATE] = MPE_Log_get_event_number();
   if (!my_rank)
      MPE_Describe_state(event_num[START][UPDATE], event_num[END][UPDATE], "update", "green");

   if ((ret = MPE_Log_event(event_num[START][INIT], 0, "start init")))
      MPIERR(ret);
   sleep(1);
   if ((ret = MPE_Log_event(event_num[END][INIT], 0, "end init")))
      MPIERR(ret);

   if ((ret = MPE_Log_event(event_num[START][UPDATE], 0, "start update")))
      MPIERR(ret);
   sleep(1);
   if ((ret = MPE_Log_event(event_num[END][UPDATE], 0, "end update")))
      MPIERR(ret);

   MPI_Finalize();
   return 0;
}
