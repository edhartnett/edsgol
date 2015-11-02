/* Game of life for High Performance Computing Class.

   Ed Hartnett, 10/13/07
   $Id: gol.c,v 1.38 2008/11/28 19:42:56 edhartnett Exp $
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#ifdef LOGGING
#include <mpe.h>
#endif

/* In the game of life, two's company, three's a crowd. */
#define COMPANY 2
#define A_CROWD 3

/* Some constants. */
#define NDIMS 2
#define NUM_PARENTS 3
#define MAX_NAME 255
#define HBUF_SIZE 100

/* These are for the event numbers array used to log various events in
 * the program with the MPE library, which produces output for the
 * Jumpshot program. */
#define NUM_EVENTS 7
#define START 0
#define END 1
#define INIT 0
#define UPDATE 1
#define WRITE 2
#define SWAP 3
#define COMM 4
#define CALCULATE 5 
#define INGEST 6

/* Some error codes for when things go wrong. */
#define ERR_FILE 1
#define ERR_DUMB 2
#define ERR_ARG 3
#define ERR_MPI 4
#define ERR_MPITYPE 5
#define ERR_LOGGING 6
#define ERR_UPDATE 7
#define ERR_CALC 8
#define ERR_COUNT 9
#define ERR_WRITE 10
#define ERR_SWAP 11
#define ERR_INIT 12

/* Error handling code derived from an MPI example here: 
   http://www.dartmouth.edu/~rc/classes/intro_mpi/mpi_error_functions.html */
#define MPIERR(e) do {                                                  \
      MPI_Error_string(e, err_buffer, &resultlen);                      \
      printf("MPI error, line %d, file %s: %s\n", __LINE__, __FILE__, err_buffer); \
      MPI_Finalize();                                                   \
      return 2;                                                         \
   } while (0) 

#define ERR(e) do {                                                     \
      MPI_Finalize();                                                   \
      return e;                                                         \
   } while (0) 

/* global err buffer for MPI. */
int resultlen;
char err_buffer[MPI_MAX_ERROR_STRING];

/* This will set up the MPE logging event numbers. */
int
init_logging(int my_rank, int event_num[][NUM_EVENTS])
{
#ifdef LOGGING
   /* Get a bunch of event numbers. */
   event_num[START][INIT] = MPE_Log_get_event_number();
   event_num[END][INIT] = MPE_Log_get_event_number();
   event_num[START][UPDATE] = MPE_Log_get_event_number();
   event_num[END][UPDATE] = MPE_Log_get_event_number();
   event_num[START][INGEST] = MPE_Log_get_event_number();
   event_num[END][INGEST] = MPE_Log_get_event_number();
   event_num[START][COMM] = MPE_Log_get_event_number();
   event_num[END][COMM] = MPE_Log_get_event_number();
   event_num[START][CALCULATE] = MPE_Log_get_event_number();
   event_num[END][CALCULATE] = MPE_Log_get_event_number();
   event_num[START][WRITE] = MPE_Log_get_event_number();
   event_num[END][WRITE] = MPE_Log_get_event_number();
   event_num[START][SWAP] = MPE_Log_get_event_number();
   event_num[END][SWAP] = MPE_Log_get_event_number();

   /* You should track at least initialization and partitioning, data
    * ingest, update computation, all communications, any memory
    * copies (if you do that), any output rendering, and any global
    * communications. */
   if (!my_rank)
   {
      MPE_Describe_state(event_num[START][INIT], event_num[END][INIT], "init", "yellow");
      MPE_Describe_state(event_num[START][INGEST], event_num[END][INGEST], "ingest", "red");
      MPE_Describe_state(event_num[START][UPDATE], event_num[END][UPDATE], "update", "green");
      MPE_Describe_state(event_num[START][CALCULATE], event_num[END][CALCULATE], "calculate", "orange");
      MPE_Describe_state(event_num[START][WRITE], event_num[END][WRITE], "write", "purple");
      MPE_Describe_state(event_num[START][COMM], event_num[END][COMM], "reduce", "blue");
      MPE_Describe_state(event_num[START][SWAP], event_num[END][SWAP], "swap", "pink");
   }
#endif /* LOGGING */
   return 0;
}


