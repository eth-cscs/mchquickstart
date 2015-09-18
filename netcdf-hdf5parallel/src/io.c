/*
 Benchmark for parallel I/O
 Written by Jean M. Favre, modified by Luca Marsella (Swiss National Supercomputing Center)
 Tested and compiled on Cray XC30 and XC40 with the following modules:
 module load PrgEnv-cray (-intel, -gnu, -pgi)
 module load netcdf-hdf5parallel
*/
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <mpi.h>

#define ZONAL_DATA 1
#define SIM_STOPPED 0
#define SIM_RUNNING 1

typedef float mytype;
/* number of arrays of scalar variables (all float) */
const int NFIELDS = 1;
int BlockSize = 64;
int NUMBER_OF_ITERATIONS = 1;
const char *cmd_names[] = {"halt", "step", "run", "Update ON/OFF", "Save ON/OFF"};
struct tms mpitms;
double mpi_elapsed;

typedef struct datagrid{
    int    Nrows;     /* local size of data array */
    int    Ncolumns;  /* local size of data array */
    int    Nlevels;   /* local size of data array */
    float *rmesh_x;
    float *rmesh_y;
    float *rmesh_z;
    int    rmesh_dims[3];
    int    rmesh_ndims;
    mytype **data;
} datagrid;

typedef struct simulation_data{
    int       cycle;
    double    time;
    int       runMode;
    int       updateplots;
    int       done;
    int       savingFiles;
    int       saveCounter;
    int       par_rank;
    int       par_size;
    int       global_dims[3];
    /* MPI topological grid of size 2 */
    int       coords[2];
    char      filename[256];
    MPI_Comm  comm_cart;
    datagrid grid;
} simulation_data;

int get_procmem(double *bytes){
  FILE *fh;
  int proc_ret;
  char proc_var[80];
  char *cp;
  long long int ibytes;

#ifndef max
  #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
  *bytes=0.0;
  fh = fopen("/proc/self/status","r");
  while(!feof(fh)){
    fgets(proc_var,80,fh);
    cp = strstr(proc_var,"VmHWM:");
    if (cp) {sscanf(cp, "VmHWM:"" %llu",&ibytes );
      *bytes=max(*bytes,ibytes);
    }
  }
  fclose(fh);
  *bytes *= 1024.0;
   return 0;
}

#include "netcdf_write.c"

void MPIIO_WriteData(simulation_data *sim, char *Filename){
  int dimuids[3]={sim->global_dims[2], sim->global_dims[1], sim->global_dims[0]};
  int rc, ustart[3], ucount[3];
  MPI_Offset disp = 0;
  MPI_File      filehandle;
  MPI_Datatype  filetype;

  rc = MPI_File_open(sim->comm_cart, Filename,
                MPI_MODE_CREATE | MPI_MODE_WRONLY,
                MPI_INFO_NULL, &filehandle);

  ustart[2] = sim->grid.Ncolumns * sim->coords[0];
  ustart[1] = sim->grid.Nrows * sim->coords[1];
  ustart[0] = 0;
  ucount[2] = sim->grid.Ncolumns;
  ucount[1] = sim->grid.Nrows;
  ucount[0] = sim->grid.Nlevels;

  /* Create the subarray representing the local block */
  MPI_Type_create_subarray(3, dimuids, ucount, ustart,
                                        MPI_ORDER_C, MPI_FLOAT, &filetype);
  MPI_Type_commit(&filetype);

  MPI_File_set_view(filehandle, disp, MPI_FLOAT,
                                filetype, "native", MPI_INFO_NULL);

  MPI_File_write_all(filehandle, sim->grid.data[0],
                     ucount[0]*ucount[1]*ucount[2],
                     MPI_FLOAT, MPI_STATUS_IGNORE);
  MPI_File_close(&filehandle);
  MPI_Type_free(&filetype);
}

