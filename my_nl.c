#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>



/* https://www.ibm.com/developerworks/ru/library/au-unix-getopt/index.html */

  /* -i, --line-increment=ЧИСЛО       шаг увеличения номеров строк */
  /* -s, --number-separator=СТРОКА    добавлять СТРОКУ после номера */
  /* -v, --starting-line-number=ЧИСЛО первый номер строки для каждой логической */
  /*                                  страницы */
  /* -w, --number-width=ЧИСЛО         использовать заданное ЧИСЛО столбцов для */
  /*                                  номеров строк */

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
	if (status == 0)
	{
		//печатаем сообщение о выводи помощи
		printf("\n--help:\n\n");
		exit( EXIT_FAILURE );
	}

	if (status == 1)
	{
		//печатаем опции
		printf("\nИспользование:\n\n");
		printf("%s [ключ] \n\n", progname);
		printf("Ключи:\n");
		printf(" -k \t ключ шифровки/дешифровки\n");
		printf(" -d \t ключ включения режима дешифровки\n");
		printf(" -f \t файл для чтения сообщения\n");
		printf(" -o \t файл для записи сообщения\n");
		printf(" -e \t примеры использования программы\n");
		printf(" -h \t вывод этого сообщения\n\n");
		exit( EXIT_FAILURE );
	}
}


/* Глобальная структура с аргументами 
 */
struct global_args_t {
	int	line_increment;		/* -i ключ */
	char	*separator;		/* -s ключ */
	int	start_number;		/* -v ключ */
	int	number_width;		/* -w ключ */
	char	**inputFiles;		/* входные файлы */
	int	numInputFiles;		/* число входных файлов */
} global_args;

static const char *opt_string = "i:s:v:w:?";	/* строка с опциями getopt() */

static const struct option longOpts[] = {
	{ "line-increment", required_argument, NULL, 'i' },
	{ "number-separator", required_argument, NULL, 's' },
	{ "starting-line-number", required_argument, NULL, 'v' },
	{ "number-width", required_argument, NULL, 'w' },
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
	global_args.number_width   = 6;		/* -f ключ */
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
	/* opt = getopt_long( argc, argv, optString, longOpts, &longIndex ) */
	/* while( (opt = getopt( argc, argv, opt_string )) != -1 ) { */
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
				
			case '0':
				if( strcmp( "help", longOpts[longIndex].name ) == 0 ) 
					usage(argv[0], 1);
				else
					usage(argv[0], 0);
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
	
	printf ("i = %d\n", global_args.line_increment);
	printf ("s = %s\n", global_args.separator);
	printf ("v = %d\n", global_args.start_number);
	printf ("w = %d\n", global_args.number_width);
	
	printf ("files = %s\n", global_args.inputFiles[0]);
	printf ("files = %s\n", global_args.inputFiles[1]);
	printf ("files = %s\n", global_args.inputFiles[2]);
	printf ("num_files = %d\n", global_args.numInputFiles);

	return EXIT_SUCCESS;
}
