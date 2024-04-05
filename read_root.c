#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char starting_cluster[4];
    char file_size[4];
} __attribute__((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short;
    unsigned char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_long;
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute__((packed)) Fat12BootSector;

typedef struct {
    unsigned char filename[8];                  // Nombre del archivo
    unsigned char extension[3];                 // Extensión del archivo
    unsigned char attributes[1];                // Atributos del archivo
    unsigned char reserved[2];                  // Reservado
    unsigned char created_time[2];              // Hora de creación
    unsigned char created_day[2];               // Fecha de creación
    unsigned char accessed_day[2];              // Fecha de último acceso
    unsigned char cluster_highbytes_address[2]; // Dirección de los bytes altos del clúster
    unsigned char written_time[2];              // Hora de última escritura
    unsigned char written_day[2];               // Fecha de última escritura
    unsigned short cluster_lowbytes_address;    // Dirección de los bytes bajos del clúster
    unsigned int size_of_file;                  // Tamaño del archivo
} __attribute((packed)) Fat12Entry;

void print_file_info(Fat12Entry *entry) {
    
    unsigned char firstChar = entry->filename[0];
     if (firstChar == 0xE5) {
        if (entry -> attributes[0] == 0x0F){
            printf("Archivo que comienza con 0xE5: [%c%.7s.%.3s]\n", 0xE5, &entry->filename[1], entry->extension);
        }
        else{
            printf("Archivo borrado: [?%.7s.%.3s]\n", &entry->filename[1], entry->extension);
        }
        return;
    }
    switch(firstChar) {
    case 0x00:
        return; // entrada no utilizada
    
    default:
        switch ((unsigned char) entry ->attributes[0]) {
        case 0x10:
            printf("Directorio: [?%.8s.%.3s]\n", entry->filename, entry->extension);
            break;
        case 0x20:
            if (entry->filename[0] != 0x0F)
            printf("Archivo: [?%.8s.%.3s]\n", entry->filename, entry->extension);
            break;
        }
    }
    
}


int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
    
    // Leer la tabla de particiones
    fseek(in, 0x1be, SEEK_SET); // Ir al inicio de la tabla de particiones en el MBR
    fread(&pt, sizeof(PartitionTable), 4, in);
    
    // Encontrar la partición FAT12
    for(i = 0; i < 4; i++) {        
        if(pt[i].partition_type == 1) {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No se encontró el sistema de archivos FAT12, saliendo...\n");
        return -1;
    }
    
    // Leer el sector de arranque (boot sector)
    fseek(in, pt[i].start_chs[2] * 512, SEEK_SET); // 512 es el tamaño del sector
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("En 0x%lX, tamaño de sector %d, tamaño de FAT %d sectores, %d FATs\n\n", 
           ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
    
    // Leer el directorio raíz
    fseek(in, (bs.reserved_sectors - 1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    printf("Entradas de directorio raíz %d \n", bs.root_dir_entries);
    for(i = 0; i < bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry);
    }
    
    printf("\nLeído el directorio raíz, ahora en 0x%lX\n", ftell(in));
    fclose(in);
    return 0;
}