/* Count up the number of life forms in a buffer. This total
 * is useful for checking that the game is working properly, since it
 * will be the same every time for a given input file and number of
 * generations. */
int
count_results(int my_rank, int count, int ln, int size, int checkerboard, 
              int event_num[][NUM_EVENTS], int verbose, unsigned char *buf, 
              int *total)
{
   int my_total;
   int ret, i, j;

   /* Count them doggies! */
   if (count)
   {
      if(checkerboard)
      {
         /* Skip first and last row and col, the ghost data. */
         my_total = *total = 0;
         for (i = 1; i < ln + 1; i++)
            for (j = 1; j < ln + 1; j++)
               if (buf[i * (ln + 2) + j]) 
                  my_total++;
      }
      else
      {
         for (my_total = 0, i = size; i < (ln + 1) * size; i++)
            if (buf[i]) 
               my_total++;
      }

      if (verbose)
         printf("%d : my_total=%d\n", my_rank, my_total);

#ifdef LOGGING      
      if ((ret = MPE_Log_event(event_num[START][COMM], 0, "start comm")))
         MPIERR(ret);
#endif

      /* Add the results from each task. */
      if ((ret = MPI_Reduce(&my_total, total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD)))
         MPIERR(ret);
            
#ifdef LOGGING
      if ((ret = MPE_Log_event(event_num[END][COMM], 0, "end comm")))
         MPIERR(ret);
#endif

      /* Print count, and, if small, the new array.*/
      if (verbose && size < 100)
      {
         if ((ret = MPI_Barrier(MPI_COMM_WORLD)))
            MPIERR(ret);
         for (i = 0, printf("%d: %d - ", my_rank, i); i < ln + 2; i++, printf("\t"))
            if (checkerboard)
               for (j = 0; j < ln + 2; j++)
                  printf("%d, ", buf[i * (ln + 2) + j]);
            else
               for (j = 0; j < size; j++)
                  printf("%d, ", buf[i * size + j]);
         printf("\n");
         if ((ret = MPI_Barrier(MPI_COMM_WORLD)))
            MPIERR(ret);
      }
   }
   return 0;
}

/* This function creates two MPI types, one to map to the file, and
 * one to map to memory (including taking account of ghost rows for
 * row decomposition, and ghost rows and colums for checkerboard
 * decomposition.) */
int
create_mpi_types(int my_rank, int n, int size, int ln, int checkerboard, 
                 MPI_Datatype *filetype, MPI_Datatype *memtype)
{
   int file_sizes[NDIMS], file_subsizes[NDIMS], file_starts[NDIMS];
   int mem_sizes[NDIMS], mem_subsizes[NDIMS], mem_starts[NDIMS];
   int sqrtn = (int)sqrt(n);
   int ret;

   /* The size of the array in the file, the same for both decompositions. */
   file_sizes[0] = file_sizes[1] = size;
      
   if (checkerboard)
   {
      /* The size of the local array (one square of the checkerboard). */
      file_subsizes[0] = file_subsizes[1] = ln;

      /* Map local array to file array based on my_rank. */
      file_starts[0] = my_rank/sqrtn * ln;
      file_starts[1] = (my_rank % sqrtn) * ln;

      /* Size of local data array, including ghost rows and columns. */
      mem_sizes[0] = mem_sizes[1] = ln + 2;

      /* Size of the "real" data in that array. */
      mem_subsizes[0] = mem_subsizes[1] = ln;
      
      /* Where to find real data. */
      mem_starts[0] = mem_starts[1] = 1;
   }
   else /* Row decomposition. */
   {
      /* Size of the local array (one row). */
      file_subsizes[0] = ln;
      file_subsizes[1] = size;
      
      /* Where in the file is *my* row? */
      file_starts[0] = my_rank * ln;
      file_starts[1] = 0;

      /* Size of row in memory. */
      mem_sizes[0] = ln + 2;
      mem_sizes[1] = size;

      /* Size of the data we really care about reading/writing. */
      mem_subsizes[0] = ln;
      mem_subsizes[1] = size;

      /* Skip the ghost row. */
      mem_starts[0] = 1;
      mem_starts[1] = 0;
   }

   /* Create and commit the types. */
   if ((ret = MPI_Type_create_subarray(NDIMS, file_sizes, file_subsizes, 
                                       file_starts, MPI_ORDER_C, MPI_BYTE, filetype)))
      MPIERR(ret);
   if ((ret = MPI_Type_commit(filetype)))
      MPIERR(ret);
   if ((ret = MPI_Type_create_subarray(NDIMS, mem_sizes, mem_subsizes, 
                                       mem_starts, MPI_ORDER_C, MPI_BYTE, memtype)))
      MPIERR(ret);
   if ((ret = MPI_Type_commit(memtype)))
      MPIERR(ret);

   return 0;
}

