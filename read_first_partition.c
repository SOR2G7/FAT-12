#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE * in = fopen("test.img", "rb");
    unsigned int i, start_sector, length_sectors;
    
    fseek(in, 446 , SEEK_SET); // Voy al inicio.
    
    for(i=0; i<1; i++) { // Leo las entradas
        printf("Partition entry %d:\n  Bootable flag: %02X\n", i, fgetc(in));
        printf("  Comienzo de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
        printf("  Partition type 0x%02X\n", fgetc(in));
        printf("  Fin de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
        
        fread(&start_sector, 4, 1, in);
        fread(&length_sectors, 4, 1, in);
        printf("  Dirección LBA relativa: 0x%08X\n  Tamaño en sectores: %d\n", start_sector, length_sectors);
    }
    
    fclose(in);
    return 0;
}
