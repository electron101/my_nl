#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>



size_t count_lines(const char* filename) 
{
	FILE*  fp;
	size_t cnt = 0u;
	if((fp = fopen(filename, "r")) == NULL)
		return 0u;
	while( !feof(fp) && !ferror(fp) ) 
	{
		fscanf(fp, "%*[^\n]%*c");
		cnt++;
	}
	fclose(fp);
	return cnt;
}


size_t cini(int n)
{
	size_t count = 0u;
	if (n / 10 == 0)
		return 1u;
	else 
	{
		while (n > 0) 
		{
			n = n / 10;
			count += 1;
		}
		return count;
	}     
}

/* Файлы, чьи имена были дополнительно указаны в команде, 
 * будут использоваться в качестве входных файлов. 
 */


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
По умолчанию используются -v1 -i1 -sTAB -w6\n\n\
"), stdout);


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
	global_args.separator      = "TAB";	/* -s ключ */
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
	while( (opt = getopt_long( argc, argv, opt_string, longOpts, &longIndex )
) != -1 ) {
		switch( opt ) {
			case 'i':
				global_args.line_increment = atoi(optarg);
				break;
			
			case 's':
				global_args.separator = optarg;	
				break;
				
			case 'v':
				global_args.start_number = atoi(optarg);
				break;
			
			case 'w':
				global_args.number_width = atoi(optarg);
				break;
			
			case 'a':
				global_args.all_lines = 1;
				break;
				
			case 0:
				if( strcmp( "help", longOpts[longIndex].name ) == 0 ) 
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
	char    *line = NULL;
	size_t  len   = 0;
	ssize_t read;
	size_t  count_lines_in_files = 0;
	
	/* count_lines_in_files += count_lines( global_args.inputFiles[0] ); */
	/* count_lines_in_files += 3; */


	if ( (fp = fopen( global_args.inputFiles[0], "r" )) == NULL )
	{
		/* perror("fopen"); */
		fprintf (stderr, 
			("Ошибка открытия файла - %s\n"), strerror(errno));
		return EXIT_FAILURE;
	}



	size_t i = global_args.start_number;
	int    j;
	char   *str_f;

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
			if (global_args.all_lines == 0)
			{
				/* выводим пустую строку */
				/* fprintf (stdout, ("\n")); */
				fprintf (stdout, ("%s\t%s"), str_f, line );
				continue;
			}
		}

		int    n = i;
		size_t count_cini = cini(n);	/* кол-во цифр в числе */
		
		/* если размер созданной пробельной строки больше 
		 * или равен количеству цифр числа дла записи номера
		 */
		if (global_args.number_width >= count_cini)
		{
			for (j = global_args.number_width - 1; n > 0 ; --j)
			{
				str_f[j] = n % 10 + '0';
				n = n / 10;
				str_f[global_args.number_width] = '\0';
			}
		}

		/* если количество цифр больше чем пробельная строка,
		 * то расширим строку влево
		 */
		if (global_args.number_width < count_cini)
		{
			char str_i[count_cini + 1];
			sprintf(str_i, "%d", n);
			strcpy( str_f, str_i );
			str_f[count_cini] = '\0';
		}

		
		/* вывод строки с номером и сепаратором, и строки файла */
		fprintf (stdout, ("%s\t%s"), str_f, line );

		/* увеличим номер строки на константу (ключ v) */
		i += global_args.line_increment;
	}

	fclose(fp);
	if (line)
		free(line);
	
	return EXIT_SUCCESS;
}
