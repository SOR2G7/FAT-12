#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char starting_cluster[4];
    char file_size[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short; // if zero, later field is used
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
} __attribute((packed)) Fat12BootSector;

typedef struct {
    unsigned char filename[8];                  // Nombre del archivo
    unsigned char ext[3];                       // Extensión del archivo
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


void print_file_info(Fat12Entry *entry){
    unsigned char firstChar = entry->filename[0];
    if (firstChar == 0xE5 && entry->attributes[0] == ' ') {
        printf("Archivo borrado: [?%.7s.%.3s]\n", &entry->filename[1], entry->ext);
        return;
    } else if (firstChar == 0x05) {
        printf("Archivo que comienza con 0x05: [%c%.7s.%.3s]\n", 0xE5, &entry->filename[1], entry->ext);
        return;
    }

    switch(entry->attributes[0]) {
    case 0x10:
        printf("Directory: [%.8s.%.3s]\n", entry->filename, entry->ext);
        break;
    case 0x20:
    	printf("----------------------------------------------\n");
        printf("File: [%.8s.%.3s]\n", entry->filename, entry->ext);
        printf("Extension: %.3s\n", entry->ext);
        printf("Cluster (alta dirección de bytes): 0x%02X%02X\n", entry->cluster_highbytes_address[1], entry->cluster_highbytes_address[0]);
        printf("Cluster (baja dirección de bytes): 0x%02X%02X\n", entry->cluster_lowbytes_address >> 8, entry->cluster_lowbytes_address & 0xFF);
        printf("Tamaño de archivo: %d bytes\n", entry->size_of_file);
        printf("----------------------------------------------\n");
        break;
    }
}




int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
   
    fseek(in, 446, SEEK_SET); // ir al inicio de la tabla de particiones
    fread(pt, sizeof(PartitionTable), 4, in); // leer tabla de particiones
    
    for(i=0; i<4; i++) {        
        if(pt[i].partition_type == 1) {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }
    
    fseek(in, pt[i].start_chs[2]*512, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in); // leer boot sector
    
    printf("En  0x%lX, sector size %d, FAT size %d sectors, %d FATs\n\n", 
           ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
           
    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    printf("Root dir_entries %d \n", bs.root_dir_entries);
    for(i=0; i<bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry);
    }
    
    printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
    fclose(in);
    return 0;
}
