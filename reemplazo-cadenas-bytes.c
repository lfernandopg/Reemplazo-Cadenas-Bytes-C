#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE *file;
int positions_len;
typedef unsigned char byte; 
int i;

int openFile(char const *fileName){
    file = fopen(fileName,"rb+");
    if(file) return 1;
    return 0;
}

void printArg(int argc, char const *argv[]){
    printf("Numero de argumentos: %d\n", argc);
    for (i = 0; i < argc; i++){
        printf("Cadena %d: %s\n", i + 1,argv[i]);
    }
}

int compare(byte *str1, byte *str2, int str_len){
    for ( i = 0; i < str_len; i++){
        if(str1[i] != str2[i]){
            return 0;
        }
    }
    return 1;
}

int search(byte *str, int str_len){
    int found = 0;
    byte *actual = (byte*)calloc(str_len,sizeof(byte));
    byte *b = (byte*)calloc(1,sizeof(byte));
    while(!found && !feof(file)){
        fread(b, sizeof(byte), 1, file);
        if(*b == str[0]){
            fseek(file, -1, SEEK_CUR);
            fread(actual, sizeof(byte), str_len, file);
            if (compare(str,actual,str_len)) {
                found = 1;
                fseek(file, -str_len, SEEK_CUR);
            }else{
                if(!feof(file)){
                    fseek(file, -(str_len - 1), SEEK_CUR);
                }
            }
        }
    }
    return found;
}

void replace(byte *str, byte *rep, int str_len, int rep_len, int num) {
    if (num > 0){
        str_len = num;
    }
    if(str_len == rep_len){
        fwrite(rep, sizeof(byte), str_len, file);
    }else {
        int positionActual = ftell(file);
        byte *b = (byte*)calloc(1,sizeof(byte));
        int delta = rep_len - str_len;
        int k = positions_len - positionActual - str_len;
        if(str_len < rep_len){
            for (i = 0; i < k; i++){
                fseek(file, (positions_len - 1) - i, SEEK_SET);
                fread(b, sizeof(byte), 1, file);
                fseek(file, (delta + positions_len  - 1) - i, SEEK_SET);
                fwrite(b, sizeof(byte), 1, file);
            }
            fseek(file, positionActual, SEEK_SET);
            fwrite(rep, sizeof(byte), rep_len, file);
        }else{
            for (i = 0; i < k; i++){
                fseek(file, (positionActual + str_len) + i, SEEK_SET);
                fread(b, sizeof(byte), 1, file);
                fseek(file, (positionActual + rep_len) + i, SEEK_SET);
                fwrite(b, sizeof(byte), 1, file);
            }
            fseek(file, delta, SEEK_END);
            ftruncate(fileno(file),ftell(file));
            fseek(file, positionActual, SEEK_SET);
            fwrite(rep, sizeof(byte), rep_len, file);
        }
        positions_len += delta;
    }
}

int toInt(char c1){
    int n = ((int) c1);
    if(n > 47 && n < 58){
        n -= 48;
    }else if(n > 64 && n < 71){
        n -= 55;
    }else if (n > 96 && n < 103){
        n -= 87;
    }else if(n == 32){
        n = 0;
    }else{
        printf("Caracter Invalido\n");
    }
    return n;
}

int strToInt(char const* c1){
    int n = 0;
    int str_len = strlen(c1);
    n = toInt(c1[str_len - 1]);
    if(str_len > 1){
        int a = 10;
        str_len--;
        while(str_len > 0){
            n += toInt(c1[str_len - 1]) * a;
            a *= 10;
            str_len--;
        }
    }
    return n;
}

byte *convert(char const *str, int *len){
    char *c;
    if(str[0] == '0' && str[1] == 'x'){
        if(!(strlen(str) % 2)){
            c = (char*) calloc(strlen(str) - 1, sizeof(char));
            for (i = 0; i < strlen(str) - 1; ++i){
                if(i == strlen(str) - 2){
                    c[i] = 0;
                    break;
                }
                c[i] = str[i + 2];
            }
        }else{
            c = (char*) calloc(strlen(str), sizeof(char));
            c[0] = 32;
            for (i = 0; i < strlen(str); ++i){
                if(i == strlen(str) - 1){
                    c[i] = 0;
                    break;
                }
                c[i + 1] = str[i];
            }
        }
    }else{
        if(!(strlen(str) % 2)){
            c = (char*) calloc(strlen(str) + 1, sizeof(char));
            for (i = 0; i < strlen(str); ++i){
                c[i] = str[i];
            }
            c[strlen(str)] = 0;
        }else{
            c = (char*) calloc(strlen(str) + 2, sizeof(char));
            c[0] = 32;
            for (i = 0; i < strlen(str); ++i){
                c[i + 1] = str[i];
            }
            c[strlen(str) + 1] = 0;
        }
    }
    int str_len = strlen(c);
    int b_len = (str_len / 2) + 1;
    byte *b = (byte*)calloc(b_len, sizeof(byte));
    for ( i = 0; i < b_len; ++i){
        if(i == b_len - 1){
            b[i] = 0;
            break;
        }
        int n = (toInt(c[i*2]) * 16 + toInt(c[(i*2) + 1]));
        b[i] = n;
    }
    *len = b_len -1;
    return b;
}

