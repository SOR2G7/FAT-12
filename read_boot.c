#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char start_sector[4];
    char length_sectors[4];
} __attribute__((packed)) PartitionTable;

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

int main() {
    FILE *in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;

    fseek(in, 0x1BE, SEEK_SET); // Ir al inicio de la tabla de particiones
    fread(pt, sizeof(PartitionTable), 4, in); // Leer entradas de la tabla de particiones

    for (i = 0; i < 4; i++) {
        printf("Tipo de partición: %d\n", pt[i].partition_type);
        if (pt[i].partition_type == 1) {
            printf("Encontrado sistema de archivos FAT12 en la partición %d\n", i);
            break;
        }
    }

    if (i == 4) {
        printf("No se encontró un sistema de archivos FAT12, saliendo...\n");
        return -1;
    }

    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);

    printf("Código de salto: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf("Código OEM: [%.8s]\n", bs.oem);
    printf("Tamaño del sector: %d\n", bs.sector_size);
    printf("Sectores por clúster: %d\n", bs.sectors_per_cluster);
    printf("Sectores reservados: %d\n", bs.reserved_sectors);
    printf("Número de FATs: %d\n", bs.number_of_fats);
    printf("Entradas máximas en el directorio raíz: %d\n", bs.max_root_entries);
    printf("Total de sectores: %d\n", bs.total_sectors);
    printf("Descriptor de medios: %d\n", bs.media_descriptor);
    printf("Sectores por FAT: %d\n", bs.sectors_per_fat);
    printf("Sectores por pista: %d\n", bs.sectors_per_track);
    printf("Número de cabezas: %d\n", bs.number_of_heads);
    printf("Sectores ocultos: %d\n", bs.hidden_sectors);
    printf("Sectores lógicos totales: %d\n", bs.total_logical_sectors);
    printf("Número de unidad: %d\n", bs.drive_number);
    printf("Reservado: %d\n", bs.reserved);
    printf("Firma de arranque: %d\n", bs.boot_signature);
    printf("ID del volumen: 0x%08X\n", bs.volume_id);
    printf("Etiqueta del volumen: [%.11s]\n", bs.volume_label);
    printf("Tipo de sistema de archivos: [%.8s]\n", bs.fs_type);
    printf("Firma del sector de arranque: 0x%04X\n", bs.boot_sector_signature);

    fclose(in);
    return 0;
}
