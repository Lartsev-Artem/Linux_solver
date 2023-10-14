#if defined ILLUM && defined SOLVERS // && !defined USE_MPI
#include "illum_calc_cpu.h"

#include "illum_params.h"
#include "illum_utils.h"
#include "scattering.h"

#include "solvers_struct.h"

#ifdef USE_CUDA
#include "cuda_interface.h"
#endif

#include <omp.h>

#include <chrono>
namespace tick = std::chrono;

int illum::cpu::CalculateIllum(const grid_directions_t &grid_direction, const std::vector<std::vector<bits_flag_t>> &face_states,
                               const std::vector<IntId> &neighbours,
                               const std::vector<std::vector<cell_local>> &vec_x0, std::vector<BasePointTetra> &vec_x,
                               const std::vector<std::vector<IntId>> &sorted_id_cell, grid_t &grid) {

  // вроде не обязательно. ПРОВЕРИТЬ
  // Illum.assign(4 * count_cells * count_directions, 0);
  // int_scattering.assign(count_cells * count_directions, 0);

  int iter = 0;    ///< номер итерации
  double norm = 0; ///< норма ошибки

  do {
    auto start_clock = tick::steady_clock::now();

    norm = -1;
    /*---------------------------------- далее FOR по направлениям----------------------------------*/
    const int count_directions = grid_direction.size;

#pragma omp parallel default(none) firstprivate(count_directions) shared(sorted_id_cell, neighbours, face_states, vec_x0, vec_x, grid, norm)
    {
      const int count_cells = grid.size;

      Type loc_norm = -1;
      std::vector<Vector3> *inter_coef = &grid.inter_coef_all[omp_get_thread_num()]; ///< указатель на коэффициенты интерполяции по локальному для потока направлению

#pragma omp for
      for (int num_direction = 0; num_direction < count_directions; ++num_direction) {

        const cell_local *X0_ptr = vec_x0[num_direction].data(); ///< индексация по массиву определяющих гранях (конвеерная т.к. заранее не известны позиции точек)

        /*---------------------------------- далее FOR по ячейкам----------------------------------*/
        for (int h = 0; h < count_cells; ++h) {

          const int num_cell = sorted_id_cell[num_direction][h];
          elem_t *cell = &grid.cells[num_cell];

          // расчитываем излучения на выходящих гранях
          for (ShortId num_out_face = 0; num_out_face < CELL_SIZE; ++num_out_face) {

            const int neigh_id = neighbours[num_cell * CELL_SIZE + num_out_face]; ///< сосед к текущей грани

            // если эта грань входящая и граничная, то пропускаем её
            if (CHECK_BIT(face_states[num_direction][num_cell], num_out_face) == e_face_type_in) {
              if (neigh_id < 0) {
                Type I0 = illum::BoundaryConditions(neigh_id);
                (*inter_coef)[num_cell * CELL_SIZE + num_out_face] = Vector3(I0, I0, I0); //значение на грани ( или коэффициенты интерполяции)
              }
              continue;
            }

            Vector3 I;
            // структура аналогичная  ::trace::GetLocNodes(...)
            for (int num_node = 0; num_node < 3; ++num_node) {

              Vector3 &x = vec_x[num_cell].x[num_out_face][num_node];
              ShortId num_in_face = X0_ptr->in_face_id;

              Type I_x0 = GetIllumeFromInFace(num_in_face, neighbours[num_cell * CELL_SIZE + num_in_face], cell, (*inter_coef)[num_cell * CELL_SIZE + num_in_face]);
              I[num_node] = GetIllum(x, X0_ptr->s, I_x0, grid.scattering[num_direction * count_cells + num_cell], *cell);

              X0_ptr++;
            } // num_node

            // если хранить не значения у коэфф. интерполяции
            {
                // Vector3 coef;
                // if (num_out_face == 3)
                //	coef = inclined_face_inverse * I;// GetInterpolationCoefInverse(inclined_face_inverse, I);
                // else
                //	coef = straight_face_inverse * I;// GetInterpolationCoefInverse(straight_face_inverse, I);
            }

            //записываем значение коэффициентов на грани
            (*inter_coef)[num_cell * CELL_SIZE + num_out_face] = I; // coef

            // если эта не граница, переносим это же значение входную грань соседней ячейки
            if (neigh_id >= 0) {
              (*inter_coef)[neigh_id] = I;
            }
          } // num_out_face
        }
        /*---------------------------------- конец FOR по ячейкам----------------------------------*/

        loc_norm = ReCalcIllum(num_direction, *inter_coef, grid);
      }
      /*---------------------------------- конец FOR по направлениям----------------------------------*/

      if (loc_norm > norm) {
#pragma omp critical
        {
          if (loc_norm > norm) {
            norm = loc_norm;
          }
        }
      }
    } // parallel

    if (_solve_mode.max_number_of_iter > 1) // пропуск первой итерации
    {
#ifndef USE_CUDA
      scattering::CalculateIntCPU(grid_direction, grid);
#else
      cuda::interface::CalculateIntScattering(grid_direction, grid);
#endif
    }

    WRITE_LOG("End iter number#: %d, norm=%.16lf, time= %lf\n", iter, norm,
              (double)tick::duration_cast<tick::milliseconds>(tick::steady_clock::now() - start_clock).count() / 1000.);
    iter++;
  } while (norm > _solve_mode.accuracy && iter < _solve_mode.max_number_of_iter);

  return e_completion_success;
}

void illum::cpu::CalculateIllumParam(const grid_directions_t &grid_direction, grid_t &grid) {

#ifndef USE_CUDA
  GetEnergy(grid_direction, grid);
  GetStream(grid_direction, grid);
  GetImpuls(grid_direction, grid);
#else
  cuda::interface::CalculateAllParam(grid_direction, grid);
#endif
}
#endif //! defined ILLUM && defined SOLVERS  && !defined USE_MPI