void r(int time, byte *str, byte *rep, int num, int str_len, int rep_len){
    fseek(file, 0, SEEK_END);
    positions_len = ftell(file);
    fseek(file, 0, SEEK_SET);
    if(time < 0){
        while (!feof(file)){
            if(search(str, str_len)){
                replace(str, rep, str_len, rep_len, num);
            }
        }
    }else{
        int cont = 0;
        while (!feof(file) && cont < time){
            if(search(str, str_len)){
                cont++;
                if(cont == time){
                    replace(str, rep, str_len, rep_len, num);
                }else{
                    fseek(file, 1, SEEK_CUR);
                }
            }
        }
    }
}

void show(byte *str, int num){
    byte *b = (byte*) calloc(1,sizeof(byte));
    int cont = 0;
    printf("0x");
    while(cont < num){
        fread(b, sizeof(byte), 1, file);
        printf("%x", b[0]);
        cont++;
    }
    printf("\n");
}

void b(int time, byte *str, int b, int str_len){
    int cont = 0;
     while (!feof(file) && cont < time){
        if(search(str, str_len)){
            cont++;
            if(cont == time){
                show(str,b);
            }else{
                fseek(file, 1, SEEK_CUR);
            }
        }
    }
}

unsigned long long strToHex(char const *a){ //convierte una cadena de bytes de string a int
    int aSize = strlen(a), i;
    unsigned long long num = 0;
    if(a[0] == '0' && a[1] == 'x'){
        for(i=2; i<aSize; i++){
            switch(a[i]){
                case '0':  
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                    
                case '1': 
                    num += 1;
                    if(i!=(aSize-1)){
                            num*=16;
                    }
                    break;
                    
                case '2': 
                    num += 2;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '3': 
                    num += 3;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '4': 
                    num += 4;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '5': 
                    num += 5;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '6': 
                    num += 6;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '7': 
                    num += 7;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '8': 
                    num += 8;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '9': 
                    num += 9;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'a': 
                case 'A': 
                    num += 0xa;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'b':
                case 'B':  
                    num += 0xb;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'c':
                case 'C':  
                    num += 0xc;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'd':
                case 'D':  
                    num += 0xd;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'e':
                case 'E':  
                    num += 0xe;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'f':
                case 'F':  
                    num += 0xf;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;                      
            }
        }
    }
    else{
        for(i=0; i<aSize; i++){
            switch(a[i]){
                case '0':  
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                    
                case '1': 
                    num += 1;
                    if(i!=(aSize-1)){
                            num*=16;
                    }
                    break;
                    
                case '2': 
                    num += 2;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '3': 
                    num += 3;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '4': 
                    num += 4;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '5': 
                    num += 5;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '6': 
                    num += 6;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '7': 
                    num += 7;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '8': 
                    num += 8;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case '9': 
                    num += 9;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'a': 
                case 'A': 
                    num += 0xa;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'b':
                case 'B':  
                    num += 0xb;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'c':
                case 'C':  
                    num += 0xc;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'd':
                case 'D':  
                    num += 0xd;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'e':
                case 'E':  
                    num += 0xe;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;
                case 'f':
                case 'F':  
                    num += 0xf;
                    if(i!=(aSize-1)){
                        num*=16;
                    }
                    break;                      
            }
        }
    }
    return num;
}

