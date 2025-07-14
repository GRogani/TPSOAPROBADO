/**
 * el dump tiene como finalidad, bajar a un archivo, el contenido de los frames de un proceso especifico
 * el dump lo que deberá hacer es:
 * 1. obtener los frames del proceso, utilizando la lista de frames asignados.
 * 2. por cada frame, obtener el contenido de la memoria física, haciendo un read al user_space con el tamaño de una pagina
 * 3. el contenido que devuelve cada lectura, debe escribirse en el archivo especifico con el siguiente formato: <PID>-<TIMESTAMP>.dmp
 */