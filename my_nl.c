#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/* https://www.ibm.com/developerworks/ru/library/au-unix-getopt/index.html */
/* Файлы, чьи имена были дополнительно указаны в команде, 
 * будут использоваться в качестве входных файлов. 
 */


/* Глобальная структура с аргументами 
 */
struct global_args_t {
	char		*key_string;	/* -k ключ */
	int		decryption;	/* -d ключ */
	const char	*in_filename;	/* -o ключ */
	FILE		*in_file;
	const char	*out_filename;	/* -f ключ */
	FILE		*out_file;
	char		*msg;		/* сообщение шифровки/дешифровки */
} global_args;