void buscar(char comando, char comando2, char comando3, int enesimaAparicion, unsigned short buscarW, int buscarI, long long buscarL, long long reemplazar){ 
    long sizeFile;
    unsigned char *datos;
    unsigned int j=0, h=0, auxi=0, k;
    long long i=0, cont=0, auxReemplazar;
    unsigned short auxw=0;
    long long auxl=0;
    int n;
    
    if(file==NULL){
        puts("No se pudo abrir el archivo");
        return;
    } 

    fseek(file,0,SEEK_END);
    sizeFile = ftell(file);
    rewind(file);
    

    datos = (char*) malloc(sizeof(char)*(sizeFile));    


    if(datos == NULL){
        puts("Error al reservar memoria");
        return;
    }
    
    fread(datos, sizeFile, 1, file);
    
    if(comando == 'w'){ //busca un short
        cont = 0;
        for(i=0; i<sizeFile; i++){      
            auxw = (datos[i] * 0x100) + datos[i+1]; 
            if(buscarW == auxw){
                auxReemplazar = reemplazar;
                cont++;
                if(comando2 == 'n' && comando3 == 'r'){
                    if(cont == enesimaAparicion){
                        datos[i+1] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i] = auxReemplazar%0x100;
                        break;
                    }       
                }
                else if(comando2 == 'n'){
                }
                else if(comando2 == 'r'){
                        datos[i+1] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                }
            }
        }
    }
    else if(comando == 'i'){  //busca un int
        cont = 0;
        for(i=0; i<sizeFile; i+=1){
            auxi = (datos[i] * 0x1000000) + (datos[i+1]*0x10000) + (datos[i+2]*0x100) + datos[i+3];
            if(buscarI == auxi){
                auxReemplazar = reemplazar;
                cont++;
                if(comando2 == 'n' && comando3 == 'r'){
                    if(cont == enesimaAparicion){
                        datos[i+3] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+2] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+1] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;                        
                        break;
                    }       
                }
                else if(comando2 == 'n'){
                }
                else if(comando2 == 'r'){
                        datos[i+3] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+2] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+1] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                }
            }
        }
    }
    else if(comando == 'l'){ //busca un long
        cont = 0;
        for(i=0; i<sizeFile; i+=1){
            auxl = (datos[i] * 0x100000000000000) + (datos[i+1]*0x1000000000000) + (datos[i+2]*0x10000000000) + (datos[i+3]*0x100000000) + (datos[i+4]*0x1000000) + (datos[i+5]*0x10000) + (datos[i+6]*0x100) + datos[i+7];
            if(buscarL == auxl){
                cont++;
                auxReemplazar = reemplazar;
                if(comando2 == 'n' && comando3 == 'r'){
                    if(cont == enesimaAparicion){
                        datos[i+7] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+6] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+5] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+4] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+3] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+2] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+1] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        break;
                    }       
                }
                else if(comando2 == 'n'){
                }
                else if(comando2 == 'r'){
                        datos[i+7] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+6] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+5] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+4] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+3] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+2] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i+1] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                        datos[i] = auxReemplazar%0x100;
                        auxReemplazar = auxReemplazar/0x100;
                }
            }
        }
    }
            
    rewind(file);
    fwrite(datos,sizeFile,1,file);
    free(datos);
}