/* Initialize the current array, either from a file or with a simple
 * starting configuration for debugging. */
int
init_cur(int n, int my_rank, int p, int size, int ln, unsigned char *cur, 
         char *input_file, int count, int checkerboard, int verbose, 
         int event_num[][NUM_EVENTS], int file_type, MPI_Datatype *filetype, 
         MPI_Datatype *memtype)
{
   int header_bytes;
   MPI_File fh;
   char hbuf[HBUF_SIZE];
   int cols, rows;
   char blob[3];
   int sqrtn = (int)sqrt(n);
   int i, j;
   int ret;

   /* If the user gave us an input file, read it. */
   if (strlen(input_file))
   {
#ifdef LOGGING
      if ((ret = MPE_Log_event(event_num[START][INGEST], 0, "start ingest")))
         MPIERR(ret);
#endif

      /* Open the file and read the header. */
      if ((ret = MPI_File_open(MPI_COMM_WORLD, input_file, MPI_MODE_RDONLY, 
                               MPI_INFO_NULL, &fh)))
         MPIERR(ret);
      if ((ret = MPI_File_read_all(fh, hbuf, HBUF_SIZE, MPI_BYTE, MPI_STATUS_IGNORE)))
         MPIERR(ret);
      rows = cols = 900;
      header_bytes = 15;

      /* Check numbers and print info. */
      if (cols != size || rows != size)
         return ERR_FILE;
      if (verbose)
	printf("my_rank=%d cols=%d rows=%d header_bytes=%d\n", my_rank, cols, rows, header_bytes);

      /* Do the data read for this task. */
      if (file_type)
      {
         /* Use the MPI types for this read. */
         if ((ret = MPI_File_set_view(fh, header_bytes, MPI_BYTE, *filetype, "native", MPI_INFO_NULL)))
            MPIERR(ret);
         
         if (verbose && !my_rank)
            printf("about to read data\n");
         
         /* Read all our data in one collective operation, which will
          * get the data for this processor and put ut in the local
          * array, leaving the ghost row and colum untouched. */
         if ((ret = MPI_File_read_all(fh, cur, 1, *memtype, MPI_STATUS_IGNORE)))
            MPIERR(ret);
         
         if (verbose && !my_rank)
            printf("read data\n");
      }
      else
      {
         /* Do this read with primitive MPI seeks. */
         if (checkerboard)
         {
            int row_skip, col_skip, skip_to, read_start;
            /* This code uses primitive MPI I/O to read the data 
	       one row at a time into the local array, 
	       calculating the file and memory offsets. */
            for (i = 1; i < ln + 1; i++)
            {
               row_skip = my_rank/sqrtn * size * size/sqrtn + (i - 1) * size;
               col_skip = (my_rank % sqrtn) * ln;
               skip_to = header_bytes + row_skip + col_skip;
               read_start = (ln + 2) * i + 1;
/*	       printf("my_rank=%d skip_to=%d read_start=%d ln=%d\n", my_rank, skip_to, read_start, ln);*/

               /* Terms for the seek are: header + row offset 
		  for this processor and row + col offset for 
		  this processor. */
               if ((ret = MPI_File_seek(fh, skip_to, MPI_SEEK_SET)))
                  MPIERR(ret);
               if ((ret = MPI_File_read(fh, &cur[read_start], ln, MPI_BYTE, MPI_STATUS_IGNORE)))
                  MPIERR(ret);
            }
         }
         else /* row decomposition. */
         {
            if (verbose)
               printf("my_rank=%d reading %d bytes starting at %d\n", my_rank, ln * size, 
               ln * my_rank * size + header_bytes);
            if ((ret = MPI_File_read_at_all(fh, ln * my_rank * size + header_bytes, 
                                            &cur[size], ln * size, MPI_BYTE, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         }
      }

      /* Close the file. */
      if ((ret = MPI_File_close(&fh)))
         MPIERR(ret);

#ifdef LOGGING
      if ((ret = MPE_Log_event(event_num[END][INGEST], 0, "end ingest")))
         MPIERR(ret);
#endif
   }
   else 
   {
      /* No file to read, set to some pattern for small array
       * testing. */
      if (checkerboard)
      {
         for (i = 1; i < ln + 1; i++)
            for (j = 1; j < ln + 1; j ++)
               cur[i * (ln + 2) + j] = (unsigned char)(i % 2 ? 1 : 0);
      }
      else
      {
         /* Set to random numbers. */
         for (i = 0; i < (ln + 2) * size; i++)
            cur[i] = (unsigned char)rand();
      }
   }
   
   return 0;
}

/* Send the edge information to adjacent processes so that everyone
 * knows what it needs to from its neighbors. */
int
update_processes(int n, int my_rank, int p, int size, int ln, int sqrtn, int checkerboard, 
                 int verbose, MPI_Datatype col_type, int event_num[][NUM_EVENTS], unsigned char *cur)
{
   int ret;

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[START][UPDATE], 0, "start update")))
      MPIERR(ret);
#endif

   if (verbose && ! my_rank)
      printf("starting update\n");
   
   /* Fill the ghost rows. */
   if (p > 1)
   {
      if (checkerboard)
      {
         MPI_Request srt, srb, rrt, rrb;
         MPI_Request scl, rcr, rcl, scr;

         /* On verbose runs, barrier here to make text output look nicer. */
         if (verbose)
            MPI_Barrier(MPI_COMM_WORLD);

         /* Send top row. */
         if (my_rank/sqrtn)
            if ((ret = MPI_Isend(&cur[ln + 3], ln, MPI_BYTE, my_rank - sqrtn, 
            0, MPI_COMM_WORLD, &srt)))
               MPIERR(ret);

         /* Recieve it as bottom row. */
         if (my_rank/sqrtn != sqrtn - 1)
            if ((ret = MPI_Irecv(&cur[(ln + 2) * (ln + 1) + 1], ln, MPI_BYTE, my_rank + sqrtn, 
            0, MPI_COMM_WORLD, &rrb)))
               MPIERR(ret);
                  

         /* Send bottom row. */
         if (my_rank/sqrtn != sqrtn - 1)
            if ((ret = MPI_Isend(&cur[(ln + 2) * ln + 1], ln, MPI_BYTE, my_rank + sqrtn, 
            0, MPI_COMM_WORLD, &srb)))
               MPIERR(ret);

         /* Recieve it as top row. */
         if (my_rank/sqrtn)
            if ((ret = MPI_Irecv(&cur[1], ln, MPI_BYTE, my_rank - sqrtn, 
            0, MPI_COMM_WORLD, &rrt)))
               MPIERR(ret);

         /* All row sends must complete before col sends, because
          * of the corners. */
         if (my_rank/sqrtn)
            if ((ret = MPI_Wait(&srt, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank/sqrtn != sqrtn - 1)
            if ((ret = MPI_Wait(&srb, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank/sqrtn)
            if ((ret = MPI_Wait(&rrt, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank/sqrtn != sqrtn - 1)
            if ((ret = MPI_Wait(&rrb, MPI_STATUS_IGNORE)))
               MPIERR(ret);

/*          MPI_Barrier(MPI_COMM_WORLD); */

         /* Send left col. */
         if (my_rank % sqrtn)
            if ((ret = MPI_Isend(&cur[1], 1, col_type, my_rank - 1, 0, MPI_COMM_WORLD, &scl)))
               MPIERR(ret);

         /* Recieve it as right col. */
         if (((my_rank + 1) % sqrtn))
            if ((ret = MPI_Irecv(&cur[ln + 1], 1, col_type, my_rank + 1, 0, MPI_COMM_WORLD, &rcr)))
               MPIERR(ret);

         /* Send right col. */
         if (((my_rank + 1) % sqrtn))
            if ((ret = MPI_Isend(&cur[ln], 1, col_type, my_rank + 1, 0, MPI_COMM_WORLD, &scr)))
               MPIERR(ret);

         /* Recieve it as left col. */
         if (my_rank % sqrtn)
            if ((ret = MPI_Irecv(&cur[0], 1, col_type, my_rank - 1, 0, MPI_COMM_WORLD, &rcl)))
               MPIERR(ret);

         /* All row sends must complete before we calculate. */
         if (my_rank % sqrtn)
            if ((ret = MPI_Wait(&scl, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (((my_rank + 1) % sqrtn))
            if ((ret = MPI_Wait(&rcr, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (((my_rank + 1) % sqrtn))
            if ((ret = MPI_Wait(&scr, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank % sqrtn)
            if ((ret = MPI_Wait(&rcl, MPI_STATUS_IGNORE)))
               MPIERR(ret);
      }
      else
      {
         MPI_Request srt, rrt, srb, rrb;

         /* Send top row. */
         if (my_rank != 0)
            if ((ret = MPI_Isend(&cur[size], size, MPI_BYTE, my_rank-1, 0, MPI_COMM_WORLD, &srt)))
               MPIERR(ret);

         if (my_rank != p - 1)
            if ((ret = MPI_Irecv(&cur[(ln + 1) * size], size, MPI_BYTE, my_rank+1, 0, MPI_COMM_WORLD, 
            &rrt)))
               MPIERR(ret);

         /* Send bottom row. */
         if (my_rank != p - 1)
            if ((ret = MPI_Isend(&cur[ln * size], size, MPI_BYTE, my_rank+1, 0, MPI_COMM_WORLD, &srb)))
               MPIERR(ret);

         if (my_rank != 0)
            if ((ret = MPI_Irecv(&cur[0], size, MPI_BYTE, my_rank-1, 0, MPI_COMM_WORLD, &rrb)))
               MPIERR(ret);

         /* All row sends must complete before we calculate. */
         if (my_rank != 0)
            if ((ret = MPI_Wait(&srt, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank != p - 1)
            if ((ret = MPI_Wait(&rrt, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank != p - 1)
            if ((ret = MPI_Wait(&srb, MPI_STATUS_IGNORE)))
               MPIERR(ret);
         if (my_rank != 0)
            if ((ret = MPI_Wait(&rrb, MPI_STATUS_IGNORE)))
               MPIERR(ret);
      }
   }

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[END][UPDATE], 0, "end update")))
      MPIERR(ret);
#endif

   return 0;
}

/* Advance the game of life by one step by looking at the cur array
 * and filling the next array with the values for the next
 * generation. */
int 
calculate_next_step(int checkerboard, int ln, int size, int event_num[][NUM_EVENTS],
                    unsigned char *cur, unsigned char *next)
{
   int neighbors;
   int i, j;
#ifdef LOGGING
   int ret;
#endif

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[START][CALCULATE], 0, "start calculate")))
      MPIERR(ret);
#endif

   if (checkerboard)
   {
      /* Skip first and last row and col, the ghost data. */
      for (i = 1; i < ln + 1; i++)
      {
         for (j = 1; j < ln + 1; j++)
         {
            /* Count neighbors. */
            neighbors = 0;
            if (cur[(i-1) * (ln + 2) + j-1]) neighbors++;
            if (cur[(i-1) * (ln + 2) + j]) neighbors++;
            if (cur[(i-1) * (ln + 2) + j+1]) neighbors++;

            if (cur[i * (ln + 2) + j-1]) neighbors++;
            if (cur[i * (ln + 2) + j+1]) neighbors++;

            if (cur[(i+1) * (ln + 2) + j-1]) neighbors++;
            if (cur[(i+1) * (ln + 2) + j]) neighbors++;
            if (cur[(i+1) * (ln + 2) + j+1]) neighbors++;

            /* Check for change. */
            if (cur[i * (ln + 2) + j])
               next[i * (ln + 2) + j] = (unsigned char)((neighbors > A_CROWD || neighbors < COMPANY) ? 0 : 255);
            else
               next[i * (ln + 2) + j] = (unsigned char)((neighbors == NUM_PARENTS) ? 255 : 0);

/*             if (verbose)
               printf("%d: %d, %d, cur=%d, neighbors=%d next=%d\n", my_rank, i, j, cur[i * (ln + 2) + j], 
               neighbors, next[i * size + j]);*/
         } /* next j */
      } /* next i */
   }
   else
   {
      for (i = 1; i < ln + 1; i++)
      {
         for (j = 0; j < size; j++)
         {
            /* Count neighbors. */
            neighbors = 0;
            if (j && cur[(i-1) * size + j-1]) neighbors++;
            if (cur[(i-1) * size + j]) neighbors++;
            if (j < size - 1 && cur[(i-1) * size + j+1]) neighbors++;

            if (j && cur[i * size + j-1]) neighbors++;
            if (j < size - 1 && cur[i * size + j+1]) neighbors++;

            if (j && cur[(i+1) * size + j-1]) neighbors++;
            if (cur[(i+1) * size + j]) neighbors++;
            if (j < size - 1 && cur[(i+1) * size + j+1]) neighbors++;

            /* Check for change. */
            if (cur[i * size + j])
               next[i * size + j] = (unsigned char)((neighbors > A_CROWD || neighbors < COMPANY) ? 0 : 255);
            else
               next[i * size + j] = (unsigned char)((neighbors == NUM_PARENTS) ? 255 : 0);

/*          if (verbose)
            printf("%d: %d, %d, cur=%d, neighbors=%d next=%d\n", my_rank, i, j, cur[i * size + j], 
            neighbors, next[i * size + j]);*/
         } /* next j */
      } /* next i */
   }

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[END][CALCULATE], 0, "end calculate")))
      MPIERR(ret);
#endif

   return 0;
}

/* Write the game data for the current generation to a PGM output
 * file, which can be understood by many programs - GIMP, for
 * example. */
int
write_output(int p, int size, int my_rank, int s, int ln, MPI_Datatype filetype, 
             MPI_Datatype memtype, int verbose, int event_num[][NUM_EVENTS], 
             unsigned char *cur)
{
   int header_bytes;
   MPI_File out_fh;
   char output_file[128], hdr[128];
   int ret;

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[START][WRITE], 0, "start write")))
      MPIERR(ret);
#endif

   /* Delete and then create output file. */
   sprintf(output_file, "ann/out_%d_%d.pgm", p, s);
   if (verbose && !my_rank)
      printf("output %s, s=%d\n", output_file, s);
   MPI_File_delete(output_file, MPI_INFO_NULL);
   if ((ret = MPI_File_open(MPI_COMM_WORLD, output_file, MPI_MODE_CREATE|MPI_MODE_RDWR, 
                            MPI_INFO_NULL, &out_fh)))
      MPIERR(ret);

   /* Create header info, and have process 0 write it to the file. */
   sprintf(hdr, "P5\n%d %d\n255\n", size, size);
   header_bytes = strlen(hdr);
   if ((ret = MPI_File_write_all(out_fh, hdr, header_bytes, MPI_BYTE, MPI_STATUS_IGNORE)))
      MPIERR(ret);
         
   /* Set the file view to translate our memory data into the file's data layout. */
   MPI_File_set_view(out_fh, header_bytes, MPI_BYTE, filetype, "native", MPI_INFO_NULL);

   /* Write the output. */
   MPI_File_write_all(out_fh, cur, 1, memtype, MPI_STATUS_IGNORE);
   if ((ret = MPI_File_close(&out_fh)))
      MPIERR(ret);

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[END][WRITE], 0, "end write")))
      MPIERR(ret);
#endif

   return 0;
}

/* Move on to the next generation. */
int
swap_buffers(int ln, int size, int checkerboard, int event_num[][NUM_EVENTS], 
             unsigned char **cur, unsigned char **next)
{
   unsigned char *temp;
   int i;
#ifdef LOGGING
   int ret;
#endif

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[START][SWAP], 0, "start swap")))
      MPIERR(ret);
#endif

   /* Swap the buffers. */
   temp = *cur;
   *cur = *next;
   *next = temp;

   /* Reinitialize for the next generation. */
   if (checkerboard)
      for (i = 0; i < (ln + 2) * (ln + 2); i++)
         (*next)[i] = 0;
   else
      for (i = 0; i < (ln + 2) * size; i++)
         (*next)[i] = 0;

#ifdef LOGGING
      if ((ret = MPE_Log_event(event_num[END][SWAP], 0, "end swap")))
         MPIERR(ret);
#endif

   return 0;
}

