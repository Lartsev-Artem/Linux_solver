/**
 * @file plunk.h
 * @brief Алгоритмы расчёта функции Планка для расчёта слагаемого с излучением
 * @version 0.1
 * @date 2024-02-03
 * @todo  Возможно следует реализовать безразмерный расчёт
 *
 * @note Вещество при T=10000K при переходе в Рентген спектр (4*10^16) практически перестает поглощать
         (при меньшей температуре -> раньше) => при гамма лучах поглощения нет

 * @note Диапазоны спектра излучения
    Имя	                 Длина волны	            Частота
Гамма-луч               < 0,02 нм	            > 15 ЭГц
Рентгеновский	        0,01 нм — 10 нм	        30 ЭГц — 30 ПГц
Ультрафиолетовый	    10 нм — 400 нм	        30 ПГц — 750 ТГц
Видимый свет	        390 нм — 750 нм	        770 ТГц — 400 ТГц
Инфракрасный	        750 нм — 1 мм	        400 ТГц — 300 ГГц
Микроволны	            1 мм — 1 м	            300 ГГц — 300 МГц
Радио	                1 м — 100 км	        300 МГц — 3 кГц

/--------------------------Видимый спектр-------------------------------------/
Рентгеновское 0.01 - 100
УФ-излучения  100 - 400
Фиолетовый (сине-фиолетовый) 390-440
Синий 440-480
Голубой (сине-зелёный) 480-510
Зелёный 510-550
Жёлто-зелёный 550-575
Жёлтый 575-585
Оранжевый 585-620
Красный 620-770
/------------------------------------------------------------------------------/
*/
#ifndef PLANK_H
#define PLANK_H

#include "global_types.h"
#include <vector>

/**
 * @brief  Функция формирует разбиение спектра (пишет по убыванию)
 * @warning расчитана на фиксированный размер 1000 ячеек. в диапазоне  [10^11-10^20]
 * @note При расчёте интегрального излучения необходимо дополнительно брать интеграл [0...10^11]
 *
 * @param[out] spectrum разбиение спектра без нуля
 */
void get_splitting_spectrum(std::vector<Type> &spectrum);

/**
 * @brief Функция возвращает номер интервала в разбиении (обратная к splitting функция)
 *
 * @param frq частота фотона
 * @return индекс в массиве
 */
int get_frq_idx(double frq);

/**
 * @brief Функция вычисляет равновесное излучение по закону Планка в частотном диапазоне
 *
 * @param T температура в Кельвинах
 * @param nu верхний предел частоты в Гц
 * @param nu0 нижний предел частоты в Гц
 * @return Значение излучения в частотном диапазоне (sigma*T^4)
 * @warning Возможно потеряно деление на Pi
 */
double B_Plank(double T, double nu, double nu0);

/**
 * @brief Функция вычисляет логарифм от равновесного излучения по закону Планка в частотном диапазоне
 *
 * @param T температура в Кельвинах
 * @param nu верхний предел частоты в Гц
 * @param nu0 нижний предел частоты в Гц
 * @return Значение излучения в частотном диапазоне log(sigma*T^4)
 * @warning Возможно потеряно деление на Pi
 */

double B_Plank_log(double T, double nu, double nu0);

#endif //! PLANK_H