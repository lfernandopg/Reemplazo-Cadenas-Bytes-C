# Reemplazo-Cadenas-Bytes-C

Todo archivo guardado en disco tiene un número mágico que lo identifica, por ejemplo, las fotos .jpg.
empiezan con 0xFFD8 los pdf con 0x25504446 que traducido a ascii significa %PDF más información
en https://asecuritysite.com/forensics/magic . El objetivo es hacer un programa que pueda detectar el
tipo de archivo que se le pasa y dependiendo del caso cambiar este número mágico para hacerlo pasar
por otro o no hacer nada. También el programa debe ser capaz de:
Cambiar el tipo de archivo: aunque en Windows nada más con cambiar la extensión del archivo ya se
confunde el sistema operativo en Linux se revisa el número mágico para saber el tipo del archivo, y por
ejemplo para el enviar virus o programas a través del correo esto representa un serio problema, ya que,
aunque el archivo no tenga una extensión de ejecutable el número mágico de este lo delatará y en caso
de Gmail no se podrá enviar el código. Debe ser posible cambiar el archivo de tipo con estos
argumentos:

-cm nuevo
donde nuevo representa el número mágico nuevo
El programa también debe ser capaz de encontrar cualquier cadena de bytes en el archivo y ser capaz
de sustituirla por otra.
-e: seguido a esto viene el patrón a buscar, en caso de que no haya ninguna operación que
programa file1 -e patrón -r reemplazo
Este comando hace que todos los patrones que coincidan con patrón sean reemplazados por reemplazo
programa file1 -e patrón -n time -b 10
este comando el patrón número time, por ejemplo, si en un archivo hay 4 cadenas de bytes que
coinciden con el patrón y time es 3 entonces solo se seleccionara la tercera y se imprimirán 10 bytes de
esta cadena a partir del inicio de la misma, por ejemplo, en caso de que el patrón tenga más de 10 bytes
entonces no se imprimirá completo, pero si tiene menos se imprimirá el patrón y su continuación
programa file1 -e patrón -n time -r reemplazo
Este comando hace que se reemplace nada más la time coincidencia del patrón en el archivo
programa file1 -e patrón -n time -b 20 -r reemplazo

Este comando hace que se encuentre el -time coincidencia de patrón en el archivo y 20 bytes desde el
inicio de la coincidencia sean sustituidos por reemplazo
El programa debe también ser capaz de encontrar, un short, un int, un long y una cadena de strings
dentro del archivo.
Programa -i n
encuentra el int n
programa -w n
Encuentra el short n
programa -l n
Encuentra un long que coincide con n
programa -s “patrón”
Encuentra una string que coincide con “patrón”
Estas opciones deben poder combinarse con -n para la enésima coincidencia del patrón, -r para
reemplazar (en este caso solo se podrá reemplazar por el tipo que se estaba buscando), pero no se
pueden combinar con -e o -b
Ejemplo:
programa -i 16 -r 32
Encuentra todos los ints 16 y los reemplaza por 32
programa -s “reprobado” -n 10 -r “aprobado”
Encuentra la décima coincidencia del string “reprobado” y la reemplaza por “aprobado”
Además de eso el programa contará con el comando swap el cual se usará para intercambiar cadenas de
bytes, las cuales al intercambiarse de nuevo será como si no hubiera habido cambio alguno. Esto para
encriptar el archivo original
por ejemplo, el comando -s de swap requiere 2 cadenas la primera será la que se va a sustituir y la
segunda por la cual será sustituida, nótese que si esto se hace entonces swap b a no revertirá el proceso,
ya que habrá cadenas de tipo b que no se habrán modificado y no ve obtendrá el original, por ejemplo

0x1020304050607080904010301020
swap 0x10 0x20
0x2020304050607080904020302020
swap 0x20 0x10
0x1010304050607080904010301010
La segunda cadena es distinta a la original por lo que un swap de este estilo no sirve, el swap tiene que
ser bidireccional, la cadena a es sustituida por la b y la cadena b tiene que ser sustituida por la a para
que luego al afectar swap de nuevo se pueda obtener el archivo original.
