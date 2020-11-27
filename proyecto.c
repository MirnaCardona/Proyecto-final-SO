#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <curses.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define byte unsigned char 

/* Variable global para mejor legibilidad */
int fd; // Archivo a leer
char *map; // Mapa

char *mapFile(char *filePath) {
  /* Abre archivo */
  fd = open(filePath, O_RDONLY);
  if (fd == -1) {
    perror("Error abriendo el archivo");
    return(NULL);
  }

  /* Mapea archivo */
  struct stat st;
  fstat(fd,&st);
  int fs = st.st_size;

  char *map = mmap(0, fs, PROT_READ, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    perror("Error mapeando el archivo");
    return(NULL);
  }

  return map;
}

int leeChar() {
  int chars[5];
    int ch,i=0;
    nodelay(stdscr, TRUE);
    while((ch = getch()) == ERR); /* Espera activa */
    ungetch(ch);
    while((ch = getch()) != ERR) {
      chars[i++]=ch;
    }
    /* convierte a numero con todo lo leido */
    int res=0;
    for(int j=0;j<i;j++){
      res <<=8;
      res |= chars[j];
    }
  return res;
}

int esMBR(char *base){
  int res = 1;  //Asumimos verdad
  int i=0;
  //Checar firma
  if(base[510] != 0x55 && base[511] != 0xAA){
    res=0;
  }
  //Checar que las particiones son  validas
  while (res && i<4){
    int p= 0x1BE + i*16;
    if(!(base[p] ==0 || base[p]==0x80)){
      res=0;
    }
    i++;
  }
  return res;  
}


int leerdatos(int i){
  int j, total=11, c, k=0;
  char ev[11];
  char ids[5];
  char ev2[11];
  char ids2[5];

  short int *ts = (short int *)&map[i+11]; // Tamaño del sector
  printf("Tamano del Sector: %d\n", *ts);

  int spc = map[i+13]; // Sectores por cluster
  printf("Numero de Sectores Por Cluster: %d\n", spc); 

  short int *sr = (short int *)&map[i+14]; // Sectores reservados
  printf("Sectores Reservados: %d\n", *sr);

  int ncf = map[i+16]; // copias del fat
  printf("Numero de Copias del FAT: %d\n", ncf);

  short int *edr = (short int *)&map[i+17]; // entradas en el directorio raiz
  printf("Entradas en el Directorio Raiz: %d\n", *edr);

  short int *tdf = (short int *)&map[i+22]; // tamaño de fat
  printf("Tamaño del FAT: %d\n", *tdf);
  int tdf2= *tdf;

  short int *sdd = (short int *)&map[i+32]; // sectores del disco
  printf("Sectores del Disco: %d\n", *sdd);
  short int temp = *sdd;

  strcpy(ev, &map[i+ 0x2b]); // etiqueta del volumen
  ev[11]='\0';
  printf("Etiqueta del Volumen: %s\n",ev);

  strcpy(ids, &map[i+ 0x36]); // id del sistema
  ids[8]='\0';
  printf("ID del Sistema: %s\n", ids);
  int dr = (*sr + (ncf * *tdf))* *ts; // dir raiz
  int di = dr + (*edr * 32); // inicio datos

  printf("Directorio Raiz: 0x%04x\n", dr);
  printf("Inicio Datos: 0x%04x\n\n", di);

  char str[80], str1[80], str2[80], str3[80], str4[80], str5[80], str6[80], str7[80],str8[80], str9[80], str10[80];
  sprintf(str, "Tamaño de disco: %d", *ts);
  sprintf(str1, "Numero de Sectores Por Cluster: %d", spc);
  sprintf(str2, "Sectores Reservados: %d", *sr);
  sprintf(str3, "Numero de FAT: %d", ncf);
  sprintf(str4, "Entradas en el Directorio Raiz: %d", *edr);
  sprintf(str5, "Tamaño del FAT: %d", *tdf);
  sprintf(str6, "Sectores del Disco: %d", temp);
  sprintf(str7, "Etiqueta del Volumen: %s\n",ev);
  sprintf(str8, "ID del Sistema: %s\n", ids);
  sprintf(str9, "Directorio Raiz: 0x%04x\n", dr);
  sprintf(str10, "Inicio Datos: 0x%04x\n\n", di);

  char *lista[] = { str, str2, str4, str5, str6, str1, str3, str8, str7, str9, str10};

  initscr();
  raw();
  noecho(); /* No muestres el caracter leido */
  cbreak(); /* Haz que los caracteres se le pasen al usuario */
  do{
    move(2,5);
    printw("INFORMACION DE DISCO");
    for(j=0; j < total; j++){
      if (j == k) {
        // attron(A_REVERSE);
      }
      mvprintw(4+j*2,5,lista[j]);
      // attroff(A_REVERSE);
    }
    move(5+j,5);
    refresh();
    c = leeChar();
    switch(c) {
      case 0x1B5B41:
        k = (k>0) ? k - 1 : total - 1;
        break;
      case 0x1B5B42:
        k = (k<total - 1) ? k + 1 : 0;
        break;
      default:
        // Nothing 
        break;
    }
    // printw("Estoy en %d: Lei 0x%08x DR: 0x%04x  ID: 0x%04x IDS: %s",k,c,dr,di, ids);
  }while (c != 0x1b);
}

int partitions(){
  int a=0, a2=0;
  if(esMBR(map)){
    printf("\nEs MBS\n\n");

    for(int i=0; i<=3; i++){
      int h =(byte)map[0x1BE + 1 + (i*16)];
      if(h!=0){
        printf("CHS Paricion %d\n", i+1);
        printf("Head: %02x\n", h);
        int s = map[0x1BE +  2 + (i*16)] & 0x3F;
        int c = map[0x1BE + 2 + (i*16)] & 0xC0;
        c<<=2;
        printf("Sector: %02x\n", s);
        c |= map[0x1BE + 3 + (i*16)];
        printf("Cylinder: %02x\n",c);
        a= ((c*255 + h)*63 +(s-1))*512;
        printf("Partition Start: %02x\n\n", a);

        int h2 =(byte)map[a+0x1BE + 1];
        if(h2!=0){
        printf("Partition Extended\n");
        printf("Head: %02x\n", h2);
        int s2 = map[a+0x1BE + 2] & 0x3F;
        int c2 = map[a+ 0x1BE + 2] & 0xC0;
        c2<<=2;
        printf("Sector: %02x\n", s2);
        c2 |= map[a+ 0x1BE + 3];
        printf("Cylinder: %02x\n",c2);
        a2= ((c2*255 + h2)*63 +(s2-1))*512;
        printf("Partition Extended Start: %02x\n\n", a2);
        leerdatos(a2);
        }else{
          leerdatos(a);
        }
      }
    }
  }else{
    printf("NO ES MBS\n");
  }
}

int abre(char *filename) {

  // Lee archivo 
  map = mapFile(filename);
  if (map == NULL) {
    exit(EXIT_FAILURE);
  }
  leerdatos(0);
  partitions();

  if (munmap(map, fd) == -1) {
    perror("Error al desmapear");
  }

  close(fd);
  return 0;
}

int main(char argc, char * argv[]) {
  abre(argv[1]);
  clear();
  endwin();
  return 0;
}