/* We need two grids, one for the current generation, and one for the
 * next generation. The size of the local buffers depends on the total
 * size of the playing field (size), the number of processors (n), and
 * the data decomposition method. This function determine sqrtn, ln,
 * and allocates the cur and next buffers. */   
int
init_grid(int my_rank, int n, int size, int checkerboard, int *sqrtn, int *ln, 
          unsigned char **cur, unsigned char **next)
{
   int buf_size;

   /* Determine local grid size. */
   if (checkerboard)
   {
      /* What is the length of a side of the local square of data? */
      *sqrtn = (int)sqrt(n);
      if (*sqrtn != sqrt(n))
         return ERR_ARG;
      *ln = size/(*sqrtn);
      
      /* Extra space for ghost rows and colums. */
      buf_size = (*ln + 2) * (*ln + 2);
   }
   else
   {
      /* How many rows in this block? */
      *ln = size / n;

      /* Extra space for ghost rows. */
      buf_size = (*ln + 2) * size;
   }

   /* We will need two grids, one for the current timestep, one for
    * the next timestep. Using calloc causes all ghost rows (and
    * columns) to be initialized to zero. */
   if (!(*cur = calloc(buf_size, 1)))
      return ERR_DUMB;
   if (!(*next = calloc(buf_size, 1)))
      return ERR_DUMB;

   return 0;
}

