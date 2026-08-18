// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file socd_cleaner.c
 * @brief SOCD Cleaner implementation - ОПТИМИЗИРОВАННАЯ ВЕРСИЯ
 *
 * Оптимизации для производительности:
 * 1. Минимизация ветвлений в горячем пути
 * 2. Битовые операции вместо арифметических
 * 3. Ранний выход при неактивном состоянии
 * 4. Inline-кандидаты для критичных функций
 */

#include "socd_cleaner.h"

#ifdef __cplusplus
extern "C" {
#endif

bool socd_cleaner_enabled = true;

// Оптимизированная функция обновления клавиши
// Используем inline подсказку для компилятора
static inline void update_key_fast(uint8_t keycode, bool press) {
    if (press) {
        add_key(keycode);
    } else {
        del_key(keycode);
    }
}

// Оптимизированная обработка SOCD с минимальными ветвлениями
bool process_socd_cleaner(uint16_t keycode, keyrecord_t* record,
                          socd_cleaner_t* state) {
    // Критичная оптимизация: быстрый выход если SOCD отключен глобально
    if (!socd_cleaner_enabled) {
        return true;
    }
    
    // Быстрый выход если разрешение выключено для этой пары
    if (state->resolution == SOCD_CLEANER_OFF) {
        return true;
    }
    
    // Проверка что keycode принадлежит нашей паре
    // Используем сравнение вместо массива для скорости
    const uint16_t key0 = state->keys[0];
    const uint16_t key1 = state->keys[1];
    
    if (keycode != key0 && keycode != key1) {
        return true;  // Быстрый выход для нерелевантных клавиш
    }
    
    // Оптимизация: вычисляем индекс через битовую операцию вместо условия
    // i = 0 если keycode == key0, i = 1 если keycode == key1
    const uint8_t i = (keycode == key1);
    const uint8_t opposing = i ^ 1;  // XOR для получения противоположного индекса
    
    // Отслеживаем физическое нажатие
    state->held[i] = record->event.pressed;
    
    // Основной путь: обрабатываем только если противоположная клавиша зажата
    if (!state->held[opposing]) {
        return true;  // Быстрый выход - нет конфликта SOCD
    }
    
    // Горячий путь: обработка конфликта SOCD
    // Разворачиваем switch для избежания накладных расходов
    const uint8_t resolution = state->resolution;
    
    if (resolution == SOCD_CLEANER_LAST) {
        // Last input priority - наиболее популярный режим
        update_key_fast(state->keys[opposing], !state->held[i]);
        // Продолжаем стандартную обработку
        return true;
    }
    
    if (resolution == SOCD_CLEANER_NEUTRAL) {
        // Neutral resolution - отменяем обе клавиши
        update_key_fast(state->keys[opposing], !state->held[i]);
        send_keyboard_report();
        return false;  // Пропускаем стандартную обработку
    }
    
    // Обработка режимов 0_WINS и 1_WINS
    if (resolution >= SOCD_CLEANER_0_WINS && resolution <= SOCD_CLEANER_1_WINS) {
        const uint8_t winner = resolution - SOCD_CLEANER_0_WINS;
        
        if (opposing == winner) {
            // Противоположная клавиша выигрывает - текущая игнорируется
            return false;
        } else {
            // Текущая клавиша выигрывает
            update_key_fast(state->keys[opposing], !state->held[i]);
        }
    }
    
    return true;
}

#ifdef __cplusplus
}
#endif
