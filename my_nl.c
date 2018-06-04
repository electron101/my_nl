/* Это программа является учебной копией известной UNIX утилиты - nl. 
 *
 * Были скопированы следующие опции оригинальной nl: -i, -v, -w, -s
 * Были добавлены собственные опции: -a
 * 
 * Сборка: gcc my_nl.c -o my_nl
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>


/* возвращает количество цифр в числе, 
 * число может быть и отрицательным 
 */
size_t cini(int n)
{
	size_t count = 0u;
	if (n < 0)
		count++;
	n = abs(n);
	if (n / 10 == 0)
		return ++count;
	else 
	{
		while (n > 0) 
		{
			n = n / 10;
			count ++;
		}
		return count;
	}     
}


/* проверяет является ли строка числом,
 * число может быть и отрицательными.
 * вернёт 1 если строка это число,
 * вернёт 0 елси строка не число
 */
int is_int(char *str)
{
	int n = 0;
	int i;

	for (i = 0; str[i] != 0; i++)
	{
		if (str[i] >= '0' && str[i] <= '9') 
			n++; //n -увеличивается, если символ 0,1,2...9
	}

	str[i] = '\0';

	if ( (str[0] == '-' && n == i - 1) || n == i ) 
		return 1;

	return 0;
}



/* Сообщение с информацией об опциях 
*/
void usage(char *progname, int status) 
{
	/* Если просто ошибка печатаем команду для вывода помощи,
	 * если введена команда вывода помощи, то печатаем все опции
	 */
	if (status == 0)	/* печатаем сообщение о выводи помощи */
		fprintf (stderr, ("По команде '%s --help'\
можно получить дополнительную информацию.\n"), progname);

	if (status == 1)
	{
		printf("Использование: %s [КЛЮЧ]… [ФАЙЛ]…\n", progname);

		fputs (("\
Печатает каждый ФАЙЛ на стандартный вывод, добавляя номера строк.\n\
		"), stdout);

		fputs (("\n\
Аргументы, обязательные для длинных ключей, обязательны и для коротких.\n\
  -i, --line-increment=ЧИСЛО       шаг увеличения номеров строк\n\
  -s, --number-separator=СТРОКА    добавлять СТРОКУ после номера\n\
  -v, --starting-line-number=ЧИСЛО первый номер строки\n\
  -w, --number-width=ЧИСЛО         использовать заданное ЧИСЛО столбцов для\n\
                                   номеров строк\n\
  -a, --all-lines                  нумеровать пустые строки\n\n\
  --help     показать эту справку и выйти\n\
		"), stdout);

		fputs (("\n\
По умолчанию используются -v1 -i1 -sTAB -w6\n\n"), 
				stdout);


	}
	exit( EXIT_FAILURE );
}


/* Глобальная структура с аргументами 
*/
struct global_args_t {
	int  line_increment;	/* -i ключ */
	char *separator;	/* -s ключ */
	int  start_number;	/* -v ключ */
	int  number_width;	/* -w ключ */
	int  all_lines;		/* -a ключ */
	char **inputFiles;	/* входные файлы */
	int  numInputFiles;	/* число входных файлов */
} global_args;

static const char *opt_string = "i:s:v:w:a?";	/* строка с опциями getopt() */

static const struct option longOpts[] = {
	{ "line-increment", required_argument, NULL, 'i' },
	{ "number-separator", required_argument, NULL, 's' },
	{ "starting-line-number", required_argument, NULL, 'v' },
	{ "number-width", required_argument, NULL, 'w' },
	{ "all-lines", no_argument, NULL, 'a' },
	{ "help", no_argument, NULL, 0 },
	{ NULL, no_argument, NULL, 0 }
};

void validate_args(int argc, char *argv[])
{
	/* Инизиализация структуры.
	 * По умолчанию используются -v1 -i1 -sTAB -w6 
	 */
	global_args.line_increment = 1;		/* -i ключ */
	global_args.separator      = "\t";	/* -s ключ */
	global_args.start_number   = 1;		/* -v ключ */
	global_args.number_width   = 6;		/* -w ключ */
	global_args.all_lines      = 0;		/* -a ключ */
	global_args.inputFiles     = NULL;	/* входные файлы */
	global_args.numInputFiles  = 0;		/* число входных файлов */


	int opt = 0;
	opterr	= 0;		/* отключим вывод ошибок в getopt()*/
	int longIndex = 0;

	/* Обработки параметров в программе. 
	 * Как только getopt() распознает параметр, конструкция switch 
	 * определит, какой точно параметр был найден, и отметит 
	 * соответствующее поле в структуре globalArgs. Когда getopt() 
	 * вернет -1, то можно считать, что обработка параметров 
	 * выполнена и оставшиеся аргументы являются файлами ввода.
	 */
	while( 
			(opt = getopt_long( argc, argv, opt_string, longOpts, &longIndex )) 
			!= -1 ) 
	{
		switch( opt ) {
			case 'i':
				/* приращение номера строки должнo быть 
				 * целым числом и быть больше нуля
				 */
				if ( is_int(optarg) == 0 || atoi(optarg) <= 0 )
				{
					fprintf (stderr, 
							("%s неверное приращение номера строки: «%s»\n"), 
							argv[0], optarg);
					usage(argv[0], 0);
				}
				global_args.line_increment = atoi(optarg);
				break;

			case 's':
				global_args.separator = optarg;	
				break;

			case 'v':
				/* номер начальной строки должнo быть 
				 * целым числом 
				 */
				if ( is_int(optarg) == 0 )
				{
					fprintf (stderr, 
							("%s неверный номер начальной строки: «%s»\n"), 
							argv[0], optarg);
					usage(argv[0], 0);
				}
				global_args.start_number = atoi(optarg);
				break;

			case 'w':
				/* шинира поля номера строки должна быть 
				 * целым числом и быть больше нуля
				 */
				if ( is_int(optarg) == 0 || atoi(optarg) <= 0 )
				{
					fprintf (stderr, 
							("%s неверная ширина поля для номера строки: «%s»\n"), 
							argv[0], optarg);
					usage(argv[0], 0);
				}

				global_args.number_width = atoi(optarg);
				break;

			case 'a':
				global_args.all_lines = 1;
				break;

			case 0:
				if( strcmp("help", 
							longOpts[longIndex].name) == 0 ) 
					usage(argv[0], 1);
				break;

			case '?':
				usage(argv[0], 0);
				break;

			default:
				/* если не один из кейсов не сработал */
				break;
		}
	}
	global_args.inputFiles    = argv + optind;
	global_args.numInputFiles = argc - optind;
}



