# GPIO STM32: кнопка и светодиоды

Для этой платы:

| Устройство | Вывод STM32 | В HAL |
| --- | --- | --- |
| Зелёный светодиод `D3` | `PB0` | `GPIOB`, `GPIO_PIN_0` |
| Красный светодиод `D6` | `PB1` | `GPIOB`, `GPIO_PIN_1` |
| Кнопка `A0` | `PA0` | `GPIOA`, `GPIO_PIN_0` |

Пин можно представить как адрес: **порт — это дом, номер пина — квартира**.

- `PA0` — дом `A`, квартира `0` → `GPIOA`, `GPIO_PIN_0`.
- `PC14` — дом `C`, квартира `14` → `GPIOC`, `GPIO_PIN_14`.

То есть для пина `PXn` в HAL всегда пишут `GPIOX` и `GPIO_PIN_n`.

Код в файле обычно расположен так: подключение HAL, имена пинов, настройка GPIO, функции с действиями, `main()`.

## Имена пинов

Пишутся в начале файла, после `#include`. Имена слева можно выбирать свои; значения справа берут из распиновки.

``` c
#include <stm32l0xx_hal.h>

// Зелёный светодиод: D3 -> PB0
#define GREEN_LED_PORT GPIOB
#define GREEN_LED_PIN  GPIO_PIN_0

// Красный светодиод: D6 -> PB1
#define RED_LED_PORT   GPIOB
#define RED_LED_PIN    GPIO_PIN_1

// Кнопка: A0 -> PA0
#define BUTTON_PORT    GPIOA
#define BUTTON_PIN     GPIO_PIN_0
```

Так вместо `GPIOB, GPIO_PIN_0` можно писать `GREEN_LED_PORT, GREEN_LED_PIN`. Это не переменные: перед компиляцией такие имена подставляются в код.

## Настройка GPIO

Внутри `GPIO_Init()` сначала включают все используемые порты. Без этого пины не работают.

``` c
// Для кнопки PA0
__HAL_RCC_GPIOA_CLK_ENABLE();

// Для светодиодов PB0 и PB1
__HAL_RCC_GPIOB_CLK_ENABLE();
```

Затем создают структуру настроек. Эта строка нужна один раз в функции.

``` c
// Переменная, в которую записываются настройки пина
GPIO_InitTypeDef GPIO_InitStruct = {0};
```

### Выход: светодиод, реле, зуммер

Этот кусок настраивает зелёный светодиод. Для другого устройства меняют только его `PORT` и `PIN`.

``` c
// Выбираем пин устройства
GPIO_InitStruct.Pin = GREEN_LED_PIN;

// Настраиваем его как цифровой выход
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

// Для светодиода подтяжка не нужна
GPIO_InitStruct.Pull = GPIO_NOPULL;

// Светодиоду достаточно низкой скорости
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

// Применяем настройки к порту светодиода
HAL_GPIO_Init(GREEN_LED_PORT, &GPIO_InitStruct);
```

`GPIO_MODE_OUTPUT_PP` — STM32 управляет уровнем на выводе. `GPIO_NOPULL` — подтяжка не нужна. `GPIO_SPEED_FREQ_LOW` подходит для светодиода.

Два пина одного порта с одинаковыми настройками можно объединить:

``` c
// Оба светодиода находятся в порту GPIOB
GPIO_InitStruct.Pin = GREEN_LED_PIN | RED_LED_PIN;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
```

### Вход: кнопка

Кнопка должна быть соединена между `PA0` и `GND`.

``` c
// Выбираем пин кнопки
GPIO_InitStruct.Pin = BUTTON_PIN;

// Кнопка передаёт состояние во вход STM32
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

// Отпущенная кнопка даёт логическую 1
GPIO_InitStruct.Pull = GPIO_PULLUP;

// Применяем настройки к порту кнопки
HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
```

`GPIO_PULLUP` держит вход в единице. Поэтому отпущенная кнопка — `GPIO_PIN_SET`, нажатая — `GPIO_PIN_RESET`.

## Команды светодиоду

``` c
// Включить зелёный светодиод
HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);

// Выключить зелёный светодиод
HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
```

Первая строка включает светодиод, вторая выключает. У `HAL_GPIO_WritePin` всегда три части: порт, пин, состояние.

## Проверка кнопки

Этот кусок пишут внутри своей функции, например `check_button()`:

``` c
// При PULLUP RESET означает: кнопка нажата
if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
{
    // Действие при нажатии
    HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);
}
```

`HAL_GPIO_ReadPin` читает уровень на входе. Из-за `PULLUP` значение `RESET` означает, что кнопка нажата.

## Повторить действие

Например, мигнуть красным пять раз:

``` c
// Повторить пять раз
for (int i = 0; i < 5; i++)
{
    // Включить красный светодиод на 500 мс
    HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_SET);
    HAL_Delay(500);

    // Выключить красный светодиод на 500 мс
    HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
    HAL_Delay(500);
}
```

`i` считает повторы. `HAL_Delay(500)` ждёт 500 миллисекунд.

После обработки нажатия ждут отпускания кнопки. Иначе при удержании действие начнётся снова.

``` c
// Пока кнопка удерживается, ничего нового не делаем
while (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
{
}

// Ждём, пока исчезнет дребезг контактов
HAL_Delay(50);
```

## Главный цикл

В `main()` сначала запускают HAL и вызывают свою настройку GPIO:

``` c
// Запуск библиотеки HAL
HAL_Init();

// Настройка портов и пинов
GPIO_Init();
```

Затем микроконтроллер постоянно проверяет кнопку и выполняет действие при нажатии:

``` c
while (1)
{
    if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
    {
        // Действие при нажатии: включаем зелёный светодиод
        HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);

        // Ждём отпускания кнопки
        while (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
        {
        }

        // Защита от дребезга контактов
        HAL_Delay(50);
    }
}
```

`while (1)` — бесконечный цикл. В него помещают постоянные действия: проверку кнопок, чтение датчиков и управление устройствами. Вместо строки включения зелёного светодиода можно вставить другой нужный фрагмент: мигание, выключение устройства или включение красного светодиода.