int Archivo_a_buscar(char const *nFile, char buscar[]){
    int n = strlen(nFile), val;
    if((nFile[n-4] == '.' && nFile[n-3] == 'p' && nFile[n-2] == 'd' && nFile[n-1] == 'f') || (nFile[n-3] == '.' && nFile[n-2] == 'a' && nFile[n-1] == 'i')){
        buscar[0] = 0x25;
        buscar[1] = 0x50;
        buscar[2] = 0x44;
        buscar[3] = 0x46;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'b' && nFile[n-2] == 'm' && nFile[n-1] == 'p'){
        buscar[0] = 0x42;
        buscar[1] = 0x4d;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'j' && nFile[n-2] == 'p' && nFile[n-1] == 'g'){
        buscar[0] = 0xff;
        buscar[1] = 0xd8;
        val = 2;
    }
    else if(nFile[n-6] == '.' && nFile[n-5] == 'c' && nFile[n-4] == 'l' && nFile[n-3] == 'a'&& nFile[n-2] == 's'&& nFile[n-1] == 's'){
        buscar[0] = 0xca;
        buscar[1] = 0xfe;
        buscar[2] = 0xba;
        buscar[3] = 0xbe;
        val = 4;
    }   
    else if(nFile[n-4] == '.' && nFile[n-3] == 'j' && nFile[n-2] == 'p' && nFile[n-1] == '2'){
        buscar[0] = 0x00;         
        buscar[1] = 0x00;
        buscar[2] = 0x00;
        buscar[3] = 0x0C;
        buscar[4] = 0x6A;
        buscar[5] = 0x50;
        buscar[6] = 0x20;
        buscar[7] = 0x20;
        buscar[8] = 0x0D;
        buscar[9] = 0x0A;
        val = 10;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'g' && nFile[n-2] == 'i' && nFile[n-1] == 'f'){
        buscar[0] = 0x47;
        buscar[1] = 0x49;
        buscar[2] = 0x46;
        buscar[3] = 0x38;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 't' && nFile[n-2] == 'i' && nFile[n-1] == 'f'){
        buscar[0] = 0x49;
        buscar[1] = 0x49;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'p' && nFile[n-2] == 'n' && nFile[n-1] == 'g'){
        buscar[0] = 0x89;
        buscar[1] = 0x50;
        buscar[2] = 0x4e;
        buscar[3] = 0x47;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'p' && nFile[n-2] == 's' && nFile[n-1] == 'd'){
        buscar[0] = 0x38;
        buscar[1] = 0x42;
        buscar[2] = 0x50;
        buscar[3] = 0x53;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'w' && nFile[n-2] == 'm' && nFile[n-1] == 'f'){
        buscar[0] = 0xd7;
        buscar[1] = 0xcd;
        buscar[2] = 0xc6;
        buscar[3] = 0x9a;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 'i' && nFile[n-1] == 'd'){
        buscar[0] = 0x4d;
        buscar[1] = 0x54;
        buscar[2] = 0x68;
        buscar[3] = 0x64;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'i' && nFile[n-2] == 'c' && nFile[n-1] == 'o'){
        buscar[0] = 0x00;
        buscar[1] = 0x00;
        buscar[2] = 0x01;
        buscar[3] = 0x00;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 'p' && nFile[n-1] == '3'){
        buscar[0] = 0x49;
        buscar[1] = 0x44;
        buscar[2] = 0x33;
        val = 3;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'a' && nFile[n-2] == 'v' && nFile[n-1] == 'i'){
        buscar[0] = 0x52;
        buscar[1] = 0x49;
        buscar[2] = 0x46;
        buscar[3] = 0x46;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 's' && nFile[n-2] == 'w' && nFile[n-1] == 'f'){
        buscar[0] = 0x46;
        buscar[1] = 0x57;
        buscar[2] = 0x53;
        val = 3;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'f' && nFile[n-2] == 'l' && nFile[n-1] == 'v'){
        buscar[0] = 0x46;
        buscar[1] = 0x4c;
        buscar[2] = 0x56;
        val = 3;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 'p' && nFile[n-1] == '4'){
        buscar[0] = 0x00;
        buscar[1] = 0x00;
        buscar[2] = 0x00;
        buscar[3] = 0x18;
        buscar[4] = 0x66;
        buscar[5] = 0x74;
        buscar[6] = 0x79;
        buscar[7] = 0x70;
        buscar[8] = 0x6d;
        buscar[9] = 0x70;
        buscar[10] = 0x34;
        buscar[11] = 0x32;
        val = 12;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 'o' && nFile[n-1] == 'v'){
        buscar[0] = 0x6d;
        buscar[1] = 0x6f;
        buscar[2] = 0x6f;
        buscar[3] = 0x76;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'w' && nFile[n-2] == 'm' && nFile[n-1] == 'v'){
        buscar[0] = 0x30;
        buscar[1] = 0x26;
        buscar[2] = 0xb2;
        buscar[3] = 0x75;
        buscar[4] = 0x8e;
        buscar[5] = 0x66;
        buscar[6] = 0xcf;
        val = 7;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'w' && nFile[n-2] == 'm' && nFile[n-1] == 'a'){
        buscar[0] = 0x30;
        buscar[1] = 0x26;
        buscar[2] = 0xb2;
        buscar[3] = 0x75;
        buscar[4] = 0x8e;
        buscar[5] = 0x66;
        buscar[6] = 0xcf;
        val = 7;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'z' && nFile[n-2] == 'i' && nFile[n-1] == 'p'){
        buscar[0] = 0x50;
        buscar[1] = 0x4b;
        buscar[2] = 0x03;
        buscar[3] = 0x04;
        val = 4;
    }
    else if(nFile[n-3] == '.' && nFile[n-2] == 'g' && nFile[n-1] == 'z'){
        buscar[0] = 0x1f;
        buscar[1] = 0x8b;
        buscar[2] = 0x08;
        val = 3;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 't' && nFile[n-2] == 'a' && nFile[n-1] == 'r'){
        buscar[0] = 0x75;
        buscar[1] = 0x73;
        buscar[2] = 0x74;
        buscar[3] = 0x61;
        buscar[4] = 0x72;
        val = 5;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 's' && nFile[n-1] == 'i'){
        buscar[0] = 0xd0;
        buscar[1] = 0xcf;
        buscar[2] = 0x11;
        buscar[3] = 0xe0;
        buscar[4] = 0xa1;
        buscar[5] = 0xb1;
        buscar[6] = 0x1a;
        buscar[7] = 0xe1;
        val = 8;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'o' && nFile[n-2] == 'b' && nFile[n-1] == 'j'){
        buscar[0] = 0x4c;
        buscar[1] = 0x01;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'd' && nFile[n-2] == 'l' && nFile[n-1] == 'l'){
        buscar[0] = 0x4d;
        buscar[1] = 0x5a;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'c' && nFile[n-2] == 'a' && nFile[n-1] == 'b'){
        buscar[0] = 0x4d;
        buscar[1] = 0x53;
        buscar[2] = 0x43;
        buscar[3] = 0x46;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'e' && nFile[n-2] == 'x' && nFile[n-1] == 'e'){
        buscar[0] = 0x4d;
        buscar[1] = 0x5a;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'r' && nFile[n-2] == 'a' && nFile[n-1] == 'r'){
        buscar[0] = 0x52;
        buscar[1] = 0x61;
        buscar[2] = 0x72;
        buscar[3] = 0x21;
        buscar[4] = 0x1a;
        buscar[5] = 0x07;
        buscar[6] = 0x00;
        val = 7;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 's' && nFile[n-2] == 'y' && nFile[n-1] == 's'){
        buscar[0] = 0x4d;
        buscar[1] = 0x5a;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'h' && nFile[n-2] == 'l' && nFile[n-1] == 'p'){
        buscar[0] = 0x3f;
        buscar[1] = 0x5f;
        buscar[2] = 0x03;
        buscar[3] = 0x00;
        val = 4;
    }
    else if(nFile[n-5] == '.' && nFile[n-4] == 'v' && nFile[n-3] == 'm' && nFile[n-2] == 'd' && nFile[n-1] == 'k'){
        buscar[0] = 0x4b;
        buscar[1] = 0x44;
        buscar[2] = 0x4d;
        buscar[3] = 0x56;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'p' && nFile[n-2] == 's' && nFile[n-1] == 't'){
        buscar[0] = 0x21;
        buscar[1] = 0x42;
        buscar[2] = 0x44;
        buscar[3] = 0x4e;
        buscar[4] = 0x42;
        val = 5;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'd' && nFile[n-2] == 'o' && nFile[n-1] == 'c'){
        buscar[0] = 0xd0;
        buscar[1] = 0xcf;
        buscar[2] = 0x11;
        buscar[3] = 0xe0;
        buscar[4] = 0xa1;
        buscar[5] = 0xb1;
        buscar[6] = 0x1a;
        buscar[7] = 0xe1;
        val = 8;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'r' && nFile[n-2] == 't' && nFile[n-1] == 'f'){
        buscar[0] = 0x7b;
        buscar[1] = 0x5c;
        buscar[2] = 0x72;
        buscar[3] = 0x74;
        buscar[4] = 0x66;
        buscar[5] = 0x31;
        val = 6;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'x' && nFile[n-2] == 'l' && nFile[n-1] == 's'){
        buscar[0] = 0xd0;
        buscar[1] = 0xcf;
        buscar[2] = 0x11;
        buscar[3] = 0xe0;
        buscar[4] = 0xa1;
        buscar[5] = 0xb1;
        buscar[6] = 0x1a;
        buscar[7] = 0xe1;
        val = 8;
    }   
    else if(nFile[n-4] == '.' && nFile[n-3] == 'p' && nFile[n-2] == 'p' && nFile[n-1] == 't'){
        buscar[0] = 0xd0;
        buscar[1] = 0xcf;
        buscar[2] = 0x11;
        buscar[3] = 0xe0;
        buscar[4] = 0xa1;
        buscar[5] = 0xb1;
        buscar[6] = 0x1a;
        buscar[7] = 0xe1;
        val = 8;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'v' && nFile[n-2] == 's' && nFile[n-1] == 'd'){
        buscar[0] = 0xd0;
        buscar[1] = 0xcf;
        buscar[2] = 0x11;
        buscar[3] = 0xe0;
        buscar[4] = 0xa1;
        buscar[5] = 0xb1;
        buscar[6] = 0x1a;
        buscar[7] = 0xe1;
        val = 8;
    }
    else if(nFile[n-5] == '.' && nFile[n-4] == 'd' && nFile[n-3] == 'o' && nFile[n-2] == 'c' && nFile[n-1] == 'x'){
        buscar[0] = 0x50;
        buscar[1] = 0x4b;
        buscar[2] = 0x03;
        buscar[3] = 0x04;
        val = 4;
    }
    else if(nFile[n-5] == '.' && nFile[n-4] == 'x' && nFile[n-3] == 'l' && nFile[n-2] == 's' && nFile[n-1] == 'x'){
        buscar[0] = 0x50;
        buscar[1] = 0x4b;
        buscar[2] = 0x03;
        buscar[3] = 0x04;
        val = 4;
    }
    else if(nFile[n-5] == '.' && nFile[n-4] == 'p' && nFile[n-3] == 'p' && nFile[n-2] == 't' && nFile[n-1] == 'x'){
        buscar[0] = 0x50;
        buscar[1] = 0x4b;
        buscar[2] = 0x03;
        buscar[3] = 0x04;
        val = 4;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 'd' && nFile[n-1] == 'b'){
        buscar[0] = 0x53;
        buscar[1] = 0x74;
        buscar[2] = 0x61;
        buscar[3] = 0x6e;
        buscar[4] = 0x64;
        buscar[5] = 0x61;
        buscar[6] = 0x72;
        buscar[7] = 0x64;
        buscar[8] = 0x20;
        buscar[9] = 0x4a;
        buscar[10] = 0x65;
        buscar[11] = 0x74;
        val = 12;
    }
    else if(nFile[n-3] == '.' && nFile[n-2] == 'p' && nFile[n-1] == 's'){
        buscar[0] = 0x25;
        buscar[1] = 0x21;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'm' && nFile[n-2] == 's' && nFile[n-1] == 'g'){
        buscar[0] = 0xd0;
        buscar[1] = 0xcf;
        buscar[2] = 0x11;
        buscar[3] = 0xe0;
        buscar[4] = 0xa1;
        buscar[5] = 0xb1;
        buscar[6] = 0x1a;
        buscar[7] = 0xe1;
        val = 8;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'e' && nFile[n-2] == 'p' && nFile[n-1] == 's'){
        buscar[0] = 0x25;
        buscar[1] = 0x21;
        buscar[2] = 0x50;
        buscar[3] = 0x53;
        buscar[4] = 0x2d;
        buscar[5] = 0x41;
        buscar[6] = 0x64;
        buscar[7] = 0x6f;
        buscar[8] = 0x62;
        buscar[9] = 0x65;
        buscar[10] = 0x2d;
        buscar[11] = 0x33;
        buscar[12] = 0x2e;
        buscar[13] = 0x30;
        buscar[14] = 0x20;
        buscar[15] = 0x45;
        buscar[16] = 0x50;
        buscar[17] = 0x53;
        buscar[18] = 0x46;
        buscar[19] = 0x2d;
        buscar[20] = 0x33;
        buscar[21] = 0x20;
        buscar[22] = 0x30;
        val = 23;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 'j' && nFile[n-2] == 'a' && nFile[n-1] == 'r'){
        buscar[0] = 0x50;
        buscar[1] = 0x4b;
        buscar[2] = 0x03;
        buscar[3] = 0x04;
        buscar[4] = 0x14;
        buscar[5] = 0x00;
        buscar[6] = 0x08;
        buscar[7] = 0x00;
        buscar[8] = 0x08;
        buscar[9] = 0x00;
        val = 10;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 's' && nFile[n-2] == 'l' && nFile[n-1] == 'n'){
        buscar[0] = 0x4d;
        buscar[1] = 0x69;
        buscar[2] = 0x63;
        buscar[3] = 0x72;
        buscar[4] = 0x6f;
        buscar[5] = 0x73;
        buscar[6] = 0x6f;
        buscar[7] = 0x66;
        buscar[8] = 0x74;
        buscar[9] = 0x20;
        buscar[10] = 0x56;
        buscar[11] = 0x69;
        buscar[12] = 0x73;
        buscar[13] = 0x75;
        buscar[14] = 0x61;
        buscar[15] = 0x6c;
        buscar[16] = 0x20;
        buscar[17] = 0x53;
        buscar[18] = 0x74;
        buscar[19] = 0x75;
        buscar[20] = 0x64;
        buscar[21] = 0x69;
        buscar[22] = 0x6f;
        buscar[23] = 0x20;
        buscar[24] = 0x53;
        buscar[25] = 0x6f;
        buscar[26] = 0x6c;
        buscar[27] = 0x75;
        buscar[28] = 0x74;
        buscar[29] = 0x69;
        buscar[30] = 0x6f;
        buscar[31] = 0x6e;
        buscar[32] = 0x20;
        buscar[33] = 0x46;
        buscar[34] = 0x69;
        buscar[35] = 0x6c;
        buscar[36] = 0x65;
        val = 37;
    }
    else if(nFile[n-5] == '.' && nFile[n-4] == 'z' && nFile[n-3] == 'l' && nFile[n-2] == 'i' && nFile[n-1] == 'b'){
        buscar[0] = 0x78;
        buscar[1] = 0x9c;
        val = 2;
    }
    else if(nFile[n-4] == '.' && nFile[n-3] == 's' && nFile[n-2] == 'd' && nFile[n-1] == 'f'){
        buscar[0] = 0x78;
        buscar[1] = 0x9c;
        val = 2;
    }   
    return val;
}