int
main(int argc, char* argv[]) 
{
   int p, my_rank;
   int n = 1, size = 4, verbose = 0, num_steps = 1, checkerboard = 0, count = 0;
   int file_type = 0, output = 0, performance = 0, header = 0;
   int ln, c;
   unsigned char *cur, *next;
   char input_file[MAX_NAME + 1] = {""};
   MPI_Datatype col_type;
   int sqrtn;
   int event_num[2][NUM_EVENTS];
   MPI_Datatype filetype, memtype;
   double time, elapsed_time;
/*   double init_time, init_start_time;*/
   int total;
   int s;
   int ret;

   /* Initialize MPI. */
   MPI_Init(&argc, &argv);
   MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

   /* Learn my rank and the total number of processors. */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &p);

/*   if (!my_rank)
     init_start_time = MPI_Wtime();*/

   /* Parse command line. 
    v - verbose output
    c - count the number of live cells after each iteration
    k - use checkerboard decomposition (instead of row) 
    s - size of side of board 
    n - number of threads
    i - input file
    t - number of timesteps
    f - file type
    o - output file name
    p - turn on performance monitoring 
    h - including header
   */
   while ((c = getopt(argc, argv, "vc:ks:n:i:t:foph")) != -1)
      switch (c)
      {
         case 'v':
            verbose++;
            break;
         case 'c':
            sscanf(optarg, "%d", &count);
            break;
         case 'k':
            checkerboard++;
            break;
         case 's':
            sscanf(optarg, "%d", &size);
            break;
         case 'n':
            sscanf(optarg, "%d", &n);
            break;
         case 'i':
            sscanf(optarg, "%s", input_file);
            break;
         case 't':
            sscanf(optarg, "%d", &num_steps);
            break;
         case 'f':
            file_type++;
            break;
         case 'o':
            output++;
            break;
         case 'p':
            performance++;
            break;
         case 'h':
            header++;
            break;
         case '?':
            fprintf (stderr, "gol -v -o -f -c [count_interations] -k -s [size_of_side] "
            "-n [num_tasks] -i [input_file] -t [num_steps]\n");
            return ERR_ARG;
         default:
            break;
      }
   
