## Задача: 
Спроектировать приложение, которое позволяет преподавателю проводить blind code review между студентами.

## Userflow: 

1) Студент отправляет готовую работу на code review в период времени с нулевого до первого дедлайна.
2) Преподаватель скачивает работы студентов в день первого дедлайна.
3) Происходит распределение анонимных работ между студентами.
4) В день первого дедлайна студентам отправляются N анонимных работ других студентов.
5) До второго дедлайна студенты должны вернуть code review отправленных им задач.
6) Преподаватель в день второго дедлайна скачивает проверенные работы и смотрит комментарии студентов, проверяют статистику.

Взаимодействие с работами студентов будет осуществляться через Git репозитории. Интерфейс для преподавателя отличается от интерфейса для студентов.

## Взаимодействие компонентов: 
1) **Userflow 1-4 : интерфейс преподавателя**. 
	В день дедлайна преподаватель в приложении нажимает на кнопку "Start review", выбирает путь до файла с ссылками, указывает путь до папки, в которую происходит клонирование всех репозиториев студентов, и происходит клонирование всех репозиториев студентов в локальную папку "repos" (предполагается, что все репозитории имеют разные названия), распределение файлов, анонимизация файлов, копирование файлов в другие репозитори и выгрузка этих файлов в репозитории студентов.
	- Компонент GUI вызывает функцию "CloneByFile" (которая в свою очередь вызывает функцию "Clone"), в которую передаётся путь до файла с ссылками на репозитории студентов.
	- Компонент GUI начинает распределение файлов между студентами для review: вызывается функция "Distribution", которая составляет список распределений "<1 студент> : <1 файл>". Список распределений хранится локально в папке программы (только у преподавателя).
	- Компонент GUI вызывает функцию "Anonymization": происходит обезличивание всех файлов - удаление из всех файлов первой строчки с комментарием ("// Copyright by Name Surname").
	- Компонент GUI вызывает функцию "Copy files": происходит копирование обезличенных файлов между студенческими репозиториями согласно списку распределений (создаётся папка "for-review", в которую копируются файлы из других репозиториев студентов).
	- Компонент GUI вызывает функцию "AddCommitPush" для папки "for-review": происходит выгрузка обезличенных файлов в папки "for-review" удалённых репозиториев студентов.

3) **Userflow 5 : интерфейс студента**. 
	Между первым и вторым дедлайном студент нажимает кнопку "Set path", выбирает путь до корневой папки локального репозитория, после чего у студента локально появляется папка "for-review", в которой появляются M обезличенных файлов других студентов.
	- Компонент GUI вызывает функцию "Pull" каждый раз, когда студент открывает приложение: происходит выгрузка добавленной папки "for-review" в локальный репозиторий студента.
	- Через компонент GUI студент оставляет комментарий к работам других студентов.
	- После закрытия компонента GUI сохраняются все комментарии, оставленные им в текстовый файл review.txt в папку "for-review" в виде "<номер стоки> <комментарий>"
	- По нажатию кнопки "Send" компонент GUI вызывает функцию "AddCommitPush" для папки "for-review": происходит выгрузка файла с комментариями (review.txt) в папку "for-review" удалённого репозитория студентов.

4) **Userflow 6 : интерфейс преподавателя**. 
	В день второго дедлайна преподаватель в приложении нажимает кнопку "End review", после чего происходит выгрузка всех комментариев студентов в локальные репозитории. Комментарии и статистика становятся доступны преподавателю и студенту для просмотра.
	- Компонент GUI вызывает функцию "Pull" по нажатию "End review": происходит выгрузка всех комментариев студентов в локальные репозитории.
	- Компонент GUI вызывает функцию "InverseDistribution": происходит перемещение файлов с комментариями в репозиторий студента, которому оставлены комментарии.
	- Компонент GUI вызывает функцию "Copy files": происходит копирование файлов с комментариями между студенческими репозиториями согласно списку распределений.
	- Компонент GUI подгружает файл с комментариями и отображает их в приложении.

------------------------------

Компоненты:
1) Взаимодействие с гитом
	- Автоматическая аутентификация в аккаунт
	- Скачивание
	- Выгрузка
	- Получение статуса репозитория	
2) Взаимодействие с fs
	- Проверка валидности данных
	- Поиск файлов
	- Обезличивание
		-  Внутреннее (удаление упоминаний автора в коде)
		- Внешнее (переименование файла в соответствии с паттерном) 
3) Взаимодействие с исходным кодом
	- Парсинг патчфайлов
	- Проверка на отсутствие изменений в коде
	- Обработка и хранение комментариев
4) Платформа аналитики
	- Визуализация данных 
		- Возможность смотреть общую/частную статистику 
	- WebUI
5) GUI для комментариев
	- read-only режим
	- write-only режим но только для оставления комментариев


Покрытие тестами
	1) Инфраструктурные тесты
			1) Подключение к гиту
			2) Подключение к бд
	2) Работа с файлами
		1) Корректное обезличивание (??)
		2) Запись и чтение комментариев
	3) Аналитика
		1) Одинаковые результаты для одинаковых данных
		2) UI тесты

1) Парсим map<string, string>  - имя владельца кода, имя ревьюера (Ключ и значение не уникальны)
2) Репозитории скачиваются
3) Обезличиваются
4) Загружаются
5) Ревьюеры пишут комментарии в GUI приложении*
6) Далее либо кнопка (повторить) либо (запустить)**
7) Находим файлы с комментариями
8) Парсим и сохраняем комментарии, матчим имя_владельца, имя_ревьюера, комментарий
9) В платформе для аналитики смотрим что можем посмотреть

		* -  GUI приложение всегда в read-only для кода и в write-only при пустых комментариях. Менять комментарии другим людям нельзя
		** - При повторном запуске заново парсим комментарии и проверяем что не нашли ничего нового