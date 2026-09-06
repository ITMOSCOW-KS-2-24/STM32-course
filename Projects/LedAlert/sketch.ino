#include <stm32l0xx_hal.h>

// Назначение выводов

#define GREEN_LED_PORT GPIOB
#define GREEN_LED_PIN  GPIO_PIN_0      // D3

#define RED_LED_PORT   GPIOB
#define RED_LED_PIN    GPIO_PIN_1      // D6

#define BUTTON_PORT    GPIOA
#define BUTTON_PIN     GPIO_PIN_0      // A0

// НАСТРОЙКА GPIO

void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    // Включаем GPIOA и GPIOB
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    
    // СВЕТОДИОДЫ

    // PB0 и PB1
    GPIO_InitStruct.Pin = GREEN_LED_PIN | RED_LED_PIN;

    // Режим выхода
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

    // Подтяжка не требуется
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    // Низкой скорости более чем достаточно
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // Применяем настройки к GPIOB
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct);

    // КНОПКА
    // PA0
    GPIO_InitStruct.Pin = BUTTON_PIN;
    // Режим входа
    GPIO_InitStruct.Mode =  GPIO_MODE_INPUT;
    // Внутренняя подтяжка к питанию
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    // Применяем настройки к GPIOA
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
}

 void check_button()
    {
        if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
          {
            // Кнопка нажата

            // Выключаем зелёный
            HAL_GPIO_WritePin( GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET );


            // Красный мигает пять раз
            for (int i = 0; i < 5; i++)
            {
                // Красный ON
                HAL_GPIO_WritePin( RED_LED_PORT, RED_LED_PIN, GPIO_PIN_SET );
                HAL_Delay(500);


                // Красный OFF
                HAL_GPIO_WritePin( RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET );
                HAL_Delay(500);
            }


            // Возвращаем нормальный режим

            // Красный выключаем
            HAL_GPIO_WritePin( RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET );
            // Зелёный включаем
            HAL_GPIO_WritePin( GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET );


            // Ждём отпускания кнопки. Иначе удержание кнопки сразу запустит аварийку ещё раз.

            while ( HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET )
            {
                // Просто ждём
            }
            // Защита от дребезга контактов
            HAL_Delay(50);
        }
    }

int main(void)
{
    // Запуск HAL
    HAL_Init();

    // Настройка GPIO
    GPIO_Init();


    // Начальное состояние

    // Зелёный горит
    // Красный выключен

    HAL_GPIO_WritePin( GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET );
    HAL_GPIO_WritePin( RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET );


    // Основной бесконечный цикл


    while (1)
    {
      check_button(); //Вызов функции обработки кнопки
    }
}
