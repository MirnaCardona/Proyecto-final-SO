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

int LeeDirArch(int dr, int i, short int *edr)
{
 char arcvbn[10], ext[3], tempc[8];
  int j, temp=0;

  j=dr+i;
  //i=0;
  while(temp<=*edr+32)
  {
   // memset(arc,0,8);
strcpy(ext, &map[j+ 0x08]); // extencion del archivo
  ext[3]='\0'; 
     strcpy(tempc, &map[j+0x00]); // nombre del archivo
        tempc[8]='\0';
       
   short int *tipo = (short int *)&map[j+0x0b];
   short int *cluster = (short int *)&map[j+0x1a];
   short int *tama = (short int *)&map[j+0x1c];
                     
  printf("Archivo %s   .%s   %d   ,%d   ,%d\n", tempc,  ext, *tipo, *cluster, *tama);
  j=j+32;
    temp=temp+32;
}
  /*
        strcpy(arc2, &map[j+0x00]); // nombre del archivo
        arc2[8]='\0';
      strcpy(ext2, &map[j+ 0x08]); // extencion del archivo
  ext2[3]='\0';  
   short int *tipo2 = (short int *)&map[j+0x0b];
   short int *cluster2 = (short int *)&map[j+0x1a];
   short int *tama2 = (short int *)&map[j+0x1c];
                     
  printf("   %s   ,%s    ,%d   ,%d   ,%d\n", arc2, ext2, *tipo2, *cluster2, *tama2); 
    j=j+20;
        strcpy(arc3, &map[j+0x00]); // nombre del archivo
        arc3[8]='\0';
      strcpy(ext3, &map[j+ 0x08]); // extencion del archivo
  ext3[3]='\0';  
   short int *tipo3 = (short int *)&map[j+0x0b];
   short int *cluster3 = (short int *)&map[j+0x1a];
   short int *tama3 = (short int *)&map[j+0x1c];
                     
  printf("   %s   ,%s    ,%d   ,%d   ,%d\n", arc3, ext3, *tipo3, *cluster3, *tama3); 
  
  */
  //printf("%s\n", ids); 

 // }
 
}
int leerdatos(int i){
  char ev[11];
  char ids[5];
  char ev2[11];
  char ids2[5];
 
  short int *ts = (short int *)&map[i+11];
  printf("Tamano del Sector: %d\n", *ts);

  int spc = map[i+13];
  printf("Numero de Sectores Por Cluster: %d\n", spc);

  short int *sr = (short int *)&map[i+14];
  printf("Sectores Reservados: %d\n", *sr);

  int ncf = map[i+16];
  printf("Numero de Copias del FAT: %d\n", ncf);

  short int *edr = (short int *)&map[i+17];
  printf("Entradas en el Directorio Raiz: %d\n", *edr);

  short int *tdf = (short int *)&map[i+22];
  printf("Tamaño del FAT: %d\n", *tdf);
  int tdf2= *tdf;
  
  short int *sdd = (short int *)&map[i+32]; // sectores del disco
  printf("Sectores del Disco: %d\n", *sdd);
  //short int temp = *sdd;

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
  LeeDirArch(dr,i,edr);
}

int partitions(){
   int a=0, a2=0;
  if(esMBR(map)){
   printf("\nEs MBS\n\n");

   for(int i=0; i<=3; i++)
   {
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
    if(h2!=0)
    {
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
  return 0;

}