#ifdef LOGGING
   if ((ret = MPE_Init_log()))
       ERR(ret);
   if (init_logging(my_rank, event_num))
      ERR(ERR_LOGGING);

   if ((ret = MPE_Log_event(event_num[START][INIT], 0, "start init")))
      MPIERR(ret);
#endif

   if (init_grid(my_rank, n, size, checkerboard, &sqrtn, &ln, &cur, &next))
      ERR(ERR_INIT);

   if (verbose && !my_rank)
      printf("n=%d size=%d input=%s num_steps=%d ln=%d checkboard=%d\n", 
      n, size, input_file, num_steps, ln, checkerboard);

   /* Create a column MPI type to send columns of ln+2 length for the
    * checkeboard data decomposition. */
   if (checkerboard)
   {
      if ((ret = MPI_Type_vector(ln + 2, 1, ln + 2, MPI_BYTE, &col_type)))
         MPIERR(ret);
      if ((ret = MPI_Type_commit(&col_type)))
         MPIERR(ret);
   }

   /* These types are used for reads when MPI types are used, and also
    * for output. */
   if (file_type || output)
      if (create_mpi_types(my_rank, n, size, ln, checkerboard, 
                           &filetype, &memtype))
         ERR(ERR_INIT);

   /* Initialize the starting configuration. Either read a file or
    * just turn on half the first row. */
   if (verbose && ! my_rank)
      printf("data initilization\n");
   if ((ret = init_cur(n, my_rank, p, size, ln, cur, input_file, count, 
                       checkerboard, verbose, event_num, file_type, &filetype, &memtype)))
      ERR(ret);
   if (count)
   {
      if (count_results(my_rank, count, ln, size, checkerboard, event_num, verbose, cur, &total))
         ERR(ERR_COUNT);
      if (verbose && !my_rank)
         printf("initial count - total %d\n", total);
   }