void grid_data_ctor(int par_size, datagrid *grid){
  int f;
  grid->Ncolumns = BlockSize; /* LONGITUDE_SIZE */
  grid->Nrows    = BlockSize; /* LATITUDE_SIZE */
  grid->Nlevels  = BlockSize; /* LEVELS_SIZE */
  grid->rmesh_x = NULL;
  grid->rmesh_y = NULL;
  grid->rmesh_z = NULL;
  grid->rmesh_dims[0] = 1; /* shall be redefined later after Parallel init*/
  grid->rmesh_dims[1] = 1;
  grid->rmesh_dims[2] = 1;
  grid->rmesh_ndims = 3;

  grid->data = (float**)malloc(sizeof(float*) * NFIELDS);
  for(f=0; f < NFIELDS; f++){
    grid->data[f] = NULL;
  }
}

void grid_data_dtor(datagrid *grid){
  int f;
  if(grid->rmesh_x != NULL){
    free(grid->rmesh_x);
    grid->rmesh_x = NULL;
  }
  if(grid->rmesh_y != NULL){
    free(grid->rmesh_y);
    grid->rmesh_y = NULL;
  }
  if(grid->rmesh_z != NULL){
    free(grid->rmesh_z);
    grid->rmesh_z = NULL;
  }
  for(f=0; f < NFIELDS; f++){
    if(grid->data[f] != NULL){
      free(grid->data[f]);
      grid->data[f] = NULL;
    }
  }
  free(grid->data);
  grid->data = NULL;
}

/* 
   3D Rectilinear mesh of the size below is built and distributed among MPI tasks
   sim->global_dims[0] * sim->global_dims[1] * sim->global_dims[2] 
*/
void grid_data_allocate(simulation_data *sim){
  int i, size;
  datagrid *grid = &(sim->grid);
  int *coords = sim->coords;
  float offset, zone_width;

  grid->rmesh_dims[0] = grid->Ncolumns+1; // size of X local coordinate array
  grid->rmesh_dims[1] = grid->Nrows+1;    // size of Y local coordinate array
  grid->rmesh_dims[2] = grid->Nlevels+1;  // size of Z local coordinate array

  grid->rmesh_x = (float*)malloc(sizeof(float) * grid->rmesh_dims[0]);
  grid->rmesh_y = (float*)malloc(sizeof(float) * grid->rmesh_dims[1]);
  grid->rmesh_z = (float*)malloc(sizeof(float) * grid->rmesh_dims[2]);

  offset = grid->Ncolumns*coords[0];
  zone_width = 1.0/sim->global_dims[0];
  for(i=0; i < grid->rmesh_dims[0]; i++)
  {
      grid->rmesh_x[i] = (offset + i)*zone_width;
  }

  offset = grid->Nrows*coords[1];
  zone_width = 1.0/sim->global_dims[1];
  for(i=0; i < grid->rmesh_dims[1]; i++){
      grid->rmesh_y[i] = (offset + i)*zone_width;
  }
  zone_width = 1.0/sim->global_dims[2];
  for(i=0; i < grid->rmesh_dims[2]; i++){
      grid->rmesh_z[i] = i * zone_width;
  }
  /* 3D array of data items exposed as solution. Data centered at cells*/
#ifdef ZONAL_DATA
  size = sizeof(float) * (grid->Ncolumns) * (grid->Nrows) * (grid->Nlevels);
#else
  size = sizeof(float) * (grid->Ncolumns+1) * (grid->Nrows+1) * (grid->Nlevels+1);
#endif
  for(i=0; i < NFIELDS; i++){
    grid->data[i] = (float*)malloc(size);
  }
}

