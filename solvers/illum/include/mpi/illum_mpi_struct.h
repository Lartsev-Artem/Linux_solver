/**
 * @file illum_mpi_struct.h
 * @brief Новые типы данных MPI
 *
 */
#if !defined ILLUM_MPI_STRUCT_H && defined ILLUM && defined SOLVERS && defined USE_MPI
#define ILLUM_MPI_STRUCT_H

#include "geo_types.h"
#include "solvers_struct.h"

/*! \addtogroup illum Модуль расчёта излучения
    @{
*/

extern MPI_Datatype MPI_RECV_ILLUM_T; ///< структура дл отправки по направлению, а приему по ячейкам
extern MPI_Datatype MPI_SUB_ARRAY_RECV_T;
namespace illum {

/**
 * @brief Функция инициализирует тип данных со смешением на число направлений
 *
 * \note используется организации пересылок с отправкой хранения по направлениям и принятием по ячейкам
 * @param[in] grid сетка направлений
 */
void MpiInitStruct(const grid_directions_t &grid);

namespace spectrum {
/**
 * @brief Функция инициализирует тип данных со смешением на число направлений и частот
 *
 * \note используется организации пересылок с отправкой хранения по направлениям и принятием по ячейкам
 * @param[in] grid сетка
 */
void MpiInitStruct(const grid_t &grid);
} // namespace spectrum
} // namespace illum
#endif //! ILLUM_MPI_STRUCT_H