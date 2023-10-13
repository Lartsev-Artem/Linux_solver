#include "graph_main.h"
#include "illum_main.h"
#include "mpi_ext.h"
#include "reader_json.h"
#include "trace_main.h"

#include <unistd.h>

#include "solvers_struct.h"

#include "ray_tracing_main.h"
#include "rhllc_main.h"

int main(int argc, char *argv[]) {

  MPI_START(argc, argv);

  files_sys::json::ReadStartSettings("/home/artem/projects/solver/config/directories_cfg.json",
                                     glb_files, &_solve_mode, &_hllc_cfg);

#ifdef ILLUM
  graph::RunGraphModule();
  trace::RunTracesModule();
  illum::RunIllumModule();
// GDB_ATTACH;
#endif

  ray_tracing::RunRayTracing(glb_files.solve_address + "0" + F_ENERGY);

  //  rhllc::RunRhllcModule();

  MPI_END;
  return 0;
}

#ifdef OST_BIN_READER

//#if defined HLLC || defined RHLLC
template <typename T>
int ReadValueGrid(const T main_dir, grid_t &grid) {
  READ_FILE(std::string(main_dir + "phys_val.bin").c_str(), grid.cells,
            phys_val);
  return 0;
}
template <typename T>
int ReadHllcInit(const T file_init_value, std::vector<elem_t> &cells) {
  READ_FILE(std::string(file_init_value).c_str(), cells, phys_val);
  return 0;
}
#endif