void data_simulate(datagrid *grid, int par_rank, int par_size, double T){
  int nsum, f, i, j, k;
  int *data = NULL;

#ifdef ZONAL_DATA
  for(k=0; k< grid->Nlevels; k++){
    for(j=0; j< grid->Nrows; j++){
      for(i=0; i < grid->Ncolumns; i++){
        int idx = k*(grid->Ncolumns)*(grid->Nrows) + j*(grid->Ncolumns) + i;
        grid->data[0][idx] = cos((grid->rmesh_y[j] + T)*3.1415) * sin((grid->rmesh_x[i] + T)*3.1415);
/* -90 < Y < 90, -pi/2 < arg < pi/2, 0 < cos() < 1, -1 < sin() < 1 */
/*
        grid->data[1][idx] = cos((grid->rmesh_y[j]/180.0 + T)*3.1415) * sin((grid->rmesh_x[i]/360.0 + T)*3.1415); 
        grid->data[2][idx] = cos(grid->rmesh_y[j]*3.1415/360.0);
        grid->data[3][idx] = sin(grid->rmesh_x[i]*3.1415/360.0);
*/
        for(f=1; f < NFIELDS; f++){
          grid->data[f][idx] = grid->rmesh_x[i] * grid->rmesh_x[i] -
                               grid->rmesh_y[j] * grid->rmesh_y[j] -
                               grid->rmesh_x[i] * grid->rmesh_y[j] * grid->rmesh_z[k];
        }
      }
    }
  }
#else
  for(k=0; k< grid->Nlevels+1; k++){
    for(j=0; j< grid->Nrows+1; j++){
      for(i=0; i < grid->Ncolumns+1; i++){
        int idx = k*(grid->Ncolumns+1)*(grid->Nrows+1) + j*(grid->Ncolumns+1) + i;
        grid->data[0][idx] = grid->rmesh_x[i] * grid->rmesh_x[i] -
                             grid->rmesh_y[j] * grid->rmesh_y[j] -
                             grid->rmesh_x[i] * grid->rmesh_y[j] * grid->rmesh_z[k];
/* -90 < Y < 90, -pi/2 < arg < pi/2, 0 < cos() < 1, -1 < sin() < 1 */
/*
        grid->data[1][idx] = cos((grid->rmesh_y[j]/180.0 + T)*3.1415) * sin((grid->rmesh_x[i]/360.0 + T)*3.1415); 
        grid->data[2][idx] = cos(grid->rmesh_y[j]*3.1415/360.0);
        grid->data[3][idx] = sin(grid->rmesh_x[i]*3.1415/360.0);
*/
       for(f=1; f < NFIELDS; f++){
          grid->data[f][idx] = cos((grid->rmesh_y[j] + T)*3.1415) * sin((grid->rmesh_x[i] + T)*3.1415);
        }
      }
    }
  }
#endif
}

void simulation_data_ctor(simulation_data *sim){
  sim->updateplots = 1;
  sim->cycle = 0;
  sim->time = 0.;
  sim->done = 0;
  sim->savingFiles = 0;
  sim->saveCounter = 0;
  sim->par_rank = 0;
  sim->par_size = 1;
  strcpy(sim->filename, "null");
  grid_data_ctor(sim->par_size, &sim->grid);
}

void simulation_data_dtor(simulation_data *sim){
  grid_data_dtor(&(sim->grid));
}

void DumpData(simulation_data *sim, char *Filename){
#ifdef ADIOS
  if(sim->cycle == 1){
    adios_init ("benchmark.xml");
  }
  ADIOS_WriteData(sim, Filename);
  if(sim->cycle == NUMBER_OF_ITERATIONS){
    adios_finalize (sim->par_rank);
  }
#elif NETCDF4
  NETCDF4_WriteData(sim, Filename);
#elif HDF5
  HDF5_WriteData(sim, Filename);
#else
  MPIIO_WriteData(sim, Filename);
#endif
}

void simulate_one_timestep(simulation_data *sim){
  ++sim->cycle;
    if(sim->cycle == NUMBER_OF_ITERATIONS){
      sim->done = 1;
    }
  sim->time += .01;

  /* Simulate the current round of grid */
  data_simulate(&sim->grid, sim->par_rank, sim->par_size, sim->time);
}

void mainloop(simulation_data *sim){
  int blocking, visitstate, err = 0;
  char Filename[256];

  do{
    simulate_one_timestep(sim);
#ifdef ADIOS
    sprintf(Filename, "%s.%04d.bp", sim->filename, sim->saveCounter++);
#elif NETCDF4
 #ifdef PARIO
    sprintf(Filename, "%s.%04d_p.nc", sim->filename, sim->saveCounter++);
 #else
    sprintf(Filename, "%s.%04d_s.nc", sim->filename, sim->saveCounter++);
 #endif
#elif HDF5
    sprintf(Filename, "%s.%04d.h5", sim->filename, sim->saveCounter++);
#else
    sprintf(Filename, "%s.%04d.bin", sim->filename, sim->saveCounter++);
#endif
    DumpData(sim, Filename);

    if(sim->cycle == NUMBER_OF_ITERATIONS){
      sim->done = 1;
    }
  } while(!sim->done && err == 0);
}