void Swap(unsigned int buscar, unsigned int cambiar){
    long sizeFile;
    unsigned char* datos;
    int i;
    int aux=0;

    fseek(file,0,SEEK_END);
    sizeFile = ftell(file);
    rewind(file);
    datos = (unsigned char*) malloc(sizeof(char)*sizeFile);
    
    if(datos == NULL){
        puts("Error al reservar memoria");
        return;
    }
    
    fread(datos, sizeFile, 1, file);
    
    for(i=0; i<sizeFile; i++){
        if(datos[i] == buscar){
            datos[i] = cambiar;
            ++i;
        }   
        if(datos[i] == cambiar){
            datos[i] = buscar;  
        }
    }
    
    
    rewind(file);
    fwrite(datos,sizeFile,1,file);
    free(datos);
}

int main(int argc, char const *argv[]){
    if(argc > 1){
        if(openFile(argv[1])){
            if(argc > 2){
                if(!strcmp(argv[2],"-e")){
                    if(argc > 3){
                        if (argc > 4){
                            if (!strcmp(argv[4],"-r")) {
                                if(argc > 5){
                                    int *str_len = (int*) calloc(1,sizeof(int));
                                    int *rep_len = (int*) calloc(1,sizeof(int));
                                    byte *str = convert(argv[3], str_len);
                                    byte *rep = convert(argv[5], rep_len);
                                    r(-1, str, rep, 0, *str_len, *rep_len);
                                }else{
                                    printf("Se necesita el reemplazo del patron");
                                }
                            }else if(!strcmp(argv[4],"-n")){
                                if(argc > 5){
                                    if(argc > 6){
                                        if(!strcmp(argv[6],"-r")){
                                            if(argc > 7){
                                                int *str_len = (int*) calloc(1,sizeof(int));
                                                int *rep_len = (int*) calloc(1,sizeof(int));
                                                byte *str = convert(argv[3], str_len);
                                                byte *rep = convert(argv[7], rep_len);
                                                r(strToInt(argv[5]), str, rep, 0, *str_len, *rep_len);
                                            }else{
                                                printf("Se necesita el reemplazo del patron");
                                            }
                                        }else if(!strcmp(argv[6],"-b")){
                                            if(argc == 8){
                                                int *str_len = (int*) calloc(1,sizeof(int));
                                                byte *str = convert(argv[3], str_len);
                                                b(strToInt(argv[5]), str, strToInt(argv[7]), *str_len);
                                            }else if(argc > 8){
                                                if (!strcmp(argv[8],"-r")){
                                                    if(argc > 9){
                                                        int *str_len = (int*) calloc(1,sizeof(int));
                                                        int *rep_len = (int*) calloc(1,sizeof(int));
                                                        byte *str = convert(argv[3], str_len);
                                                        byte *rep = convert(argv[9], rep_len);
                                                        r(strToInt(argv[5]), str, rep, strToInt(argv[9]), *str_len, *rep_len);
                                                    }else{
                                                        printf("Se necesita el reemplazo del patron");
                                                    }
                                                }
                                            }else{
                                                printf("Se necesita el numero de bytes");
                                            } 
                                        }
                                    }
                                }else{
                                    printf("Se necesita el numero de veces");
                                }
                            }
                        }
                    }else{
                        printf("Se necesita un patron a buscar\n");
                    }
                }else if(!strcmp(argv[2],"-i")){
                    if(argc > 6){
                        buscar('i',argv[4][1],argv[6][1],strToHex(argv[5]), 0, strToHex(argv[3]), 0,strToHex(argv[7]));
                    }
                    else if(argc > 4){
                        if(argv[4][1] == 'n'){
                            buscar('i','n','/',strToHex(argv[5]), 0, strToHex(argv[3]), 0,0);
                        }
                        else if(argv[4][1] == 'r'){
                            buscar('i','r','/',0, 0, strToHex(argv[3]), 0,strToHex(argv[5]));
                        }       
                    }
                }else if(!strcmp(argv[2],"-w")){
                    if(argc > 6){
                        buscar('w',argv[4][1],argv[6][1],strToHex(argv[5]), strToHex(argv[3]), 0, 0, strToHex(argv[7]));
                    }
                    else if(argc > 4){
                        if(argv[4][1] == 'n'){
                            buscar('w','n','/',strToHex(argv[5]), strToHex(argv[3]), 0, 0,0);
                        }
                        else if(argv[4][1] == 'r'){
                            buscar('w','r','/',0, strToHex(argv[3]), 0, 0,strToHex(argv[5]));
                        }       
                    }               
                }else if(!strcmp(argv[2],"-l")){
                    if(argc > 6){
                        buscar('l',argv[4][1],argv[6][1],strToHex(argv[5]), 0, 0, strToHex(argv[3]), strToHex(argv[7]));
                    }
                    else if(argc > 4){
                        if(argv[4][1] == 'n'){
                            buscar('l','n','/',strToHex(argv[5]), 0, strToHex(argv[3]), 0,0);
                        }
                        else if(argv[4][1] == 'r'){
                            buscar('l','r','/',0, 0, 0, strToHex(argv[3]), strToHex(argv[5]));  
                        }
                        
                    }               
                }else if(!strcmp(argv[2],"-s")){
                    if(argc > 3){
                        int str_len = strlen(argv[3]);
                        char const *c = argv[3];
                        byte *b = (byte*) calloc(str_len + 1, sizeof(byte));
                        for ( i = 0; i < str_len + 1; i++){
                            if (i == str_len){
                                b[i] = 0;
                            }
                            b[i] = (int) c[i];
                        }
                        if(argc > 4){
                            if(!strcmp(argv[4],"-r")){
                                if(argc > 5){
                                    int *rep_len = (int*) calloc(1,sizeof(int));
                                    byte *rep = convert(argv[5], rep_len);
                                    r(-1, b, rep, 0, str_len, *rep_len);
                                }
                            }else if(!strcmp(argv[4],"-n")){
                                if (argc > 5){
                                    if(argc > 6){
                                        if (!strcmp(argv[6],"-r")){
                                            if (argc > 7){
                                                int *rep_len = (int*) calloc(1,sizeof(int));
                                                byte *rep = convert(argv[7], rep_len);
                                                r(strToInt(argv[5]), b, rep, 0, str_len, *rep_len);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }else if(!strcmp(argv[2],"-cm")){
                    int j=0, i=0, cont=0, aux=0, z=0;
                    int hex = strToHex(argv[3]);
                    byte buscar[37];                                        
                    int cambiarSize = (strlen(argv[3])-2)/2;
                    unsigned char *cambiar, *b;
                    cambiar = (char*) malloc(sizeof(char)*(cambiarSize));
                    b = (unsigned char*) malloc(sizeof(char)*(cambiarSize));
                    
                    while(hex>0){
                        aux = hex%256;
                        hex = hex/256;
                        b[i] = aux;
                        i++;
                        cont++;
                    }
                    i--;    cont--;
                    
                    while(i>=0){
                        cambiar[j] = b[i];
                        i--;
                        j++;
                    }
                    
                    z = Archivo_a_buscar(argv[1],buscar);   
                    r(1,buscar, cambiar,0, z, strlen(cambiar));                
                    free(cambiar);
                    free(b);
                }
                else if(!strcmp(argv[2],"swap") || !strcmp(argv[2],"-swap")){
                    unsigned long long buscar, cambiar;
                    buscar = strToHex(argv[3]);
                    cambiar = strToHex(argv[4]);
                    Swap(buscar,cambiar);
                }
            }else{
                printf("Se debe indicar un comando\n");
            }
            fclose(file);
        }else{
            printf("No se pudo abrir el archivo\n");
            printf("Por favor compruebe que el nombre y la ruta del archivo sea correcta\n");
        }
    }else{
        printf("'ERROR' debe indicar un nombre de archivo\n");
    }
    return 0;
}