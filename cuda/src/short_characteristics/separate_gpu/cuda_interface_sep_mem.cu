#include "cuda_init_mem.h"
#include "cuda_memory.h"
#include "cuda_multi_init.h"
#include "cuda_multi_interface.h"

#include "mpi_shifts.h"

#ifdef SEPARATE_GPU
cuda::multi_gpu_config_t gpu_config;

std::vector<cuda::geo::grid_directions_device_t *> grid_dir_deviceN;
std::vector<cuda::geo::grid_device_t *> grid_deviceN;
std::vector<cuda::geo::device_host_ptr_t> device_host_ptrN;

int cuda::interface::separate_device::InitDevice(const grid_directions_t &grid_dir_host, grid_t &grid_host) {

  CUDA_CALL_FUNC(cudaGetDeviceCount, &gpu_config.GPU_N);
#ifdef SINGLE_GPU
  gpu_config.GPU_N = GPU_DIV_PARAM;
#endif
  GetSend(gpu_config.GPU_N, grid_host.size, gpu_config.size);
  GetDisp(gpu_config.GPU_N, grid_host.size, gpu_config.disp);

  int _dev_n = gpu_config.GPU_N; //реальное число карт
#ifdef SINGLE_GPU
  _dev_n = 1;
#endif

  device_host_ptrN.resize(_dev_n);
  grid_dir_deviceN.resize(_dev_n);
  grid_deviceN.resize(_dev_n);

  for (int dev_id = 0; dev_id < _dev_n; dev_id++) {
    CUDA_CALL_FUNC(cudaSetDevice, dev_id);
    cuda::separate_device::InitDirectionsOnMultiDevice(grid_dir_host, device_host_ptrN[dev_id], grid_dir_deviceN[dev_id]);
    cuda::separate_device::InitMultiDeviceGrid(dev_id, gpu_config, grid_host, grid_dir_host, device_host_ptrN[dev_id], grid_deviceN[dev_id]);
  }

  /// \todo это отдельно, т.к. относится к инициализации хоста
  mem_protected::MallocHost((grid_dir_host.size * grid_host.size * sizeof(Type)), &grid_host.Illum);
  mem_protected::MallocHost(((grid_dir_host.loc_size) * grid_host.size * sizeof(Type)), &grid_host.scattering);

  if (grid_dir_host.loc_shift == 0 || (grid_host.size != grid_host.loc_size)) // или нулевой узел, или данные разделены
  {
    mem_protected::MallocHost((grid_host.size * sizeof(Type)), &grid_host.energy);
    mem_protected::MallocHost((grid_host.size * sizeof(Vector3)), &grid_host.stream);
    mem_protected::MallocHost((grid_host.size * sizeof(Matrix3)), &grid_host.impuls);

    // //сопоставимо с хранением energy и stream и impuls
    // const Type abs_op = grid.cells[cell].illum_val.absorp_coef;
    // const Type scat_op = grid.cells[cell].illum_val.scat_coef;
    // const Type rho = grid.cells[cell].phys_val.d;
    // const Type prs = grid.cells[cell].phys_val.p;
    // const Vector3 &v = grid.cells[cell].phys_val.v;
  }

  return e_completion_success;
}

void cuda::interface::separate_device::ClearDevice() {

  cuda::separate_device::ClearDirectionsOnMultiDevice(gpu_config, device_host_ptrN, grid_dir_deviceN);
  cuda::separate_device::ClearGridOnMultiDevice(gpu_config, device_host_ptrN, grid_deviceN);

  // CUDA_CALL_FUNC(cudaDeviceReset);

  WRITE_LOG("Free device arrays\n");
}
void cuda::interface::separate_device::ClearHost(grid_t &grid_host) {

  mem_protected::FreeMemHost(grid_host.Illum);
  mem_protected::FreeMemHost(grid_host.scattering);

#ifdef ON_FULL_ILLUM_ARRAYS
  mem_protected::FreeMemHost(grid_host.energy);
  mem_protected::FreeMemHost(grid_host.stream);
  mem_protected::FreeMemHost(grid_host.impuls);
#endif

  WRITE_LOG("Free host arrays\n");
}

#endif //! SEPARATE_GPU