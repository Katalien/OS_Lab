# Параллельные структуры. Вариант 10. Зинякова Екатерина
## Структура 2
Множество с “тонкой” синхронизацией (Fine-Grained Synchronization)[1, p.201]. Множество реализуется с помощью связного списка. "Тонкая" синхронизация подразумевает блокирование каждого узла вместо блокирования всего множества в "грубой".

# Запуск программы:
- chmod +x ./build.sh
- sudo ./build.sh
- ./Lab3 threads_num elements_num

  threads_num -  кол-во потоков,
  elements_num -  кол-во элементов