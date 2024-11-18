# Терминал чтения и отправки сообщений в com-port
**недостатки**
*сделано на QT + serial, используются контейнеры QT типа: QVector<>, QByteArray*
**Возможности:**
1. передача/приём данных в hex формате (при этом строка ввода анализуется и отбрасываются все символы кроме: 0-9, A-F, a-f)
2. возожность построения графика
3. запись данных в файл (сейчас не сделано)
4. отображение принятых знаков

## как пользоваться
**важно!!! ** 
*формат команды : * [sync][len][cmd][data][xor]
1. sync -- байт синхронизации (0xCA)
2. len -- длина сообщения, начиная с байта длины
3. cmd -- команда, два старших бита команды используются для идентификации: записи в файл, построение точки
4. data -- это поле данных 
5. xor -- закомментировано

*команды*
1. 0x88 -- команда установки точек для построения графики и записи в файл
	0xCA	0xNN	0x88	 {offset_x[1]}	{size_x[1]}	{offset_y[1]}	{size_y[1]}	xor
	после отправки этой команды, и получения любой другой команды, у которой старший бит установлен в 1
	будет выполняться построение графика
2. 0x01	-- команда передачи значений температуры и установленного ШИМ сигнала
	0xCA	0x0D	0x01	{time[4]}	{temperature[4]}	{pwm[2]}	xor

*пример использования*
Для построения графика температур, необходимо отправить команды:
	CA	07	88	00	04	04	04	{xor]
	при получении команды :
					time			temperature		pwm
	CA    0D	81	01 00 00 00		65 0e 00 00		45 00	xor
	будет построена одна точка графки где x = 00000001, а y = 00000e65

