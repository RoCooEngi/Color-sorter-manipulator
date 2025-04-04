# Automatic cube sorting by color

## Installation

The project was created using the PlatformIO extension for Visual Studio code. To work with the programs, you need to unzip the repository or clone it using the command `git clone https://github.com/RoCooEngi/Color-sorter-manipulator.git your_folder` and open the folders separately from each other using the PlatformIO extension. If you need to work in the Arduino editor, extract the source files of the programs along the path `src/main.cpp`, rename them with the `.ino` extension and check for the presence of the corresponding libraries in your Arduino environment.

## System description

The project is an automated color cube sorting system using ESP32-CAM for color analysis and Arduino UNO to control the manipulator. The two-link manipulator is equipped with four servos that provide gripping, lifting, moving and sorting of objects. The entire system is controlled via a serial connection between ESP32-CAM and Arduino UNO.

## Hardware

The basis of the system is ESP32-CAM, which performs image processing and cube color detection. Arduino UNO is responsible for controlling the servos, performing sorting based on the transmitted data. The design uses four servos: one for forward and backward movement, the second for lifting and lowering, the third for rotating the manipulator and the fourth for gripping cubes. An LED is connected to indicate the system status. All components are powered by an external power source, and the communication between the ESP32-CAM and Arduino is carried out via UART. The TX0 pins of the Arduino UNO and RX0 of the ESP32-CAM are connected via a voltage divider, since the latter's pins are not tolerant of 5V.
The full circuit diagram looks like this:
![electric_circuit](https://github.com/user-attachments/assets/cb6cf936-5b37-4d64-9713-ac0db1b1ef87)

## Software

### Image processing on the ESP32-CAM

The camera operates in RGB565 format with a resolution of 240x240. The image is divided into a 3x3 grid, after which the average color is calculated for each segment. Next, noise is filtered based on brightness and saturation, and then the dominant color is determined. If a color is recognized, the ESP32-CAM transmits the corresponding value ("RED", "GREEN" or "PURPLE") to the Arduino via UART.

The following formulas are used to convert RGB565 to 8-bit RGB:

$$R = (R_{565} << 3) | (R_{565} >> 2)$$

$$G = (G_{565} << 2) | (G_{565} >> 4)$$

$$B = (B_{565} << 3) | (B_{565} >> 2)$$

Brightness and saturation are determined by the formulas:

$$Brightness = \frac{R + G + B}{3}$$

$$Saturation = \frac{Max(R, G, B) - Min(R, G, B)}{Max(R, G, B)}$$

### Manipulator control on Arduino UNO

Arduino receives data from ESP32-CAM, determines the required direction of the manipulator movement and performs sorting. When a color recognition command is received, the manipulator grabs the cube, picks it up, moves it to the desired position, releases it, and then returns to its original state. After the process is completed, the "go" command is sent to ESP32-CAM, signaling readiness for the next cycle.

Simple smooth servo movement functions are used to control the movements. For example, capture is performed by the command `servo_catch.write(170)`, lifting - `slow_servo(servo_raise, 110)`, moving forward - `slow_servo(servo_push, 70)`, and rotation to the desired angle is performed via `slow_servo(servo_rotate, rotation)`. After sorting, the manipulator returns to its original position.


# Автоматическая сортировка кубиков по цвету

## Установка

Проект был создан при помощи расширения PlatformIO для Visual Studio code. Для работы с программами необходимо разархивировать репозиторий или клонировать его используя комманду `git clone https://github.com/RoCooEngi/Color-sorter-manipulator.git ваша_папка` и открывать папки отдельно друг от друга используя расширение PlatformIO. Если необходимо работать в редакторе Arduino, то извлеките исходные файлы программ по пути `src/main.cpp`, переименуйте их с расширением `.ino` и проверьте наличие соответствующих библиотек в своей среде Arduino.

## Описание системы

Проект представляет собой автоматизированную систему сортировки цветных кубиков с использованием ESP32-CAM для цветового анализа и Arduino UNO для управления манипулятором. Двузвенный манипулятор оснащен четырьмя сервоприводами, которые обеспечивают захват, подъем, перемещение и сортировку объектов. Вся система управляется через последовательное соединение между ESP32-CAM и Arduino UNO.

## Аппаратная часть

Основу системы составляет ESP32-CAM, выполняющий обработку изображения и определение цвета кубика. Arduino UNO отвечает за управление сервоприводами, выполняя сортировку на основании переданных данных. В конструкции используются четыре сервопривода: один для перемещения вперед-назад, второй для подъема и опускания, третий для поворота манипулятора и четвертый для захвата кубиков. Светодиод подключен для индикации состояния системы. Все компоненты питаются от внешнего источника питания, а связь между ESP32-CAM и Arduino осуществляется через UART. Пины TX0 от Arduino UNO и RX0 от ESP32-CAM соединены через делитель напряжения, т.к. пины последнего не толерантны к напряжению 5V.
Полная электросхема имеет следующий вид:
![electric_circuit](https://github.com/user-attachments/assets/cb6cf936-5b37-4d64-9713-ac0db1b1ef87)


## Программная часть

### Обработка изображения на ESP32-CAM

Камера работает в формате RGB565 с разрешением 240x240. Изображение делится на сетку 3×3, после чего для каждого сегмента вычисляется средний цвет. Далее происходит фильтрация шумов на основе яркости и насыщенности, а затем определяется доминирующий цвет. Если цвет распознан, ESP32-CAM передает соответствующее значение ("RED", "GREEN" или "PURPLE") на Arduino через UART.

Для конвертации RGB565 в 8-битный RGB используются следующие формулы:

$$R = (R_{565} << 3) | (R_{565} >> 2)$$

$$G = (G_{565} << 2) | (G_{565} >> 4)$$

$$B = (B_{565} << 3) | (B_{565} >> 2)$$

Яркость и насыщенность определяются по формулам:

$$Brightness = \frac{R + G + B}{3}$$

$$Saturation = \frac{Max(R, G, B) - Min(R, G, B)}{Max(R, G, B)}$$

### Управление манипулятором на Arduino UNO

Arduino принимает данные от ESP32-CAM, определяет необходимое направление движения манипулятора и выполняет сортировку. При поступлении команды цветового распознавания манипулятор захватывает кубик, поднимает его, перемещает в нужное положение, отпускает, а затем возвращается в исходное состояние. После завершения процесса отправляется команда "go" на ESP32-CAM, сигнализирующая о готовности к следующему циклу.

Для управления движениями используются простые функции плавного перемещения сервоприводов. Например, захват осуществляется командой `servo_catch.write(170)`, подъем — `slow_servo(servo_raise, 110)`, перемещение вперед — `slow_servo(servo_push, 70)`, а поворот на нужный угол выполняется через `slow_servo(servo_rotate, rotation)`. После сортировки манипулятор возвращается в исходное положение.
