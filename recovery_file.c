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
    unsigned short sector_size; // 2 bytes
    unsigned char sectors_per_cluster; // 1 byte
    unsigned short reserved_sectors; // 2 bytes
    unsigned char number_of_fats; // 1 byte
    unsigned short max_root_entries; // 2 bytes
    unsigned short total_sectors; // 2 bytes
    unsigned char media_descriptor; // 1 byte
    unsigned short sectors_per_fat; // 2 bytes
    unsigned short sectors_per_track; // 2 bytes
    unsigned short number_of_heads; // 2 bytes
    unsigned int hidden_sectors; // 4 bytes
    unsigned int total_logical_sectors; // 4 bytes
    unsigned char drive_number; // 1 byte
    unsigned char reserved; // 1 byte
    unsigned char boot_signature; // 1 byte
    unsigned int volume_id; // 4 bytes
    char volume_label[11];
    char fs_type[8]; // Tipo en ASCII
    char boot_code[448];
    unsigned short boot_sector_signature; // 2 bytes
} __attribute__((packed)) Fat12BootSector;


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

char *read_file(unsigned short firstCluster, unsigned short fileFirstCluster, unsigned short clusterSize, int fileSize) {
    FILE *in = fopen("test.img", "rb");
    if (in == NULL) {
        printf("Error: No se pudo abrir el archivo\n");
        exit(1);
    }

    fseek(in, firstCluster + ((fileFirstCluster - 2) * clusterSize), SEEK_SET);
    
    char *to_read = malloc(fileSize);
    if (to_read == NULL) {
        printf("Error: No se pudo asignar memoria para leer el archivo\n");
        fclose(in);
        exit(1);
    }
    
    fread(to_read, fileSize, 1, in);
    
    fclose(in);
    return to_read;
}

void file_recovery(Fat12Entry *entry, unsigned short firstCluster, int clusterSize, long int lugar_actual) {

	unsigned char charToReplace[] = {'R'}; //Reemplazaremos el primer char del archivo borrado por R de Recovered
	FILE * in = fopen("test.img", "rb+");
	
    if (entry->filename[0] == 0x00) {
        // Entrada de directorio vacía o eliminada, ignorar
        return;
    }
    
    // Verificar si el archivo borrado no estaba vacío
    if (entry->filename[0] == 0xE5 && entry->cluster_lowbytes_address!=0) {
        // Recuperar el nombre del archivo
        
        char *contenido = read_file(firstCluster, entry->cluster_lowbytes_address, clusterSize, entry->size_of_file);
                
        fseek(in, lugar_actual, SEEK_SET);
        fwrite(charToReplace, sizeof(entry->filename[0]), 1, in);
        
        printf("Archivo recuperado: [%.8s%.3s]",  entry->filename, entry->ext);
        printf(" , cuyo contenido es:\n%s", contenido);
        
        // Recuperar el tamaño del archivo
        unsigned int fileSize = entry->size_of_file;
        
                
    }
}

int main() {

    FILE * in = fopen("test.img", "rwb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
    unsigned short firstCluster;
    
    fseek(in, 446, SEEK_SET); // ir al inicio de la tabla de particiones
    fread(pt, sizeof(PartitionTable), 4, in); // leer tabla de particiones
    
    for(i=0; i<4; i++) {        
        if(pt[i].partition_type == 1) {
            break;
        }
    }
    
    if(i == 4) {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }
    
    fseek(in, pt[i].start_chs[2]*512, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in); // leer boot sector
   
           
    fseek(in, (bs.reserved_sectors-1 + bs.sectors_per_fat * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    firstCluster = ftell(in) + (bs.max_root_entries * sizeof(entry));
    
    for(i=0; i<bs.max_root_entries; i++) {
    	long int lugar= ftell(in);
        //printf("%ld \n", lugar);
    	fread(&entry, sizeof(entry), 1, in);
    	file_recovery(&entry, firstCluster, bs.sector_size * bs.sectors_per_cluster, lugar);
    	
    }
    
    printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
    fclose(in);
    return 0;
}
