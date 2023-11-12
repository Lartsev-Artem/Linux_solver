
#if !defined ILLUM_PART_H && defined ILLUM && defined SOLVERS
#define ILLUM_PART_H
#include "geo_types.h"
#include "solvers_struct.h"

namespace illum {

/**
 * @brief Пространство имён расчёта излучения на  видеокарте с асинхронной пересылкой между процессами
 *
 */
namespace gpu_async {

void InitSender(const grid_directions_t &grid_dir, const grid_t &grid);

int CalculateIllum(const grid_directions_t &grid_direction, const std::vector<std::vector<State>> &face_states, const std::vector<int> &pairs,
                   const std::vector<std::vector<IntId>> &inner_bound_code,
                   const std::vector<std::vector<cell_local>> &vec_x0, std::vector<BasePointTetra> &vec_x,
                   const std::vector<std::vector<int>> &sorted_id_cell, grid_t &grid);

void NewInitSender(const grid_directions_t &grid_dir, const grid_t &grid);

int NewCalculateIllum(const grid_directions_t &grid_direction, const std::vector<std::vector<State>> &face_states, const std::vector<int> &pairs,
                      const std::vector<std::vector<IntId>> &inner_bound_code,
                      const std::vector<std::vector<cell_local>> &vec_x0, std::vector<BasePointTetra> &vec_x,
                      const std::vector<std::vector<int>> &sorted_id_cell, grid_t &grid);

} // namespace gpu_async
} // namespace illum
#endif