int main( int argc, char *argv[] )
{
	if (argc < 2)			/* если ключи не заданы */
		usage(argv[0], 0);

	validate_args(argc, argv);	/* проверка входных параметров */

	/* printf ("i = %d\n", global_args.line_increment); */
	/* printf ("s = %s\n", global_args.separator); */
	/* printf ("v = %d\n", global_args.start_number); */
	/* printf ("w = %d\n", global_args.number_width); */
	/* printf ("a = %d\n", global_args.all_lines); */
	/*  */
	/* printf ("files = %s\n", global_args.inputFiles[0]); */
	/* printf ("files = %s\n", global_args.inputFiles[1]); */
	/* printf ("files = %s\n", global_args.inputFiles[2]); */
	/* printf ("num_files = %d\n", global_args.numInputFiles); */


	FILE    *fp   = NULL;
	size_t  count_lines_in_files = 0;
	int     k;
	char    *line = NULL;
	size_t  len   = 0;
	ssize_t read;
	char    *str_f;
	int     j;


	/* цикл по всем файлам которые нужно вывести */
	for (k = 0; k < global_args.numInputFiles; k++) 
	{
		/* открытие файла на чтрение */
		if ( (fp = fopen( global_args.inputFiles[k], "r" )) == NULL )
		{
			/* perror("fopen"); */
			fprintf (stderr, 
					("Ошибка открытия файла - %s\n"), strerror(errno));
			return EXIT_FAILURE;
		}

		
		/* читаем строку из файла */
		while ( (read = getline(&line, &len, fp)) != -1 ) 
		{
			/* заполним строку пробелами, в количестве
			 * равному ключу w
			 */
			for (j = 0; j < global_args.number_width; j++)
				str_f[j] = (char)32;
			str_f[global_args.number_width] = '\0';


			/* нужно ли нумеровать пустую строку */
			if (read <= 1)
			{
				if (global_args.all_lines == 0) /* если не нужно */
				{
					/* выводим пустую строку */
					fprintf (stdout, ("%s\t\n"), str_f );
					continue;
				}
			}

			int    n = global_args.start_number;	/* номер строки */
			size_t count_cini = cini(n);	/* кол-во цифр в числе */
			char   str_i[count_cini + 1];	

			sprintf(str_i, "%d", n);	/* номер строки в char */

			/* если размер созданной пробельной строки больше 
			 * или равен количеству цифр номера
			 * то запишем номер в конец строки, начиная с той
			 * позиции. Позиция считается по формуле = 
			 * (длинна строки str_f) - (кол-во цифр номера) 
			 *
			 * ПРИМЕР: 
			 * 1. длинна строки str_f - 8
			 *  ___  ___  ___  ___  ___  ___  ___  ___   ___
			 * |   ||   ||   ||   ||   ||   ||   ||   | |▓▓▓
			 * |___||___||___||___||___||___||___||___| |▓▓▓
			 *   0    1    2    3    4    5    6    7    \0
			 *
			 * 2. мы хотим записать цисло -467, оно состоит из
			 * четырёх символов (знак минус, 4, 6, 7)
			 * 3. position = 8 - 4 = 4. 
			 * 4. записывать будем начиная с 4 индекса куда указывает
			 * position. записть будет идти по элементно в цикле
			 * в итоге получим строку с текущим номером в конце
			 *  ___  ___  ___  ___  ___  ___  ___  ___   ___
			 * |   ||   ||   ||   || - || 4 || 6 || 7 | |▓▓▓
			 * |___||___||___||___||___||___||___||___| |▓▓▓
			 *   0    1    2    3    4    5    6    7    \0
			 *                       ^
			 *                    position
			 */
			if (global_args.number_width >= count_cini)
			{
				size_t position = strlen(str_f) - count_cini;

				for (j = 0; j < count_cini + 1; j++)
				{
					str_f[position] = str_i[j];
					position++;
				}
				str_f[position] = '\0';
			}

			/* если количество цифр больше чем пробельная строка,
			 * то запишем число в начало строки
			 */
			if (global_args.number_width < count_cini)
			{
				strcpy( str_f, str_i );
				str_f[count_cini] = '\0';
			}

			/* вывод строки с номером и сепаратором, и строки файла */
			fprintf (stdout, ("%s%s%s"), str_f, global_args.separator, line);

			/* увеличим номер строки на шаг нумерации (ключ i) */
			global_args.start_number += global_args.line_increment;
		}
}

/* Освободим память */
fclose(fp);
if (line)
	free(line);

return EXIT_SUCCESS;
}
