#include <stdio.h>
#include <stdlib.h>
 
typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char start_sector[4];
    unsigned int length_sectors;
} __attribute((packed)) PartitionTable;
 
typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sector_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short sector_volumen;
    unsigned char descriptor;
    unsigned short fat_size_sectors;
    unsigned short sector_track;
    unsigned short headers;
    unsigned int sector_hidden;
    unsigned int sector_partition;
    unsigned char physical_device;
    unsigned char current_header;
    unsigned char firm;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;
 
typedef struct {
    unsigned char filename[1];
    unsigned char nombre[7];
    unsigned char extension[3];
    unsigned char attributes[1];
    unsigned char reserved;
    unsigned char created_time_seconds;
    unsigned char created_time_hours_minutes_seconds[2];
    unsigned char created_day[2];
    unsigned char accessed_day[2];
    unsigned char cluster_highbytes_address[2];
    unsigned char written_time[2];
    unsigned char written_day[2];
    unsigned short cluster_lowbytes_address;
    unsigned int size_of_file;
} __attribute((packed)) Fat12Entry;
 
void print_file_info(Fat12Entry *entry, unsigned short firstCluster, unsigned short clusterSize) {
    switch(entry->filename[0]) {
    case 0x00:
        return; // unused entry
    case 0xE5:
        return;
    default:
        switch(entry->attributes[0]) {
            case 0x20:
            	
                printf("\n Archivo: [%.1s%.7s.%.3s]\n",  entry->filename, entry->nombre, entry->extension);
                read_file(firstCluster, entry->cluster_lowbytes_address, clusterSize, entry->size_of_file);
                return;
        }
    }    
}
 
void read_file(unsigned short firstCluster, unsigned short fileFirstCluster, unsigned short clusterSize, int fileSize){
    FILE * in = fopen("test.img", "rb");
    int i;
    char to_read[fileSize];
 
    fseek(in, firstCluster + ((fileFirstCluster - 2) * clusterSize) , SEEK_SET);
    fread(to_read, fileSize, 1, in);
    
    printf("\n");
 
    for(i=0; i<fileSize; i++) {
            printf("%c", to_read[i]);
    }
    
    printf("\n\n ----------------- \n");
 
    fclose(in);
}
 
int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
    unsigned short firstCluster;
 
    fseek(in, 0x1BE, SEEK_SET); 
    fread(pt, sizeof(PartitionTable), 4, in);
 
    for(i=0; i<4; i++) {        
        if(pt[i].partition_type == 1) {
            break;
        }
    }
 
    if(i == 4) {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }
 
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
 
    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
 
    firstCluster = ftell(in) + (bs.root_dir_entries * sizeof(entry));

    for(i=0; i<bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry, firstCluster, bs.sector_size * bs.sector_cluster);
    }
    fclose(in);
    return 0;
}