int main(int argc, char **argv){
  char *env = NULL;
  simulation_data sim;
  simulation_data_ctor(&sim);
  sim.runMode = SIM_STOPPED;
  int r, c, ndims=2, dims[2]={8,8}, periods[2]={0,0};
  int xdivs=2, ydivs=2, zdivs=1;
  double before, after, *mem;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &sim.par_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &sim.par_size);
  if(!sim.par_rank)
    get_procmem(&before);

#define OPTS 1
#ifdef OPTS
  if(!sim.par_rank){
    while(1){
      static struct option long_options[] =
        { 
          {"xdivs",  required_argument, 0, 'x'},
          {"ydivs",  required_argument, 0, 'y'},
          {"blocksize",  required_argument, 0, 'b'},
          {"timesteps",  required_argument, 0, 't'},
          {"file",    required_argument, 0, 'f'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;
     
      c = getopt_long (argc, argv, "x:y:b:t:f:", long_options, &option_index);
     
      /* Detect the end of the options. */           
      if (c == -1)
        break;
                
      switch (c){
        case 0:
        /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
            printf ("\n");
            break;
     
        case 'x':
          dims[0] = atoi(optarg);
          break;
     
        case 'y':
          dims[1] = atoi(optarg);
          break;

        case 'b':
          BlockSize = atoi(optarg);
          break;

        case 't':
          NUMBER_OF_ITERATIONS = atoi(optarg);
          break;
     
        case 'f':
          strcpy(sim.filename, optarg);
          break;
     
        case '?':
          /* getopt_long already printed an error message. */
          break;
     
        default:
          abort ();
      }
    }
  }
#endif
  MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&BlockSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&NUMBER_OF_ITERATIONS, 1, MPI_INT, 0, MPI_COMM_WORLD);
  /* broadcast name, even if it was not set */
  MPI_Bcast(sim.filename, 256, MPI_CHAR, 0, MPI_COMM_WORLD);

  if(sim.par_size != dims[0]*dims[1]){
    if(sim.par_rank == 0)
      fprintf(stderr,"The cartesian topology requested (%d nodes) does not allow to map all tasks. Resubmit\n", sim.par_size);
    MPI_Finalize();
    exit(1);
  }

  MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, 1, &sim.comm_cart);
  MPI_Cart_coords(sim.comm_cart, sim.par_rank, 2, sim.coords);

  /* Adjust the partitioning */
  sim.grid.Ncolumns = BlockSize; /* LONGITUDE_SIZE; assume they divide evenly */
  sim.global_dims[0] = BlockSize * dims[0];

  sim.grid.Nrows = BlockSize; /* LATITUDE_SIZE; assume they divide evenly */
  sim.global_dims[1] = BlockSize * dims[1];

  sim.global_dims[2] = BlockSize; /* LEVELS_SIZE */
  sim.grid.Nlevels = BlockSize;

  grid_data_allocate(&sim);

  mpi_elapsed = MPI_Wtime();
  mainloop(&sim);
  mpi_elapsed = MPI_Wtime() - mpi_elapsed;

  grid_data_dtor(&sim.grid);

  if(!sim.par_rank){
    get_procmem(&after);
    printf("Testing get_procmem... %f %f %f\n",
          before, after, after-before);
  }

  MPI_Barrier(sim.comm_cart);
 
  if(sim.par_rank == 0){
    printf("written grids of %d,%d,%d\n",  BlockSize, BlockSize, BlockSize);
    printf("written %d iterations\n", NUMBER_OF_ITERATIONS);
    printf("MPI elapsed time: %f s\n", mpi_elapsed);
    printf("Average performance: %f GB/s\n", (BlockSize*BlockSize*BlockSize*4.0*sim.par_size*NUMBER_OF_ITERATIONS) / (1024*1024*1024*mpi_elapsed));
  }

  MPI_Finalize();

  return 0;
}