/*   init_time = MPI_Wtime() - init_start_time;*/
/*   printf("init time - %f\n", init_time);
     return 0;*/

   if (verbose && ! my_rank)
      printf("initilization complete\n");

#ifdef LOGGING
   if ((ret = MPE_Log_event(event_num[END][INIT], 0, "end init")))
      MPIERR(ret);
#endif

   /* Play the game of life! Sends and receives depend on reasonable
    * buffering of MPI. */
   if (!my_rank && performance)
      time = MPI_Wtime();
   for (s = 0; s < num_steps; s++)
   {
      if (update_processes(n, my_rank, p, size, ln, sqrtn, checkerboard, 
                           verbose, col_type, event_num, cur))
         ERR(ERR_UPDATE);

      if (calculate_next_step(checkerboard, ln, size, event_num, cur, next))
         ERR(ERR_CALC);

      if (count)
      {
	 if (!((s + 1) % count))
	    if (count_results(my_rank, count, ln, size, checkerboard, event_num, verbose, next, &total))
	       ERR(ERR_COUNT);
	 if (!my_rank)
	    printf("after step: %d total: %d\n", s, total);
      }

      if (output)
	 if (write_output(p, size, my_rank, s, ln, filetype, memtype, verbose, event_num, next))
            ERR(ERR_WRITE);

      if (swap_buffers(ln, size, checkerboard, event_num, &cur, &next))
         ERR(ERR_SWAP);
   } /* next s */

   /* Wait for everyone to get performance. */
   if (performance)
      if ((ret = MPI_Barrier(MPI_COMM_WORLD)))
         MPIERR(ret);

   if (!my_rank && performance)
   {
      if (header)
         printf("n, p, cb, size, avg time\n");
      elapsed_time = MPI_Wtime() - time;
      printf("%d, %d, %d, %d, %f\n", n, p, checkerboard, size, elapsed_time/num_steps);
   }

   /* Fold our tents. */
   free(cur);
   free(next);

/* #ifdef LOGGING */
/*    { */
/*        /\* This causes problems on my MPICH2 library on Linux, but seems to be */
/* 	* required for frost. *\/ */
/*        char file_name[128]; */
/*        sprintf(file_name, "chart_%d_%d", n, checkerboard ? 1 : 0); */
/*        if ((ret = MPE_Finish_log(file_name))) */
/* 	   MPIERR(ret); */
/*    } */
/* #endif */

/*   printf("my_rank=%d about to finalize.\n", my_rank);*/

   MPI_Finalize();
   return 0;
}




