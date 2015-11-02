/* Test program for MPI file types.

   Ed Hartnett, 10/27/08
   $Id: test_file_type.c,v 1.1 2008/10/30 20:57:18 edhartnett Exp $
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

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
#define NDIMS 2
#define FILE_NAME "dumb_file.pgm"

int
main(int argc, char* argv[]) 
{
   int n, my_rank;
   int array_of_subsizes[NDIMS], array_of_starts[NDIMS], array_of_sizes[NDIMS];
   int size = 4;
   int sqrtn;
   int ln;
   MPI_Datatype filetype, memtype;
   MPI_File fh;
   char hdr[128];
   int header_bytes;
   unsigned char *cur;
   char name[128];
   int resultlen;
   int ret;
   int i, j;

   /* Initialize MPI. */
   MPI_Init(&argc, &argv);
   MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

   /* Learn my rank and the total number of processors. */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &n);

   /* Speak! */
   MPI_Get_processor_name(name, &resultlen);
   printf("process %d running on %s\n", my_rank, name);

   /* Set up our values. */
   sqrtn = (int)sqrt(n);
   ln = size/sqrtn;
   printf("n = %d, sqrtn = %d, ln = %d storage = %d\n", n, sqrtn, ln, (ln + 2) * (ln + 2));

   /* Allocation storage. */
   if (!(cur = calloc((ln + 2) * (ln + 2), 1)))
      return ERR;

   /* Initialize data. */
   for (i = 1; i < ln + 1; i++)
      for (j = 1; j < ln + 1; j++)
	 cur[i * (ln + 2) + j] = my_rank;

   /* Create a subarray type for the file. */
   array_of_sizes[0] = array_of_sizes[1] = size;
   array_of_subsizes[0] = array_of_subsizes[1] = ln;
   array_of_starts[0] = my_rank/sqrtn * ln;
   array_of_starts[1] = (my_rank % sqrtn) * ln;
   if ((ret = MPI_Type_create_subarray(NDIMS, array_of_sizes, array_of_subsizes, array_of_starts, MPI_ORDER_C, MPI_BYTE, &filetype)))
      MPIERR(ret);
   if ((ret = MPI_Type_commit(&filetype)))
      MPIERR(ret);

   /* Create a subarray type for memory. */
   array_of_sizes[0] = array_of_sizes[1] = ln + 2;
   array_of_subsizes[0] = array_of_subsizes[1] = ln;
   array_of_starts[0] = array_of_starts[1] = 1;
   if ((ret = MPI_Type_create_subarray(NDIMS, array_of_sizes, array_of_subsizes, array_of_starts, MPI_ORDER_C, MPI_BYTE, &memtype)))
      MPIERR(ret);
   if ((ret = MPI_Type_commit(&memtype)))
      MPIERR(ret);

   MPI_File_delete(FILE_NAME, MPI_INFO_NULL);
   if ((ret = MPI_File_open(MPI_COMM_WORLD, FILE_NAME, MPI_MODE_CREATE|MPI_MODE_RDWR, 
        MPI_INFO_NULL, &fh)))
      MPIERR(ret);

   /* Create header info, and have process 0 write it to the file. */
   sprintf(hdr, "P5\n%d %d\n255\n", size, size);
   header_bytes = strlen(hdr);
   if ((ret = MPI_File_write_all(fh, hdr, header_bytes, MPI_BYTE, MPI_STATUS_IGNORE)))
      MPIERR(ret);
	 
   /* Set the file view to translate our memory data into the file's data layout. */
   MPI_File_set_view(fh, header_bytes, MPI_BYTE, filetype, "native", MPI_INFO_NULL);

   /* Write the output. */
   MPI_File_write(fh, cur, 1, memtype, MPI_STATUS_IGNORE);

   if ((ret = MPI_File_close(&fh)))
      MPIERR(ret);

   MPI_Finalize();
   return 0;
}
